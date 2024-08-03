// ライセンス: GPL2

//#define _DEBUG
//#define _SHOW_GETBOARD
//#define _SHOW_BOARD
//#define _TEST_CACHE
#include "jddebug.h"

#include "root.h"

#include "bbsmenu.h"
#include "boardbase.h"
#include "boardfactory.h"

#include "config/globalconf.h"
#include "jdlib/cookiemanager.h"
#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"
#include "skeleton/editviewdialog.h"
#include "skeleton/msgdiag.h"

#include "cache.h"
#include "command.h"
#include "environment.h"
#include "global.h"
#include "httpcode.h"
#include "type.h"

#include <sys/stat.h>
#include <sys/types.h> // chmod

#include <iterator>
#include <sstream>


#ifdef _TEST_CACHE
int cache_hit1 = 0;
int cache_hit2 = 0;
int cache_nohit = 0;
#endif

enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024  //  bbsmenu.html の最大サイズ
};

// ルート要素名( boards.xml )
#define ROOT_NODE_NAME "boardlist"

using namespace DBTREE;


// is_moved() の戻り値
enum
{
    BOARD_MOVED = 0,
    BOARD_NEW,
    BOARD_EXISTS
};


Root::Root()
    : SKELETON::Loadable()
    , m_board_null{ std::make_unique<DBTREE::BoardBase>( "", "", "" ) }
    , m_bbsmenu_null{ std::make_unique<DBTREE::BBSMenu>( "", "" ) }
    , m_enable_save_movetable( true )
{
    m_xml_document.clear();
    clear();
    clear_load_data();
    load_movetable();
    load_cache();
    load_etc();
    load_bbsmenu();

    // JDのサポートBBS登録
    set_board( ENVIRONMENT::get_jdbbs(), "JDサポートBBS" );

    // 2chのスレの過去ログ
    set_board( ENVIRONMENT::get_jd2chlog(), "2chスレ過去ログ" );

    // ローカルファイル
    set_board( URL_BOARD_LOCAL, "ローカルファイル" );

}



//
// デストラクタで子Boardクラスをすべて削除
//
Root::~Root()
{
#ifdef _DEBUG    
    std::cout << "Root::~Root\n";
#endif

    // JDim 終了時に bbsmenu.txt を保存しないと板移転の結果が反映されない
    save_bbsmenu();

    clear();

    for( auto& b : m_list_board ) b->terminate_load();

#ifdef _TEST_CACHE
    std::cout << "board cache\n"
              << "hit1 = " << cache_hit1 << std::endl
              << "hit2 = " << cache_hit2 << std::endl
              << "nohit = " << cache_nohit << std::endl
              << "(hit1+hit2)/total*100 = " << (double)(cache_hit1+cache_hit2)/(cache_hit1+cache_hit2+cache_nohit)*100. << std::endl;
#endif    
}


void Root::clear()
{
    m_rawdata.clear();
    m_rawdata.shrink_to_fit();
}


//
// URLから BoardBase を取得する関数
//
// count は無限再帰呼び出し禁止用
//
BoardBase* Root::get_board( const std::string& url, const int count )
{
#ifdef _SHOW_GETBOARD
    std::cout << "Root::get_board : count = " << count << " url = " << url << std::endl;
#endif

    const int max_count = 50;

    // キャッシュ
    if( m_get_board ){

        if( url == m_get_board_url ){
#ifdef _TEST_CACHE
            ++cache_hit1;
#endif
            return m_get_board;
        }
        else if( m_get_board->equal( url ) ){

            m_get_board_url = url;

#ifdef _TEST_CACHE
            ++cache_hit2;
#endif
            return m_get_board;
        }
    }
#ifdef _TEST_CACHE
    ++cache_nohit;
#endif

    m_get_board_url = url;
    m_get_board = nullptr;

    if( count == 0 ){

        const std::size_t pos = url.rfind( "http://" );
        const std::size_t pos2 = url.rfind( "https://" );

        // ユーザープロフィールアドレス( http://be.2ch.net/test/p.php?u=d:http://〜 )の様に
        // 先頭以外に http:// が入っている場合は失敗
        if( ( pos != std::string::npos && pos != 0 ) || ( pos2 != std::string::npos && pos2 != 0 ) ) {
            return m_board_null.get();
        }

        // http:// が含まれていなかったら先頭に追加して再帰呼び出し
        else if( pos == std::string::npos && pos2 == std::string::npos && ! is_local( url ) ){
            const char* scheme{ url.rfind( "//", 0 ) == 0 ? "http:" : "http://" };
            BoardBase* board = get_board( scheme + url, count + 1 );
            m_get_board_url = url;
            return board;
        }
    }

    // サーチ
    auto it_board = std::find_if( m_list_board.begin(), m_list_board.end(),
                                  [&url]( const auto& b ) { return b->equal( url ); } );
    if( it_board != m_list_board.end() ) {

        m_get_board = it_board->get();
        m_get_board->read_info(); // 板情報の取得( 詳しくはBoardBase::read_info()をみること )

#ifdef _SHOW_GETBOARD
            std::cout << "found\n";
#endif
        return m_get_board;
    }

    // 見つからなかった

    if( count < max_count ){

        // 移転した時はrootを付け変えて再帰呼び出し
        std::string new_url = is_board_moved( url );
        if( ! new_url.empty() ){
            BoardBase* board = get_board( new_url, count + 1 );
            m_get_board_url = url;
            return board;
        }

        // 2ch型の場合、板パスを見てもし一致したら新ホストに移転したと判断して移転テーブルを更新する
        if( is_2ch( url ) ){

            // 板パスを見て一致したら移転したと見なす
            // TODO : 板パスが同じ板が2つ以上あるときどうするか？
            const auto match_path = [&url]( const auto& b ) {
                return is_2ch( b->get_root() ) && url.find( b->get_path_board() + "/" ) != std::string::npos;
            };

            // 全ての板をサーチして移転先の板を探す
            auto it = std::find_if( m_list_board.begin(), m_list_board.end(), match_path );
            if( it != m_list_board.end() ) {
                BoardBase* board = it->get();

                const std::string hostname = MISC::get_hostname( url );
                const std::string& path_board = board->get_path_board();

                // 板移転テーブルを更新
                push_movetable( hostname, path_board, board->get_root(), path_board );

                std::ostringstream ss;
                ss << board->get_name() << '\n'
                   << "旧 URL = " << hostname + path_board << '/' << '\n'
                   << "新 URL  = " << board->url_boardbase() << std::endl;
                MISC::MSG( ss.str() );

                if( m_enable_save_movetable ){

                    //移転テーブル保存
                    save_movetable();

                    // サイドバーに登録されているURL更新
                    CORE::core_set_command( "update_sidebar_item" );
                }

                board = get_board( url, count + 1 );
                m_get_board_url = url;
                return board;
            }
        }

        // 最後が "/" で終わってなかったら足して再帰呼び出し
        if( url[ url.length() -1 ] != '/' ){
            BoardBase* board = get_board( url + "/" , count + 1 );
            m_get_board_url = url;
            return board;
        }
    }

#ifdef _DEBUG            
    std::cout << "Root::get_board: not found url = " << url << std::endl;
#endif
    
    // それでも見つからなかったらNullクラスを返す
    return m_board_null.get();
}


// ローカルキャッシュから板一覧XML読み込み
//
// (注) 板一覧 XML の保存は BBSLIST::BBSListViewMain が行う
//
void Root::load_cache()
{
    clear();

    std::string file_in = CACHE::path_xml_listmain();

#ifdef _DEBUG
    std::cout << "Root::load_cache xml  = " << file_in << std::endl;
#endif    

    std::string xml_bbsmenu;
    if( CACHE::load_rawdata( file_in, xml_bbsmenu ) )
    {
        // Domノードを初期化
        m_xml_document.init( xml_bbsmenu );

        // Domノードの内容からDBに板を登録
        analyze_board_xml();
    }
}


//
// サーバから bbsmenu.html を読み込んで xml に変換開始
//
// 読み終わったらreceive_finish()でXMLに変換して"update_bbslist"コマンド発行
//
void Root::download_bbsmenu()
{
    if( is_loading() ) return;

    clear();
    m_xml_document.clear();
    m_rawdata.reserve( SIZE_OF_RAWDATA );

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = CONFIG::get_url_bbsmenu();
    data.modified = get_date_modified();

    bool send_cookie = true;

    constexpr bool protocol = false;
    const std::string host = MISC::get_hostname( data.url, protocol );
    if( host.find( ".5ch.net" ) != std::string::npos || host.find( ".2ch.net" ) != std::string::npos ) {
        data.agent = CONFIG::get_agent_for2ch();
        if( CONFIG::get_use_proxy_for2ch() ) {
            data.host_proxy = CONFIG::get_proxy_for2ch();
            data.port_proxy = CONFIG::get_proxy_port_for2ch();
            data.basicauth_proxy = CONFIG::get_proxy_basicauth_for2ch();
            data.basicauth = CONFIG::get_proxy_basicauth_for2ch();
            send_cookie = CONFIG::get_send_cookie_to_proxy_for2ch();
        }
    }
    else {
        data.agent = CONFIG::get_agent_for_data();
        if( CONFIG::get_use_proxy_for_data() ) {
            data.host_proxy = CONFIG::get_proxy_for_data();
            data.port_proxy = CONFIG::get_proxy_port_for_data();
            data.basicauth_proxy = CONFIG::get_proxy_basicauth_for_data();
            data.basicauth = CONFIG::get_proxy_basicauth_for_data();
            send_cookie = CONFIG::get_send_cookie_to_proxy_for_data();
        }
    }

    if( send_cookie ) {
        const JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
        data.cookie_for_request = cookie_manager->get_cookie_by_host( data.url );
    }

    start_load( data );
}


//
// bbsmenu 受信中
//
// virtual
void Root::receive_data( std::string_view buf )
{
    m_rawdata.append( buf );
}


//
// bbsmenu 受信完了
//
// virtual
void Root::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Root::receive_finish code = " << get_code() << std::endl;
#endif

    if( get_code() == HTTP_NOT_MODIFIED ){

        std::string msg = get_str_code() + "\n\nサーバー上の板一覧は更新されていません。強制的に再読み込みをしますか？";
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ){
            set_date_modified( std::string() );
            download_bbsmenu();
        }

        return;
    }

    if( ( get_code() == HTTP_MOVED_PERM || get_code() == HTTP_REDIRECT || get_code() == HTTP_PERMANENT_REDIRECT )
            && ! location().empty() ){

        const std::string msg = get_str_code() + "\n\n板一覧が " + location() + " に移転しました。更新しますか？";
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ){
            set_date_modified( std::string() );
            CONFIG::set_url_bbsmenu( location() );
            download_bbsmenu();
        }
        return;
    }

    if( get_code() != HTTP_OK ){

        std::string msg = get_str_code() + "\n\n板一覧の読み込みに失敗したため板一覧は更新されませんでした。\n\nプロキシ設定や板一覧を取得するサーバのアドレスを確認して、ファイルメニューから板一覧の再読み込みをして下さい。\n板一覧取得サーバのアドレスはabout:configで確認出来ます。";
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        MISC::ERRMSG( "bbsmenu load failed : " + get_str_code() );

        CORE::core_set_command( "update_bbslist" );

        return;
    }

    // 文字コードを変換してXML作成
    JDLIB::Iconv libiconv{ Encoding::utf8, Encoding::sjis };
    const std::string& rawdata_utf8 = libiconv.convert( m_rawdata.data(), m_rawdata.size() );
    bbsmenu2xml( rawdata_utf8 );

    if( m_xml_document.hasChildNodes() )
    {
        // データベース更新
        analyze_board_xml();

        // bbslistview更新
        CORE::core_set_command( "update_bbslist" );
    }

    clear();
}



//
// bbsmenu.html -> xml 変換
//
void Root::bbsmenu2xml( const std::string& menu )
{
    if( menu.empty() ) return;

#ifdef _DEBUG
    std::cout << "Root::bbsmenu2xml\n";
#endif

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    // menu のノードツリーを取得( menu がHTMLなので第二引数は true )
    const XML::Document html( menu, true );

    // XML用のノードツリーにルートノードを追加
    m_xml_document.clear();
    XML::Dom* root = m_xml_document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

    // カテゴリの要素 <subdir></subdir>
    XML::Dom* subdir = nullptr;

    // カテゴリの有効/無効
    bool enabled = true;

    // 現在の仕様では HTML > BODY > font[size="2"] の子要素が対象
    // 特定のサイト(2ch.sc、next2ch.net)のbbsmenu.htmlにはfontタグがないため別のタグを使う
    std::list<XML::Dom*> targets = html.getElementsByTagName( "font" );
    if( targets.empty() ) targets = html.getElementsByTagName( "small" );
    if( targets.empty() ) targets = html.getElementsByTagName( "body" );
    if( targets.empty() ) {
        MISC::ERRMSG( "parse error for bbsmenu" );
        return;
    }
    for( const XML::Dom* child : *targets.front() )
    {
        // 要素b( カテゴリ名 )
        if( child->nodeName() == "b" )
        {
            const std::string category = MISC::chref_decode( child->firstChild()->nodeValue() );

            // 追加しないカテゴリ
            if( category == "チャット"
             || category == "ツール類"
             || category == "他のサイト" )
            {
                enabled = false;
                continue;
            }
            else enabled = true;

            // <subdir>
            subdir = root->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
            subdir->setAttribute( "name", category );
        }
        // 要素bに続く要素a( 板URL )
        else if( subdir && enabled && child->nodeName() == "a" )
        {
            const std::string board_name = MISC::chref_decode( child->firstChild()->nodeValue() );
            const std::string url = child->getAttribute( "href" );

            // 板として扱うURLかどうかで要素名を変える
            std::string element_name;
            if( CONFIG::use_link_as_board() ) element_name = "board";
            else if( ( regex.exec( "^(https?:)?//.*/.*/$", url, offset, icase, newline, usemigemo, wchar )
                       && ( is_2ch( url ) || is_machi( url ) ) )
                     || is_JBBS( url )
                     || is_vip2ch( url )
                     || is_open2ch( url )
                     || is_next2ch( url )
                     || is_2chsc( url )
                ) element_name = "board";
            else element_name = "link";

            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, element_name );
            board->setAttribute( "name", board_name );
            board->setAttribute( "url", url );
        }
    }

    root->setAttribute( "date_modified", get_date_modified() );
#ifdef _DEBUG
    std::cout << "modified = " << get_date_modified() << std::endl;
#endif
}


/** @brief XML に含まれる板情報を取り出してデータベースを更新
 *
 * @param[in,out] bbsmenu BBSMENU から取得した板の情報
 */
template<typename T>
void Root::slot_analyze_board_xml( T& bbsmenu )
{
    m_move_info.clear();
    m_analyzing_board_xml = true;
    m_analyzed_path_board.clear();

    const std::list<XML::Dom*> boards = bbsmenu.xml_document().getElementsByTagName( "board" );

    for( const XML::Dom* child : boards ) {

        const std::string name = child->getAttribute( "name" );
        const std::string url = child->getAttribute( "url" );

        // 板情報セット
        set_board( url, name );
    }

    // 移転があった
    if( ! m_move_info.empty() ) {

        SKELETON::EditViewDialog diag( m_move_info, "移転板一覧", false );
        diag.set_default_size( 600, 400 );
        diag.run();

        if( m_enable_save_movetable ) {

            // 移転テーブル保存
            save_movetable();

            // サイドバーに登録されているURL更新
            CORE::core_set_command( "update_sidebar_item" );
        }
    }

    const XML::Dom* root = bbsmenu.xml_document().get_root_element( ROOT_NODE_NAME );
    if( root ) bbsmenu.set_date_modified( root->getAttribute( "date_modified" ) );

    m_analyzing_board_xml = false;
    m_analyzed_path_board.clear();

#ifdef _DEBUG
    std::cout << "Root::slot_analyze_board_xml : date_modified = "
              << bbsmenu.get_date_modified() << std::endl;
#endif
}


/**
 * @brief XML に含まれる板情報を取り出してデータベースを更新
 */
void Root::analyze_board_xml()
{
    slot_analyze_board_xml( *this );
}


//
// 板のタイプを判定
//
int Root::get_board_type( const std::string& url, std::string& root, std::string& path_board ) const
{
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    int type = TYPE_BOARD_UNKNOWN;

    // 2ch
    if( is_2ch( url ) ){

        if( regex.exec( "(https?://[^/]*)(/[^/]*)/$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = regex.str( 2 );

            type = TYPE_BOARD_2CH;
        }
    }

    // JBBS
    else if( is_JBBS( url ) ){

        if( regex.exec( "(https?://[^/]*)(/.*)/(index2?\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = regex.str( 2 );

            type = TYPE_BOARD_JBBS;
        }
    }

    // まち
    else if( is_machi( url ) ){

        if( regex.exec( "(https?://[^/]*)(/[^/]*)/(index2?\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = regex.str( 2 );

            type = TYPE_BOARD_MACHI;
        }
    }

    // vipサービス
    else if( is_vip2ch( url ) ){

        if( regex.exec( "(https?://[^/]*)(/[^/]*)/$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = regex.str( 2 );

            type = TYPE_BOARD_2CH_COMPATI;
        }
    }

    // ローカルファイル
    else if( is_local( url ) ){

        root = "file://";
        path_board = "/local";

        type = TYPE_BOARD_LOCAL;
    }

    // その他は互換型
    else{

        if( regex.exec( "(https?://.*)(/[^/]*)/([^\\.]+\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = regex.str( 2 );

            if( is_open2ch( url ) ) type = TYPE_BOARD_OPEN2CH;
            else if( is_next2ch( url ) ) type = TYPE_BOARD_NEXT2CH;
            else if( is_2chsc( url ) ) type = TYPE_BOARD_2CHSC;
            else type = TYPE_BOARD_2CH_COMPATI;
        }
    }

    return type;
}


//
// 板のタイプを簡易判定
//
// 通常の get_board_type() で得た root を再判定するときに使用する ( 板移転処理用 )
//
int Root::get_board_type( const std::string& root ) const
{
    int type = TYPE_BOARD_UNKNOWN;

    // 2ch
    if( is_2ch( root ) )
        type = TYPE_BOARD_2CH;

    // JBBS
    else if( is_JBBS( root ) )
        type = TYPE_BOARD_JBBS;

    // まち
    else if( is_machi( root ) )
        type = TYPE_BOARD_MACHI;

    // おーぷん２ちゃんねる
    else if( is_open2ch( root ) ) {
        type = TYPE_BOARD_OPEN2CH;
    }
    // Next2ch
    else if( is_next2ch( root ) ) {
        type = TYPE_BOARD_NEXT2CH;
    }
    // 2ch.sc
    else if( is_2chsc( root ) ) {
        type = TYPE_BOARD_2CHSC;
    }
    // ローカルファイル
    else if( is_local( root ) )
        type = TYPE_BOARD_LOCAL;

    // その他は互換型
    else
        type = TYPE_BOARD_2CH_COMPATI;
    
    return type;
}


//
// 板のタイプに合わせて板情報をセット
// ついでに移転の自動判定と移転処理もおこなう
//
bool Root::set_board( const std::string& url, const std::string& name, const std::string& basicauth )
{
#ifdef _SHOW_BOARD
    std::cout << "Root::set_board " << url << " " << name << std::endl;
#endif

    std::string real_url;
    std::string root;
    std::string path_board;

    // scheme省略の場合はbbsmenuのURLから補う
    if( url.rfind( "//", 0 ) == 0 ) {
        const std::string menu_url = CONFIG::get_url_bbsmenu();
        const std::size_t pos = menu_url.find( "://" );
        if( pos != std::string::npos ) real_url = menu_url.substr( 0, pos + 1 );
    }
    real_url.append( url );

    // タイプ判定
    const int type = get_board_type( real_url, root, path_board );
    if( type == TYPE_BOARD_UNKNOWN ) return false;

    // 移転チェック
    BoardBase* board = nullptr;
    const int state = is_moved( root, path_board, name, &board );

#ifdef _SHOW_BOARD
    std::cout << "root = " << root << " path_board = " << path_board
              << " basicauth = " << basicauth <<" type = " << type << " state = " << state << std::endl;
#endif

    // 新板登録
    if( state == BOARD_NEW ){

        auto uniq = DBTREE::BoardFactory( type, root, path_board, name, basicauth );
        if( uniq ){
            board = uniq.get();
            m_list_board.push_back( std::move( uniq ) );
            if( m_analyzing_board_xml ) m_analyzed_path_board.insert( path_board );
        }
    }

    // 移転処理
    else if( state == BOARD_MOVED ){

        // XML解析中に、
        // <board name="ニュース実況+" url="http://anchorage.2ch.net/liveplus/" />
        // <board name="ニュース実況+" url="http://gimpo.2ch.net/liveplus/" />
        // のように同じ板で異なるアドレスが現れた場合は、移転処理をせずにキャッシュが存在する方のアドレスを残す
        if( m_analyzing_board_xml && m_analyzed_path_board.find( path_board ) != m_analyzed_path_board.end() ){

            std::string tmp_msg = "Root::set_board : The XML file is broken !\n";
            tmp_msg += url + " has already been registered as " + board->url_boardbase();
            MISC::ERRMSG( tmp_msg );

            const std::string path1 = CACHE::path_board_root_fast( board->url_boardbase() );
            const std::string path2 = CACHE::path_board_root_fast( real_url );

#ifdef _DEBUG
            std::cout << "path1 = " << path1 << std::endl
                      << "path2 = " << path2 << std::endl;
#endif
            // キャッシュが存在する方を正しいアドレスとする
            if( CACHE::file_exists( path1 ) == CACHE::EXIST_DIR ){
#ifdef _DEBUG
                std::cout << "path1 exists\n";
#endif
                // 既に登録済みなので何もしない
            }
            else if( CACHE::file_exists( path2 ) == CACHE::EXIST_DIR ){
#ifdef _DEBUG
                std::cout << "path2 exists\n";
#endif
                // url の方を登録する
                board->update_url( root, path_board ); 
            }
        }

        // 移転処理実行
        else{

            if( ! exec_move_board( board,
                                   board->get_root(),
                                   board->get_path_board(),
                                   root,
                                   path_board ) ) return false;

            if( m_analyzing_board_xml ) m_analyzed_path_board.insert( path_board );
        }
    }

    return true;
}



//
// (明示的に)板移転
//
bool Root::move_board( const std::string& url_old, const std::string& url_new )
{
    if( url_old == url_new ) return false;

#ifdef _DEBUG
    std::cout << "Root::move_board " << url_old << " -> " << url_new << std::endl;
#endif

    m_move_info = std::string();

    std::string root;
    std::string path_board;

    BoardBase * board = get_board( url_old );
    if( ! board ) return false;

    // タイプ判定
    int type = get_board_type( url_new, root, path_board );
    if( type == TYPE_BOARD_UNKNOWN ) return false;

    if( ! exec_move_board( board,
                           board->get_root(),
                           board->get_path_board(),
                           std::move( root ),
                           std::move( path_board )
            ) ) return false;

    // キャッシュを移動した
    if( ! m_move_info.empty() ){

        if( m_enable_save_movetable ){

            save_movetable();

            // サイドバーに登録されているURL更新
            CORE::core_set_command( "update_sidebar_item" );
        }
    }

    return true;
}




//
// 板移転処理実行
//
// 引数を参照渡し(const std::string&)するとDB更新で値が変わるため値渡しする
//
bool Root::exec_move_board( BoardBase* board,
                            std::string old_root,
                            std::string old_path_board,
                            std::string new_root,
                            std::string new_path_board )
{
    if( ! board ) return false;

#ifdef _SHOW_BOARD
    std::cout << "Root::exec_move_board\n";
    std::cout << old_root << old_path_board << " -> " << new_root <<  new_path_board << std::endl;
#endif

    if( old_root == new_root && old_path_board == new_path_board ){

        std::string errmsg = "移転元のアドレスと移転先のアドレスが同じです\n\n"
        + old_root + old_path_board + " → " + new_root +  new_path_board;

        SKELETON::MsgDiag mdiag( nullptr, errmsg, false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        return false;
    }

    const std::string old_url = board->url_boardbase();
    std::string old_path = CACHE::path_board_root( old_url );

    // DB更新
    board->update_url( new_root, new_path_board ); 

    std::string new_url = board->url_boardbase();
    std::string new_path = CACHE::path_board_root( new_url );

    std::ostringstream ss;
    ss << board->get_name() << std::endl
       << " 旧 URL = " << old_url << std::endl
       << " 新 URL = " << new_url << std::endl;
    MISC::MSG( ss.str() );

    m_get_board_url = std::string();
    m_get_board = nullptr;

    // もしキャッシュが存在したら移動して移転テーブル更新
    if( CACHE::file_exists( old_path ) == CACHE::EXIST_DIR ){

        // キャッシュがある場合はダイアログに表示
        m_move_info += ss.str() + "\n";

        if( new_path != old_path ) {

            // 移動先に同名のファイルかフォルダ何かあったらリネームしてバックアップをとっておく
            if( CACHE::file_exists( new_path ) != CACHE::EXIST_ERROR ) {

                std::string path_bk = new_path.substr( 0, new_path.length() - 1 ) + "_bk/";
                if( rename( new_path.c_str(), path_bk.c_str() ) == 0 ) {
                    MISC::MSG( "rename : " +  new_path + " -> " + path_bk );
                }
                else MISC::ERRMSG( "can't rename " + new_path + " to " + path_bk );
            }

            // キャッシュ移動
            if( CACHE::mkdir_parent_of_board( new_url ) ) {

                if( rename( old_path.c_str(), new_path.c_str() ) == 0 ) {
                    MISC::MSG( "cache was moved : " +  old_path + " -> " + new_path );
                }
                else MISC::ERRMSG( "can't move cache from " + old_path + " to " + new_path );
            }
        }

#ifdef _DEBUG
        std::cout << "movetable was updated.\n"
                  << "old_root = " << old_root << std::endl
                  << "new_root = " << new_root << std::endl
                  << "old_path_board = " << old_path_board << std::endl
                  << "new_path_board = " << new_path_board << std::endl;
#endif
        push_movetable( std::move( old_root ), std::move( old_path_board ),
                        std::move( new_root ), std::move( new_path_board ) );

        // この板に関連する表示中のviewのURLを更新
        CORE::core_set_command( "update_url", old_url, new_url );
    }

    return true;
}


//
// 板移転テーブルを更新
//
void Root::push_movetable( std::string old_root,
                           std::string old_path_board,
                           std::string new_root,
                           std::string new_path_board )
{
#ifdef _DEBUG
    std::cout << "Root::push_movetable : " << old_root << old_path_board << " -> " << new_root << new_path_board << std::endl;
#endif            

    if( old_root == new_root && old_path_board == new_path_board ){

        std::string errmsg = "移転元のアドレスと移転先のアドレスが同じです (Root::push_movetable)\n\n"
        + old_root + old_path_board + " → " + new_root +  new_path_board;

        SKELETON::MsgDiag mdiag( nullptr, errmsg, false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        return;
    }

    std::string str;

    // new_root, new_path_board, old_root, old_path が過去に登録済みなら
    // 消す、または修正する(パフォーマンス向上、循環防止)
    std::list< MOVETABLE >::iterator it_move = m_movetable.begin();
    for( ; it_move != m_movetable.end(); ){

#ifdef _DEBUG
        std::cout << "size = " << m_movetable.size() << " "
                  << it_move->old_root << it_move->old_path_board << "/ -> " << it_move->new_root << std::endl;
#endif

        if(
            ( it_move->old_root == new_root
              && it_move->old_path_board == new_path_board )

            ||

            ( it_move->old_root == old_root
              && it_move->old_path_board == old_path_board )

            ){

            const std::string str_tmp = "削除: " + it_move->old_root + it_move->old_path_board + "/ -> "
                                        + it_move->new_root + "\n";
#ifdef _DEBUG
            std::cout << str_tmp << std::endl;
#endif
            str += str_tmp;

            it_move = m_movetable.erase( it_move );
            continue;
        }

        // 移転先を最新にする
        //
        // (注意) old_root == new_root かつ old_path_board == new_path_board のとき
        // erase した内容と push_back した内容が同じになるので無限ループに落ちる
        else if(

            it_move->new_root == old_root
            && it_move->new_path_board == old_path_board ){

            MOVETABLE movetable = *it_move;
            movetable.new_root = new_root;
            movetable.new_path_board = new_path_board;

            const std::string str_tmp = "更新: " + it_move->old_root + it_move->old_path_board + "/ -> "
                                        + it_move->new_root + " => " + movetable.old_root + movetable.old_path_board
                                        + "/ -> " + movetable.new_root + "\n";
#ifdef _DEBUG
            std::cout << str_tmp << std::endl;
#endif
            str += str_tmp;

            it_move = m_movetable.erase( it_move );
            m_movetable.push_back( std::move( movetable ) );
            continue;
        }

        ++it_move;
    }

    if( ! str.empty() ) MISC::MSG( "\n" + str );

    MOVETABLE movetable;
    movetable.old_root = std::move( old_root );
    movetable.old_path_board = std::move( old_path_board );
    movetable.new_root = std::move( new_root );
    movetable.new_path_board = std::move( new_path_board );
    m_movetable.push_back( std::move( movetable ) );
}



//
// 板をデータベースから削除
//
bool Root::remove_board( const std::string& url )
{
#ifdef _SHOW_BOARD
    std::cout << "Root::remove_board " << url << std::endl;
#endif

    BoardBase * board = get_board( url );
    if( ! board ) return false;

#ifdef _SHOW_BOARD
    std::cout << "found\n"
              << "root = " << board->get_root() << std::endl
              << "path = " << board->get_path_board() << std::endl
              << "name = " << board->get_name() << std::endl;
#endif

    // この板に関連するビューを全て閉じる
    // delete board する前に全て閉じないとセグフォの原因となるので注意
    CORE::core_set_command( "close_board", url );

    // 削除対象はアドレスで判定する
    m_list_board.remove_if( [board]( const auto& b ) { return board == b.get(); } );

    m_get_board_url = std::string();
    m_get_board = nullptr;

    return true;
}



//
// 板が移転したどうかチェックする関数
//
// 戻り値 :
//
// BOARD_EXISTS : DBに登録されていて移転していない
// BOARD_MOVED  : DBに登録されていて移転した
// BOARD_NEW    : DBに登録されていない
//
// 移転したなら board_old に古いデータが入って戻る
//
int Root::is_moved( const std::string& root,
                     const std::string& path_board,
                     const std::string& name,
                     BoardBase** board_old )
{
    for( auto& board : m_list_board ) {

        if( board->get_path_board() == path_board ){

            // 既にリストに登録されてる
            if( board->get_root() == root ) return BOARD_EXISTS;

            // 名前が同じで、サイトが同じなら移転
            if( board->get_name() == name
                    && get_board_type( board->get_root() ) == get_board_type( root ) ){
                *board_old = board.get();
                return BOARD_MOVED;
            }
        }
    }
    
    return BOARD_NEW;
}



//
// etc.txtから外部板情報を読み込み
//
// etc.txt(Navi2ch互換) を読み込んで外部板情報( etcboardinfo.h )作成およびデータベース登録
//
void Root::load_etc()
{
    m_etcboards.clear();

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    std::string file_etctxt = CACHE::path_etcboard();
    std::string etcboard;
    if( CACHE::load_rawdata( file_etctxt, etcboard ) )
    {
        std::list< std::string > list_etc = MISC::get_lines( etcboard );
        list_etc = MISC::remove_commentline_from_list( list_etc );
        list_etc = MISC::remove_space_from_list( list_etc );
        list_etc = MISC::remove_nullline_from_list( list_etc );
        std::list< std::string >::iterator it;
        for( it = list_etc.begin(); it != list_etc.end(); ++it ){

            DBTREE::ETCBOARDINFO info;

            // 名前
            info.name = *( it++ );
            if( it == list_etc.end() ) break;

            // url
            info.url = *( it++ );
            if( it == list_etc.end() ) break;

            // basic認証
            if( regex.exec( "(https?://)([^/]+:[^/]+@)(.+)$" , info.url, offset, icase, newline, usemigemo, wchar ) )
            {
                info.basicauth = regex.str( 2 );
                info.basicauth.pop_back();
                info.url = regex.str( 1 ) + regex.str( 3 );
            }

            // board id
            info.boardid = *( it );

#ifdef _DEBUG
            std::cout << "etc board : name = " << info.name << std::endl
                      << "url = " << info.url << std::endl
                      << "id:passwd = " <<info. basicauth << std::endl
                      << "boardid = " << info.boardid << std::endl << std::endl;
#endif

            // DBに登録
            if( set_board( info.url, info.name, info.basicauth ) )
            {
#ifdef _DEBUG
                std::cout << "added.";
#endif
                m_etcboards.push_back( info );
            }
            else MISC::ERRMSG( "failed to add " + info.name );
        }
    }

#ifdef _DEBUG
    std::cout << "number of etc boards：" << m_etcboards.size() << std::endl;
#endif
}


//
// 外部板追加
//
bool Root::add_etc( const std::string& url, const std::string& name, const std::string& basicauth, const std::string& id )
{
#ifdef _DEBUG
    std::cout << "Root::add_etc url = " << url << " name = " << name
              << " auth = " << basicauth << std::endl;
#endif 

    DBTREE::ETCBOARDINFO info;
    info.url = url;
    info.name = name;
    info.basicauth = basicauth;
    info.boardid = id;

    if( set_board( info.url, info.name, info.basicauth ) )
    {

#ifdef _DEBUG
        std::cout << "set ok\n";
#endif
        m_etcboards.push_front( info );
        return true;
    }
    else MISC::ERRMSG( "failed to add " + info.name );

    return false;
}


// 外部板更新
bool Root::move_etc( const std::string& url_old, const std::string& url_new,
                     const std::string& name_old, const std::string& name_new,
                     const std::string& basicauth, const std::string& boardid )
{
    BoardBase * board = get_board( url_old );
    if( ! board ) return false;

    using Info = DBTREE::ETCBOARDINFO;
    const auto find_info = [&url_old, &name_old]( const Info& i ) { return i.url == url_old && i.name == name_old; };
    auto it = std::find_if( m_etcboards.begin(), m_etcboards.end(), find_info );
    if( it == m_etcboards.end() ) return false;

#ifdef _DEBUG
    std::cout << "Root::move_etc " << url_old << " -> " << url_new << std::endl
              << name_old << " -> " << name_new << std::endl
              << board->get_basicauth() << " -> " << basicauth << std::endl;
#endif 

    // 移転処理
    if( url_old != url_new ){
        (*it).url = url_new;
        move_board( url_old, url_new );
    }

    // 名前変更
    (*it).name = name_new;
    board->update_name( name_new );

    // BASIC認証変更
    (*it).basicauth = basicauth;
    board->set_basicauth( basicauth );

    // ID更新
    (*it).boardid = boardid;

    save_etc();

    return true;
}


//
// 外部板削除
//
bool Root::remove_etc( const std::string& url, const std::string& name )
{
#ifdef _DEBUG
    std::cout << "Root::remove_etc url = " << url << " name = " << name << std::endl;
#endif 

    if( m_etcboards.empty() ) return false;

    const auto find_info = [&]( const DBTREE::ETCBOARDINFO& i ) { return i.url == url && i.name == name; };
    const auto it = std::find_if( m_etcboards.cbegin(), m_etcboards.cend(), find_info );
    if( it != m_etcboards.cend() ) {
#ifdef _DEBUG
        std::cout << "found\n";
#endif 
        remove_board( url );
        m_etcboards.erase( it );
        return true;
    }

    return false;
}


//
// 外部板保存
//
// 外部板情報から etc.txt(Navi2ch互換)作成
//
void Root::save_etc()
{
    // m_etcboards が空でも実行する

#ifdef _DEBUG
    std::cout << "Root::save_etc\n";
#endif 

    std::string etcboard;

    for( const DBTREE::ETCBOARDINFO& info : m_etcboards ) {

        etcboard += info.name + "\n";
        if( info.basicauth.empty() ) etcboard += info.url + "\n";
        else{

            const std::size_t i = info.url.find( "://" );
            if( i != std::string::npos ){
                etcboard += info.url.substr( 0, i+3 ) + info.basicauth + "@" + info.url.substr( i+3 ) + "\n";
            }
            else etcboard += info.url + "\n";

        }
        etcboard += info.boardid + "\n";
    }

    std::string file_etctxt = CACHE::path_etcboard();
    if( ! CACHE::save_rawdata( file_etctxt, etcboard ) ){
        MISC::ERRMSG( "failed to save " + file_etctxt );
    }

    // BASIC認証のパスワード対策
    else chmod( file_etctxt.c_str(), S_IWUSR | S_IRUSR );

#ifdef _DEBUG
    std::cout << etcboard << std::endl;
#endif 
}


//
// 移転テーブル読み込み
//
void Root::load_movetable()
{
#ifdef _DEBUG
    std::cout << "Root::load_movetable\n";
#endif 
   
    std::string file_move = CACHE::path_movetable();
    std::string movetable_rawdata;
    if( CACHE::load_rawdata( file_move, movetable_rawdata ) ){

        std::list< std::string > list_table = MISC::get_lines( movetable_rawdata );
        for( const std::string& line : list_table ) {

            std::list<std::string> tokens = MISC::split_line( line );

            if( tokens.size() == 3 // 旧形式
                || tokens.size() == 4 ){

                auto move_it = std::make_move_iterator( tokens.begin() );

                MOVETABLE movetable;
                movetable.old_root = *(move_it++);
                movetable.new_root = *(move_it++);
                movetable.old_path_board = *(move_it++);
                if( tokens.size() == 4 ) movetable.new_path_board = *(move_it++);
                else movetable.new_path_board = movetable.old_path_board;
                m_movetable.push_back( movetable );
            }
        }
    }

#ifdef _DEBUG
    std::cout << "MOVETABLE : \n";
    for( const MOVETABLE& t : m_movetable ) {
        std::cout << t.old_root << t.old_path_board << " -> " << t.new_root << t.new_path_board << std::endl;
    }
#endif
}



//
// 板が移転したかチェックする
//
// 移転した時は移転後のURLを返す
//
std::string Root::is_board_moved( const std::string& url ) // 簡易版
{
    std::string old_root;
    std::string old_path_board;
    std::string new_root;
    std::string new_path_board;
    return is_board_moved( url, old_root, old_path_board, new_root, new_path_board );
}


std::string Root::is_board_moved( const std::string& url,
                                  std::string& old_root,
                                  std::string& old_path_board,
                                  std::string& new_root,
                                  std::string& new_path_board,
                                  const int count )
{
    const int max_count = 50;

#ifdef _DEBUG            
    std::cout << "Root::is_board_moved count = " << count << " url = " << url << std::endl;
#endif

    // 移転テーブルが循環している場合 2ch 型ならテーブルを修復する
    if( count > max_count ){

        if( is_2ch( url ) ){

            // 板パスを見て一致したら移転したと見なす
            // TODO : 板パスが同じ板が2つ以上あるときどうするか？
            const auto match_path = [&url]( const auto& b ) {
                return is_2ch( b->get_root() ) && url.find( b->get_path_board() + "/" ) != std::string::npos;
            };

            // 板の最新のrootとpathを取得する
            auto it_board = std::find_if( m_list_board.cbegin(), m_list_board.cend(), match_path );
            if( it_board != m_list_board.cend() ) {
                const BoardBase* board = it_board->get();

                std::string msg = "移転テーブルが破損していたので修復しました\n";

                for( auto it = m_movetable.begin(); it != m_movetable.end(); ) {

                    if( is_2ch( it->old_root )
                            && url.find( it->old_path_board + "/" ) != std::string::npos ) {

                        // 最新のrootとpathに変更する
                        it->new_root = board->get_root();
                        it->new_path_board = board->get_path_board();

                        const std::string from_to = it->old_root + it->old_path_board + "/ -> "
                                                    + board->url_boardbase() + "\n";

                        // url -> url の形となった場合は消す
                        if( it->old_root == it->new_root
                                && it->old_path_board == it->new_path_board ) {

                            msg.append( "削除: " );
                            msg.append( from_to );
                            it = m_movetable.erase( it );
                            continue;
                        }

                        msg.append( "更新: " );
                        msg.append( from_to );
                    }

                    ++it;
                }

                MISC::MSG( msg );

                if( m_enable_save_movetable ){

                    //移転テーブル保存
                    save_movetable();

                    // サイドバーに登録されているURL更新
                    CORE::core_set_command( "update_sidebar_item" );
                }

                // 改めてもう一度実行
                return is_board_moved( url, old_root, old_path_board, new_root, new_path_board, 0 );
            }
        }

        return std::string();
    }

    // 移転テーブルを検索
    auto it = std::find_if( m_movetable.cbegin(), m_movetable.cend(),
                            [&url]( const MOVETABLE& table )
                            { return url.rfind( table.old_root, 0 ) == 0
                                  && url.find( table.old_path_board + "/" ) != std::string::npos; } );

    if( it != m_movetable.cend() ) {

        const std::string new_url = it->new_root + it->new_path_board + "/";

        old_root = it->old_root;
        old_path_board = it->old_path_board;
        new_root = it->new_root;
        new_path_board = it->new_path_board;

        std::string old_root_bkup = old_root;
        std::string old_path_board_bkup = old_path_board;

#ifdef _DEBUG
        std::cout << "count = " << count << " : " << url << " is moved to " << new_url << std::endl;
#endif

        // 連鎖的に検索
        std::string ret_url = is_board_moved( new_url, old_root, old_path_board, new_root, new_path_board, count +1 );

        // old_root と old_path_board が書き換わっているので戻しておく
        //
        // (注意) もし再帰呼び出ししたis_board_moved() の中で m_movetable.erase を実行すると
        // table は無効になるので、以前の様に
        // old_root = table.old_root;
        // old_path_board = table.old_path_board;
        // と table を使ってold_rootなどに代入するとメモリを破壊してセグフォになる時がある
        // (イテレータのメモリが残っていればセグフォにならないので完全に運まかせ)
        // 次にJDを起動したときは再帰呼び出ししたis_board_moved() の中の save_movetable()で
        // 移転テーブルは更新済みで落ちないのでタチが悪い
        old_root = std::move( old_root_bkup );
        old_path_board = std::move( old_path_board_bkup );

        return ret_url;
    }

    if( count ) return url; // 再起呼び出しの場合

    return std::string();
}



// 全板の情報ファイル読み込み
void Root::read_boardinfo_all()
{
    for( auto& b : m_list_board ) b->read_info();
}



// 全スレ情報の保存
void Root::save_articleinfo_all()
{
#ifdef _DEBUG
    std::cout << "Root::save_articleinfo_all\n";
#endif

    for( auto& b : m_list_board ) b->save_articleinfo_all();

#ifdef _DEBUG
    std::cout << "end\n";
#endif
}



// 全ログ検索
void Root::search_cache( std::vector< ArticleBase* >& list_article,
                         const std::string& query, const bool mode_or, const bool bm, const std::atomic<bool>& stop )
{
    for( auto& b : m_list_board ) {
        b->search_cache( list_article, query, mode_or, bm, stop );
        if( stop.load( std::memory_order_acquire ) ) break;
    }
}


// 全てのスレの書き込み履歴削除
void Root::clear_all_post_history()
{
    for( auto& b : m_list_board ) b->clear_all_post_history();
}


//
// 移転テーブル保存
//
void Root::save_movetable()
{
#ifdef _DEBUG
    std::cout << "Root::save_movetable\n";
#endif    
    std::string file_move = CACHE::path_movetable();
    std::ostringstream movetable;

    for( const MOVETABLE& t : m_movetable ) {

        movetable << t.old_root << " "
                  << t.new_root << " "
                  << t.old_path_board;

        // 新形式
        if( ! t.new_path_board.empty()
            && t.old_path_board != t.new_path_board ) movetable << " " << t.new_path_board;

        movetable << std::endl;
    }

#ifdef _DEBUG
    std::cout << movetable.str();
#endif
    
    CACHE::save_rawdata( file_move, movetable.str() );
}



/** @brief 2ch.net or 5ch.net or bbspink.comのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_2ch( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    if( ( MISC::ends_with( hostname, ".2ch.net" ) && hostname != "info.2ch.net" )
        // サブドメイン無しのURLに対応する
        || ( MISC::ends_with( hostname, "5ch.net" ) && hostname != "info.5ch.net" )
        || MISC::ends_with( hostname, ".bbspink.com" ) ) return true;

    return false;
}



/** @brief したらば掲示板(JBBS)型のURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_JBBS( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return ( hostname == "jbbs.shitaraba.net"
            || hostname == "jbbs.shitaraba.com"
            || hostname == "jbbs.livedoor.jp" );
}


/** @brief まちBBS型のURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_machi( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return MISC::ends_with( hostname, "machi.to" );
}


/** @brief VIPサービスのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_vip2ch( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return MISC::ends_with( hostname, ".vip2ch.com" );
}


/** @brief おーぷん２ちゃんねるのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_open2ch( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return MISC::ends_with( hostname, "open2ch.net" );
}


/** @brief Next2chのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_next2ch( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return hostname == "next2ch.net";
}


/** @brief 2ch.scのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_2chsc( std::string_view url )
{
    constexpr bool protocol = false;
    const std::string hostname = MISC::get_hostname( url, protocol );

    return ( MISC::ends_with( hostname, "2ch.sc" ) && hostname != "info.2ch.sc" );
}


/** @brief ローカルファイルのURLかどうか
 *
 * @param[in] url チェック対象
 * @return マッチしたらtrueを返す
 */
bool Root::is_local( std::string_view url )
{
    if( url.find( "file://" ) != std::string_view::npos ) return true;

    return false;
}


//
// スレあぼーん情報を更新した時、全boardbaseクラスに対応するスレ一覧の表示を更新させる
//
// CONFIG::set_abone_number_thread() などでグローバル設定をした後などに呼び出す
//
void Root::update_abone_thread()
{
    for( auto& b : m_list_board ) b->update_abone_thread( true );
}



//
// 全boardbaseクラスに、それに属する全articlebaseクラスのあぼーん状態の更新をさせる
//
void Root::update_abone_all_article()
{
    for( auto& b : m_list_board ) b->update_abone_all_article();
}


//
// 全boardbaseクラスに、それに属する全articlebaseクラスの書き込み時間とスレ立て時間の文字列をリセットさせる
//
void Root::reset_all_since_date()
{
    for( auto& b : m_list_board ) b->reset_all_since_date();
}

void Root::reset_all_write_date()
{
    for( auto& b : m_list_board ) b->reset_all_write_date();
}

void Root::reset_all_access_date()
{
    for( auto& b : m_list_board ) b->reset_all_access_date();
}


/** Rootに登録されたBBSMenuを返す
 *
 * @details URLに一致するBBSMenuがないときはNull objectを返す
 * @param[in] url BBSMENUのURL
 * @return urlのBBSMENU
 */
BBSMenu* Root::get_bbsmenu( std::string_view url )
{
    if( ! url.empty() ) {
        auto it = std::find_if( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                                [url]( const BBSMenu& b ) { return b.equals( url ); } );
        if( it != m_list_bbsmenu.end() ) {
            return std::addressof( *it );
        }
    }
    return m_bbsmenu_null.get();
}


/** @brief bbsmenu/bbsmenu.txt から外部BBSMENU情報を読み込む
 *
 * @details bbsmenu/bbsmeu.txt を読み込んで外部BBSMENU情報( bbsmenu.h )作成およびデータベース登録。
 * Navi2chと互換はない。
 */
void Root::load_bbsmenu()
{
    m_list_bbsmenu.clear();

    const std::string bbsmenu_txt = CACHE::path_bbsmenu();
    std::string bbsmenu_data;
    if( ! CACHE::load_rawdata( bbsmenu_txt, bbsmenu_data ) ) return;

    // ファイル内容から使わない部分を省く
    std::list<std::string> list_bbsmenu = MISC::get_lines( bbsmenu_data );
    for( auto it = list_bbsmenu.begin(); it != list_bbsmenu.end(); ) {

        // trim spaces
        std::string tmp_str = MISC::utf8_trim( *it );
        // remove empty line or comment line
        if( tmp_str.empty() || tmp_str[0] == '#' ) {
            it = list_bbsmenu.erase( it );
            continue;
        }
        // update line
        *it = std::move( tmp_str );
        ++it;
    }

    // 文字列の行が不足した場合は追加処理を打ち切る
    for( auto it = list_bbsmenu.begin(); it != list_bbsmenu.end(); ++it ) {

        std::string& name = *it;
        if( ++it == list_bbsmenu.end() ) break;

        std::string& url = *it;
        if( ++it == list_bbsmenu.end() ) break;

        // 未使用データ
        if( it == list_bbsmenu.end() ) break;

#ifdef _DEBUG
        std::cout << "Root::load_bbsmenu: name = " << name << ", url = " << url << std::endl;
#endif
        // 設定ファイルから読み込むときは重複チェックしない
        BBSMenu& bbsmenu = m_list_bbsmenu.emplace_back( url, name );
        bbsmenu.sig_analyze_board_xml().connect( sigc::mem_fun( *this, &Root::slot_analyze_board_xml<BBSMenu> ) );

        bbsmenu.load_cache();
    }
}


/** Rootに外部BBSMENUを追加する
 *
 * @details URLが重複していたら追加せず失敗(false)を返す
 * @param[in] url  BBSMENUのURL
 * @param[in] name BBSMENUの名前
 * @return 追加に成功したらtrue
 */
bool Root::add_bbsmenu( const std::string& url, const std::string& name )
{
#ifdef _DEBUG
    std::cout << "Root::add_bbsmenu url = " << url << " name = " << name << std::endl;
#endif

    auto it = std::find_if( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                            [&url]( const BBSMenu& b ) { return b.equals( url ); } );
    if( it == m_list_bbsmenu.end() ) {
        auto& bbsmenu = m_list_bbsmenu.emplace_front( url, name );
        bbsmenu.sig_analyze_board_xml().connect( sigc::mem_fun( *this, &Root::slot_analyze_board_xml<BBSMenu> ) );
        return true;
    }

    MISC::ERRMSG( "BBSMENU: Failed to add " + url + " : " + name );
    return false;
}


/** 外部BBSMENUを更新する
 *
 * @param[in] url_old  現在のURL
 * @param[in] url_new  新しいURL
 * @param[in] name_old 現在の名前
 * @param[in] name_new 新しい名前
 * @return 更新に成功したらtrue
 */
bool Root::move_bbsmenu( const std::string& url_old, const std::string& url_new,
                         const std::string& name_old, const std::string& name_new )
{
    decltype( m_list_bbsmenu.begin() ) it;
    if( url_old == url_new ) {
        // 新旧でデータが同じかチェックしてURLと名前が同一なら成功を返す
        if( name_old == name_new ) return true;

        // 名前を入れ替えるBBSMenuを探す
        it = std::find_if( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                           [&url_old, &name_old]( const BBSMenu& b ) { return b.equals( url_old, name_old ); } );
    }
    else {
        // 新しいURLが登録済みなら失敗を返す
        if( std::any_of( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                         [&url_new]( const BBSMenu& b ) { return b.equals( url_new ); } ) ) return false;

        // URLと名前を入れ替えるBBSMenuを探す
        it = std::find_if( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                           [&url_old]( const BBSMenu& b ) { return b.equals( url_old ); } );
    }

    // 登録されていないときは失敗を返す
    if( it == m_list_bbsmenu.end() ) return false;

    // URLと名前を入れ替えて構築したデータをリセットする
    it->reset( url_new, name_new );
    // キャッシュディレクトリの bbsmenu/ は Root::save_bbsmenu() で作成する
    save_bbsmenu();
    it->download_bbsmenu();

    return true;
}


/** @brief 外部BBSMENUを削除する
 *
 * @param[in] url  BBSMENUのURL
 * @param[in] name BBSMENUの名前
 * @return 削除に成功したらtrueを返す
 */
bool Root::remove_bbsmenu( const std::string& url, const std::string& name )
{
#ifdef _DEBUG
    std::cout << "Root::remove_bbsmenu url = " << url << " name = " << name << std::endl;
#endif
    // リストが空のときは失敗を返す
    if( m_list_bbsmenu.empty() ) return false;

    auto it = std::find_if( m_list_bbsmenu.begin(), m_list_bbsmenu.end(),
                            [&url, &name]( const BBSMenu& b ) { return b.equals( url, name ); } );
    if( it != m_list_bbsmenu.end() ) {
        m_list_bbsmenu.erase( it );
        return true;
    }

    return false;
}


/** @brief 外部BBSMENUのURLリストを保存 (キャッシュディレクトリの bbsmenu/ 作成)
 *
 * @details BBSMenuのオブジェクトから bbsmenu.txt を作成する。
 * Root::save_etc() を参考にしているがNavi2chとは関係ない。
 * m_list_bbsmenu が空でもデータを更新するため保存する。
 */
void Root::save_bbsmenu()
{
    std::string buf;

    for( const DBTREE::BBSMenu& bbsmenu : m_list_bbsmenu ) {

        buf.append( bbsmenu.get_name() );
        buf.push_back( '\n' );
        buf.append( bbsmenu.get_url() );
        buf.push_back( '\n' );
        buf.append( bbsmenu.path_bbsmenu_boards_xml() );
        buf.push_back( '\n' );
    }

    if( const std::string path_root = CACHE::path_bbsmenu_root();
            ! CACHE::jdmkdir( path_root ) ) {
        MISC::ERRMSG( "Can't create " + path_root );
        return;
    }

    // save_rawdata() は保存に失敗したとき、または空データを保存(空ファイルを作成)したときに 0 を返す
    // buf の長さが 0 つまり空データときはエラーを表示しないようにする
    if( const std::string path = CACHE::path_bbsmenu();
            ! CACHE::save_rawdata( path, buf ) && ! buf.empty() ) {
        MISC::ERRMSG( "Failed to save " + path );
    }

#ifdef _DEBUG
    std::cout << "Root::save_bbsmenu : bbsmenu data = " << buf << std::endl;
#endif
}
