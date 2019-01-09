// ライセンス: GPL2

//#define _DEBUG
//#define _SHOW_GETBOARD
//#define _SHOW_BOARD
//#define _TEST_CACHE
#include "jddebug.h"

#include "root.h"
#include "boardfactory.h"
#include "boardbase.h"
#include "articlebase.h"

#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"

#include "skeleton/editviewdialog.h"
#include "skeleton/msgdiag.h"

#include "type.h"
#include "command.h"
#include "config/globalconf.h"
#include "cache.h"
#include "httpcode.h"
#include "environment.h"
#include "global.h"

#include <sstream>
#include <cstring>
#include <sys/types.h> // chmod
#include <sys/stat.h>

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
    , m_rawdata ( 0 )
    , m_lng_rawdata( 0 )
    , m_analyzing_board_xml( false )
    , m_board_null( 0 )
    , m_get_board( NULL )
    , m_enable_save_movetable( true )
{
    m_xml_document.clear();
    clear();
    clear_load_data();
    load_movetable();
    load_cache();
    load_etc();

    // JDのサポートBBS登録
    set_board( ENVIRONMENT::get_jdbbs(), "JDサポートBBS" );

    // 2chのスレの過去ログ
    set_board( ENVIRONMENT::get_jd2chlog(), "2chスレ過去ログ" );

    // ローカルファイル
    set_board( URL_BOARD_LOCAL, "ローカルファイル" );

    m_board_null = new DBTREE::BoardBase( "", "", "" );
}



//
// デストラクタで子Boardクラスをすべて削除
//
Root::~Root()
{
#ifdef _DEBUG    
    std::cout << "Root::~Root\n";
#endif

    clear();

    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){
        ( *it )->terminate_load();
        delete ( *it );
    }

    if( m_board_null ) delete m_board_null;

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
    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;
    m_lng_rawdata = 0;
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
    m_get_board = NULL;

    if( count == 0 ){

        size_t pos = url.rfind( "http://" );

        // ユーザープロフィールアドレス( http://be.2ch.net/test/p.php?u=d:http://〜 )の様に
        // 先頭以外に http:// が入っている場合は失敗
        if( pos != std::string::npos && pos != 0 ) return m_board_null;

        size_t pos2 = url.rfind( "https://" );
        if ( pos2 != std::string::npos && pos2 != 0 ) return m_board_null;
        if ( pos2 == 0 ) pos = 0;

        // http[s]:// が含まれていなかったら先頭に追加して再帰呼び出し
        if( pos == std::string::npos && ! is_local( url ) ){
            BoardBase* board = get_board( "http://" + url , count + 1 );
            m_get_board_url = url;
            return board;
        }
    }

    // サーチ
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        BoardBase* board = *( it );
        if( board->equal( url ) ){

            board->read_info(); // 板情報の取得( 詳しくはBoardBase::read_info()をみること )
            m_get_board = board;

#ifdef _SHOW_GETBOARD
            std::cout << "found\n";
#endif
            return board;
        }
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

            // 全ての板をサーチして移転先の板を探す
            std::list< BoardBase* >::iterator it;
            for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

                BoardBase* board = *( it );

                // 板パスを見て一致したら移転したと見なす
                // TODO : 板パスが同じ板が2つ以上あるときどうするか？
                if( is_2ch( board->get_root() ) && url.find( board->get_path_board() + "/" ) != std::string::npos ){

                    // 板移転テーブルを更新
                    push_movetable( MISC::get_hostname( url ), board->get_path_board(), board->get_root(), board->get_path_board() );
                    
                    std::ostringstream ss;
                    ss << board->get_name() << std::endl
                       << "旧 URL = " << MISC::get_hostname( url ) + board->get_path_board() << "/" << std::endl
                       << "新 URL  = " << board->url_boardbase() << std::endl;
                    MISC::MSG( ss.str() );

                    if( m_enable_save_movetable ){

                        //移転テーブル保存
                        save_movetable();

                        // サイドバーに登録されているURL更新
                        CORE::core_set_command( "update_sidebar_item" );
                    }

                    BoardBase* board = get_board( url, count + 1 );
                    m_get_board_url = url;
                    return board;
                }
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
    std::cout << "Root::get_board: not found url = " << url << std::endl;;
#endif
    
    // それでも見つからなかったらNULLクラスを返す
    return m_board_null;
}


// ローカルキャッシュから板一覧XML読み込み
//
// (注) 板一覧 XML の保存は BBSLIST::BBSListViewMain が行う
//
void Root::load_cache()
{
    clear();

    // ファイルが存在しなければ入力を旧ファイル名にする
    std::string file_in = CACHE::path_xml_listmain();
    if( CACHE::file_exists( file_in ) != CACHE::EXIST_FILE )
    {
        file_in = CACHE::path_xml_listmain_old();
    }

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
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = CONFIG::get_url_bbsmenu();
    data.modified = get_date_modified();

    start_load( data );
}


//
// bbsmenu 受信中
//
// virtual
void Root::receive_data( const char* data, size_t size )
{
    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
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
        SKELETON::MsgDiag mdiag( NULL, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ){
            set_date_modified( std::string() );
            download_bbsmenu();
        }

        return;
    }

    if( get_code() != HTTP_OK ){

        std::string msg = get_str_code() + "\n\n板一覧の読み込みに失敗したため板一覧は更新されませんでした。\n\nプロキシ設定や板一覧を取得するサーバのアドレスを確認して、ファイルメニューから板一覧の再読み込みをして下さい。\n板一覧取得サーバのアドレスはabout:configで確認出来ます。";
        SKELETON::MsgDiag mdiag( NULL, msg, false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        MISC::ERRMSG( "bbsmenu load failed : " + get_str_code() );

        CORE::core_set_command( "update_bbslist" );

        return;
    }

    // 文字コードを変換してXML作成
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( "MS932", "UTF-8" );
    int byte_out;
    const char* rawdata_utf8 = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    bbsmenu2xml( rawdata_utf8 );

    if( m_xml_document.hasChildNodes() )
    {
        // データベース更新
        analyze_board_xml();

        // bbslistview更新
        CORE::core_set_command( "update_bbslist" );
    }

    if( libiconv ) delete libiconv;
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
    XML::Document html( menu, true );

    // XML用のノードツリーにルートノードを追加
    m_xml_document.clear();
    XML::Dom* root = m_xml_document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

    // カテゴリの要素 <subdir></subdir>
    XML::Dom* subdir = 0;

    // カテゴリの有効/無効
    bool enabled = true;

    // 現在の仕様では HTML > BODY > font[size="2"] の子要素が対象
    XML::DomList targets = html.getElementsByTagName( "font" )[0]->childNodes();
    std::list< XML::Dom* >::iterator it = targets.begin();
    while( it != targets.end() )
    {
        // 要素b( カテゴリ名 )
        if( (*it)->nodeName() == "b" )
        {
            const std::string category = (*it)->firstChild()->nodeValue();

            // 追加しないカテゴリ
            if( category == "チャット"
             || category == "ツール類"
             || category == "他のサイト" )
            {
                enabled = false;
                ++it;
                continue;
            }
            else enabled = true;

            // <subdir>
            subdir = root->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
            subdir->setAttribute( "name", category );
        }
        // 要素bに続く要素a( 板URL )
        else if( subdir && enabled && (*it)->nodeName() == "a" )
        {
            const std::string board_name = (*it)->firstChild()->nodeValue();
            const std::string url = (*it)->getAttribute( "href" );

            // 板として扱うURLかどうかで要素名を変える
            std::string element_name;
            if( CONFIG::use_link_as_board() ) element_name = "board";
            else if( ( regex.exec( "^https?://.*/.*/$", url, offset, icase, newline, usemigemo, wchar )
			            && ( is_2ch( url ) || is_machi( url ) ) )
                     || is_JBBS( url )
                     || is_vip2ch( url )
                ) element_name = "board";
            else element_name = "link";

            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, element_name );
            board->setAttribute( "name", board_name );
            board->setAttribute( "url", url );
        }

        ++it;
    }

    root->setAttribute( "date_modified", get_date_modified() );
#ifdef _DEBUG
    std::cout << "modified = " << get_date_modified() << std::endl;
#endif
}



//
// XML に含まれる板情報を取り出してデータベースを更新
//
void Root::analyze_board_xml()
{
    m_move_info = std::string();
    m_analyzing_board_xml = true;
    m_analyzed_path_board.clear();

    XML::DomList boards = m_xml_document.getElementsByTagName( "board" );

    std::list< XML::Dom* >::iterator it = boards.begin();
    while( it != boards.end() )
    {
        const std::string name = (*it)->getAttribute( "name" );
        const std::string url = (*it)->getAttribute( "url" );

        //板情報セット
        set_board( url, name );

        ++it;
    }

    // 移転があった
    if( ! m_move_info.empty() )
    {
        SKELETON::EditViewDialog diag( m_move_info, "移転板一覧", false );
        diag.resize( 600, 400 );
        diag.run();

        if( m_enable_save_movetable ){

            //移転テーブル保存
            save_movetable();

            // サイドバーに登録されているURL更新
            CORE::core_set_command( "update_sidebar_item" );
        }
    }

    XML::Dom* root = m_xml_document.get_root_element( std::string( ROOT_NODE_NAME ) );
    if( root ) set_date_modified( root->getAttribute( "date_modified" ) );

    m_analyzing_board_xml = false;
    m_analyzed_path_board.clear();

#ifdef _DEBUG
    std::cout << "Root::analyze_board_xml\n";
    std::cout << "date_modified = " << get_date_modified() << std::endl;
#endif
}


//
// 板のタイプを判定
//
int Root::get_board_type( const std::string& url, std::string& root, std::string& path_board, const bool etc )
{
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    int type = TYPE_BOARD_UNKNOWN;

    // 2ch
    if( ! etc && is_2ch( url ) ){

        if( regex.exec( "(https?://[^/]*)/([^/]*)/$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH;
        }
    }

    // JBBS
    else if( is_JBBS( url ) ){

        if( regex.exec( "(https?://[^/]*)/(.*)/(index2?\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = "http://jbbs.shitaraba.net";
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_JBBS;
        }
    }

    // まち
    else if( is_machi( url ) ){

        if( regex.exec( "(https?://[^/]*)/([^/]*)/(index2?\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_MACHI;
        }
    }

    // vipサービス
    else if( is_vip2ch( url ) ){

        if( regex.exec( "(https?://[^/]*)/([^/]*)/$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH_COMPATI;
        }
    }

    // ローカルファイル
    else if( is_local( url ) ){

        root = URL_BOARD_LOCAL;
        path_board = "/local";

        type = TYPE_BOARD_LOCAL;
    }

    // その他は互換型
    else{

        if( regex.exec( "(https?://.*)/([^/]*)/([^\\.]+\\.html?)?$" , url, offset, icase, newline, usemigemo, wchar ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH_COMPATI;
        }
    }

    return type;
}


//
// 板のタイプを簡易判定
//
// 通常の get_board_type() で得た root を再判定するときに使用する ( 板移転処理用 )
//
int Root::get_board_type( const std::string& root, const bool etc ){

    int type = TYPE_BOARD_UNKNOWN;

    // 2ch
    if( ! etc && is_2ch( root ) )
        type = TYPE_BOARD_2CH;

    // JBBS
    else if( is_JBBS( root ) )
        type = TYPE_BOARD_JBBS;

    // まち
    else if( is_machi( root ) )
        type = TYPE_BOARD_MACHI;

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
// etc == true なら etc.txtに登録された外部板を意味する
//
bool Root::set_board( const std::string& url, const std::string& name, const std::string& basicauth, bool etc )
{
#ifdef _SHOW_BOARD
    std::cout << "Root::set_board " << url << " " << name << std::endl;
#endif

    std::string root;
    std::string path_board;

    // タイプ判定
    int type = get_board_type( url, root, path_board, etc );
    if( type == TYPE_BOARD_UNKNOWN ) return false;

    // 移転チェック
    BoardBase* board = NULL;
    const int stat = is_moved( root, path_board, name, &board, etc );

#ifdef _SHOW_BOARD
    std::cout << "root = " << root << " path_board = " << path_board
              << " basicauth = " << basicauth <<" type = " << type << " stat = " << stat << std::endl;
#endif

    // 新板登録
    if( stat == BOARD_NEW ){

        board = DBTREE::BoardFactory( type, root, path_board, name, basicauth );
        if( board ){
            m_list_board.push_back( board );
            if( m_analyzing_board_xml ) m_analyzed_path_board.insert( path_board );
        }
    }

    // 移転処理
    else if( stat == BOARD_MOVED ){

        // XML解析中に、
        // <board name="ニュース実況+" url="http://anchorage.2ch.net/liveplus/" />
        // <board name="ニュース実況+" url="http://gimpo.2ch.net/liveplus/" />
        // のように同じ板で異なるアドレスが現れた場合は、移転処理をせずにキャッシュが存在する方のアドレスを残す
        if( m_analyzing_board_xml && m_analyzed_path_board.find( path_board ) != m_analyzed_path_board.end() ){

            std::string tmp_msg = "Root::set_board : The XML file is broken !\n";
            tmp_msg += url + " has already been registered as " + board->url_boardbase();
            MISC::ERRMSG( tmp_msg );

            const std::string path1 = CACHE::path_board_root_fast( board->url_boardbase() );
            const std::string path2 = CACHE::path_board_root_fast( url );

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
bool Root::move_board( const std::string& url_old, const std::string& url_new, const bool etc )
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
    int type = get_board_type( url_new, root, path_board, etc );
    if( type == TYPE_BOARD_UNKNOWN ) return false;

    if( ! exec_move_board( board,
                           board->get_root(),
                           board->get_path_board(),
                           root,
                           path_board
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
bool Root::exec_move_board( BoardBase* board,
                       const std::string old_root,
                       const std::string old_path_board,
                       const std::string new_root,
                       const std::string new_path_board
    ){

    if( ! board ) return false;

#ifdef _SHOW_BOARD
    std::cout << "Root::exec_move_board\n";
    std::cout << old_root << old_path_board << " -> " << new_root <<  new_path_board << std::endl;
#endif

    if( old_root == new_root && old_path_board == new_path_board ){

        std::string errmsg = "移転元のアドレスと移転先のアドレスが同じです\n\n"
        + old_root + old_path_board + " → " + new_root +  new_path_board;

        SKELETON::MsgDiag mdiag( NULL, errmsg, false, Gtk::MESSAGE_ERROR );
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
    m_get_board = NULL;

    // もしキャッシュが存在したら移動して移転テーブル更新
    if( CACHE::file_exists( old_path ) == CACHE::EXIST_DIR ){

        // キャッシュがある場合はダイアログに表示
        m_move_info += ss.str() + "\n";

        // 移動先に同名のファイルかフォルダ何かあったらリネームしてバックアップをとっておく
        if( CACHE::file_exists( new_path ) != CACHE::EXIST_ERROR ){

            std::string path_tmp = new_path.substr( 0, new_path.length() - 1 ) + "_bk/";
            if( rename( new_path.c_str(), path_tmp.c_str() ) == 0 ) MISC::MSG( "rename : " +  new_path + " -> " + path_tmp );
            else MISC::ERRMSG( "can't rename " + new_path + " to " + path_tmp );
        }

        // キャッシュ移動
        if( CACHE::mkdir_parent_of_board( new_url ) ){

            if( rename( old_path.c_str(), new_path.c_str() ) == 0 ) MISC::MSG( "cache was moved : " +  old_path + " -> " + new_path );
            else MISC::ERRMSG( "can't move cache from " + old_path + " to " + new_path );
        }

#ifdef _DEBUG
        std::cout << "movetable was updated.\n"
                  << "old_root = " << old_root << std::endl
                  << "new_root = " << new_root << std::endl
                  << "old_path_board = " << old_path_board << std::endl
                  << "new_path_board = " << new_path_board << std::endl;
#endif
        push_movetable( old_root, old_path_board, new_root, new_path_board );

        // この板に関連する表示中のviewのURLを更新
        CORE::core_set_command( "update_url", old_url, new_url );
    }

    return true;
}


//
// 板移転テーブルを更新
//
void Root::push_movetable( const std::string old_root,
                           const std::string old_path_board,
                           const std::string new_root,
                           const std::string new_path_board
    )
{
#ifdef _DEBUG
    std::cout << "Root::push_movetable : " << old_root << old_path_board << " -> " << new_root << new_path_board << std::endl;
#endif            

    if( old_root == new_root && old_path_board == new_path_board ){

        std::string errmsg = "移転元のアドレスと移転先のアドレスが同じです (Root::push_movetable)\n\n"
        + old_root + old_path_board + " → " + new_root +  new_path_board;

        SKELETON::MsgDiag mdiag( NULL, errmsg, false, Gtk::MESSAGE_ERROR );
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
                  << ( *it_move ).old_root << ( *it_move ).old_path_board << "/ -> " << ( *it_move ).new_root << std::endl;
#endif            

        if(
            ( ( *it_move ).old_root == new_root
              && ( *it_move ).old_path_board == new_path_board )

            ||

            ( ( *it_move ).old_root == old_root
              && ( *it_move ).old_path_board == old_path_board )

            ){

            const std::string str_tmp = "削除: " +( *it_move ).old_root + ( *it_move ).old_path_board + "/ -> " + ( *it_move ).new_root + "\n";
#ifdef _DEBUG
            std::cout << str_tmp << std::endl;
#endif
            str += str_tmp;

            m_movetable.erase( it_move );
            it_move = m_movetable.begin();
            continue;
        }

        // 移転先を最新にする
        //
        // (注意) old_root == new_root かつ old_path_board == new_path_board のとき
        // erase した内容と push_back した内容が同じになるので無限ループに落ちる
        else if(

            ( *it_move ).new_root == old_root
            && ( *it_move ).new_path_board == old_path_board ){

            MOVETABLE movetable = *it_move;
            movetable.new_root = new_root;
            movetable.new_path_board = new_path_board;

            const std::string str_tmp = "更新: " +( *it_move ).old_root + ( *it_move ).old_path_board + "/ -> " + ( *it_move ).new_root
            + " => " + movetable.old_root + movetable.old_path_board + "/ -> " + movetable.new_root + "\n";
#ifdef _DEBUG
            std::cout << str_tmp << std::endl;
#endif
            str += str_tmp;

            m_movetable.erase( it_move );
            m_movetable.push_back( movetable );
            it_move = m_movetable.begin();
            continue;
        }

        ++it_move;
    }

    if( ! str.empty() ) MISC::MSG( "\n" + str );

    MOVETABLE movetable;
    movetable.old_root = old_root;
    movetable.old_path_board = old_path_board;
    movetable.new_root = new_root;
    movetable.new_path_board = new_path_board;
    m_movetable.push_back( movetable );
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

    m_list_board.remove( board );
    delete board;

    m_get_board_url = std::string();
    m_get_board = NULL;

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
                     BoardBase** board_old, bool etc )
{
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        BoardBase* board = *( it );

        if( board->get_path_board() == path_board ){

            // 既にリストに登録されてる
            if( board->get_root() == root ) return BOARD_EXISTS;

            // 名前が同じで、サイトが同じなら移転
            if( board->get_name() == name
                    && get_board_type( board->get_root() ) == get_board_type( root, etc ) ){
                *board_old = board;
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
            if( regex.exec( "https?://([^/]+:[^/]+@)(.+)$" , info.url, offset, icase, newline, usemigemo, wchar ) )
            {
                info.basicauth = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
                info.url = ( info.url[4] == 's' ? "https://" : "http://" ) + regex.str( 2 );
            }

            // board id
            info.boardid = *( it );
            if( it == list_etc.end() ) break;

#ifdef _DEBUG
            std::cout << "etc board : name = " << info.name << std::endl
                      << "url = " << info.url << std::endl
                      << "id:passwd = " <<info. basicauth << std::endl
                      << "boardid = " << info.boardid << std::endl << std::endl;
#endif

            // DBに登録
            if( set_board( info.url, info.name, info.basicauth, true ) )
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
bool Root::add_etc( const std::string& url, const std::string& name, const std::string& basicauth, const std::string& boardid )
{
#ifdef _DEBUG
    std::cout << "Root::add_etc url = " << url << " name = " << name
              << " auth = " << basicauth << std::endl;
#endif 

    DBTREE::ETCBOARDINFO info;
    info.url = url;
    info.name = name;
    info.basicauth = basicauth;
    info.boardid = boardid;

    if( set_board( info.url, info.name, info.basicauth, true ) )
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

    std::list< DBTREE::ETCBOARDINFO >::iterator it = m_etcboards.begin();
    for( ; it != m_etcboards.end(); ++it ){
        if( (*it).url == url_old && (*it).name == name_old ) break;
    }
    if( it == m_etcboards.end() ) return false;

#ifdef _DEBUG
    std::cout << "Root::move_etc " << url_old << " -> " << url_new << std::endl
              << name_old << " -> " << name_new << std::endl
              << board->get_basicauth() << " -> " << basicauth << std::endl;
#endif 

    // 移転処理
    if( url_old != url_new ){
        (*it).url = url_new;
        move_board( url_old, url_new, true );
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

    std::list< DBTREE::ETCBOARDINFO >::iterator it = m_etcboards.begin();
    for( ; it != m_etcboards.end(); ++it ){

        if( (*it).url == url && (*it).name == name ){
#ifdef _DEBUG
            std::cout << "found\n";
#endif 

            remove_board( url );
            m_etcboards.erase( it );
            return true;
        }
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

    std::list< DBTREE::ETCBOARDINFO >::iterator it = m_etcboards.begin();
    for( ; it != m_etcboards.end(); ++it ){

        etcboard += (*it).name + "\n";
        if( (*it).basicauth.empty() ) etcboard += (*it).url + "\n";
        else{

            size_t i = (*it).url.find( "://" );
            if( i != std::string::npos ){
                etcboard += (*it).url.substr( 0, i+3 ) + (*it).basicauth + "@" + (*it).url.substr( i+3 ) + "\n";
            }
            else etcboard += (*it).url + "\n";

        }
        etcboard += (*it).boardid + "\n"; 
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
    std::string movetable;
    if( CACHE::load_rawdata( file_move, movetable ) ){

        std::list< std::string > list_table = MISC::get_lines( movetable );
        std::list< std::string >::iterator it;
        for( it = list_table.begin(); it != list_table.end(); ++it ){

            std::list< std::string > lines = MISC::split_line( *it );

            if( lines.size() == 3 // 旧形式
                || lines.size() == 4 ){

                std::list< std::string >::iterator it2 = lines.begin();

                MOVETABLE movetable;
                movetable.old_root = *(it2++);
                movetable.new_root = *(it2++);
                movetable.old_path_board = *(it2++);
                if( lines.size() == 4 ) movetable.new_path_board = *(it2++);
                else movetable.new_path_board = movetable.old_path_board;
                m_movetable.push_back( movetable );
            }
        }
    }

#ifdef _DEBUG
    std::cout << "MOVETABLE : \n";
    std::list< MOVETABLE >::iterator it_move;
    for( it_move = m_movetable.begin(); it_move != m_movetable.end(); ++it_move )
        std::cout << ( *it_move ).old_root << ( *it_move ).old_path_board
                  << " -> " << ( *it_move ).new_root <<  ( *it_move ).new_path_board << std::endl;
#endif


}



//
// 板が移転したかチェックする
//
// 移転した時は移転後のURLを返す
//
const std::string Root::is_board_moved( const std::string& url ) // 簡易版
{
    std::string old_root;
    std::string old_path_board;
    std::string new_root;
    std::string new_path_board;
    return is_board_moved( url, old_root, old_path_board, new_root, new_path_board );
}


const std::string Root::is_board_moved( const std::string& url,
                                        std::string& old_root,
                                        std::string& old_path_board,
                                        std::string& new_root,
                                        std::string& new_path_board,
                                        const int count
    )
{
    const int max_count = 50;

#ifdef _DEBUG            
    std::cout << "Root::is_board_moved count = " << count << " url = " << url << std::endl;
#endif

    // 移転テーブルが循環している場合 2ch 型ならテーブルを修復する
    if( count > max_count ){

        if( is_2ch( url ) ){

            std::list< BoardBase* >::iterator it;
            for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

                // 板の最新のrootとpathを取得する
                BoardBase* board = *( it );

                // 板パスを見て一致したら移転したと見なす
                // TODO : 板パスが同じ板が2つ以上あるときどうするか？
                if( is_2ch( board->get_root() ) && url.find( board->get_path_board() + "/" ) != std::string::npos ){

                    std::string str = "移転テーブルが破損していたので修復しました\n";

                    std::list< MOVETABLE >::iterator it_move = m_movetable.begin();
                    for( ; it_move != m_movetable.end(); ){

                        if( is_2ch( ( *it_move ).old_root ) &&
                            url.find( ( *it_move ).old_path_board + "/" ) != std::string::npos
                            ){

                            // 最新のrootとpathに変更する
                            ( *it_move ).new_root = board->get_root();
                            ( *it_move ).new_path_board = board->get_path_board();

                            // url -> url の形となった場合は消す
                            if( ( *it_move ).old_root == ( *it_move ).new_root &&
                                ( *it_move ).old_path_board == ( *it_move ).new_path_board ){

                                str += "削除: " +( *it_move ).old_root + ( *it_move ).old_path_board + "/ -> " + board->url_boardbase() + "\n";

                                m_movetable.erase( it_move );
                                it_move = m_movetable.begin();

                                continue;
                            }

                            str += "更新: " + ( *it_move ).old_root + ( *it_move ).old_path_board + "/ -> " + board->url_boardbase() + "\n";
                        }

                        ++it_move;
                    }

                    MISC::MSG( str );

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
        }

        return std::string();
    }

    // 移転テーブルを検索
    std::list< MOVETABLE >::iterator it_move = m_movetable.begin();
    for( ; it_move != m_movetable.end(); ++it_move ){

        if( url.find( ( *it_move ).old_root ) == 0
            && url.find( ( *it_move ).old_path_board + "/" ) != std::string::npos ){

            const std::string new_url = ( *it_move ).new_root + ( *it_move ).new_path_board + "/";

            old_root = ( *it_move ).old_root;
            old_path_board = ( *it_move ).old_path_board;
            new_root = ( *it_move ).new_root;
            new_path_board = ( *it_move ).new_path_board;

            const std::string old_root_bkup = old_root;
            const std::string old_path_board_bkup = old_path_board;

#ifdef _DEBUG            
            std::cout << "count = " << count << " : " << url << " is moved to " << new_url << std::endl;
#endif

            // 連鎖的に検索
            std::string ret_url = is_board_moved( new_url, old_root, old_path_board, new_root, new_path_board, count +1 ); 

            // old_root と old_path_board が書き換わっているので戻しておく
            //
            // (注意) もし再帰呼び出ししたis_board_moved() の中で m_movetable.erase を実行すると
            // it_move は無効になるので、以前の様に
            // old_root = ( *it_move ).old_root;
            // old_path_board = ( *it_move ).old_path_board;
            // と it_move を使ってold_rootなどに代入するとメモリを破壊してセグフォになる時がある
            // (イテレータのメモリが残っていればセグフォにならないので完全に運まかせ)
            // 次にJDを起動したときは再帰呼び出ししたis_board_moved() の中の save_movetable()で
            // 移転テーブルは更新済みで落ちないのでタチが悪い
            old_root = old_root_bkup;
            old_path_board = old_path_board_bkup;

            return ret_url;
        }
    }

    if( count ) return url; // 再起呼び出しの場合

    return std::string();
}



// 全板の情報ファイル読み込み
void Root::read_boardinfo_all()
{
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        ( *it )->read_info();
    }
}



// 全スレ情報の保存
void Root::save_articleinfo_all()
{
#ifdef _DEBUG
    std::cout << "Root::save_articleinfo_all\n";
#endif

    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ){
        ( *it )->save_articleinfo_all();
    }

#ifdef _DEBUG
    std::cout << "end\n";
#endif
}



// 全ログ検索
void Root::search_cache( std::vector< ArticleBase* >& list_article,
                         const std::string& query, const bool mode_or, const bool bm, const bool& stop )
{
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){
        ( *it )->search_cache( list_article, query, mode_or, bm, stop );
        if( stop ) break;
    }
}


// 全てのスレの書き込み履歴削除
void Root::clear_all_post_history()
{
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){
        ( *it )->clear_all_post_history();
    }
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

    std::list< MOVETABLE >::iterator it_move;
    for( it_move = m_movetable.begin(); it_move != m_movetable.end(); ++it_move ){

        movetable <<( *it_move ).old_root << " "
                  << ( *it_move ).new_root << " "
                  << ( *it_move ).old_path_board;

        // 新形式
        if( ! ( *it_move ).new_path_board.empty()
            && ( *it_move ).old_path_board != ( *it_move ).new_path_board ) movetable << " " << ( *it_move ).new_path_board;

        movetable << std::endl;
    }

#ifdef _DEBUG
    std::cout << movetable.str();
#endif
    
    CACHE::save_rawdata( file_move, movetable.str() );
}



//
// 2ch型のURLかどうか
//
bool Root::is_2ch( const std::string& url )
{
    const std::string hostname = MISC::get_hostname( url );

    if( ( hostname.find( ".2ch.net" ) != std::string::npos && hostname.find( "info.2ch.net" ) == std::string::npos )
        || ( hostname.find( ".5ch.net" ) != std::string::npos && hostname.find( "info.5ch.net" ) == std::string::npos )
        || hostname.find( ".bbspink.com" ) != std::string::npos ) return true;

    return false;
}



//
// JBBS型のURLかどうか
//
bool Root::is_JBBS( const std::string& url )
{
    const std::string hostname = MISC::get_hostname( url );

    if( hostname.find( "jbbs.livedoor.jp" ) != std::string::npos
        || hostname.find( "jbbs.shitaraba.com" ) != std::string::npos
        || hostname.find( "jbbs.shitaraba.net" ) != std::string::npos ) return true;

    return false;
}


//
// まち型のURLかどうか
//
bool Root::is_machi( const std::string& url )
{
    const std::string hostname = MISC::get_hostname( url );

    if( hostname.find( ".machi.to" ) != std::string::npos ) return true;

    return false;
}


//
// vipサービスのURLか
//
bool Root::is_vip2ch( const std::string& url )
{
    const std::string hostname = MISC::get_hostname( url );

    if( hostname.find( ".vip2ch.com" ) != std::string::npos ) return true;

    return false;
}


//
// ローカルファイルか
//
bool Root::is_local( const std::string& url )
{
    if( url.find( "file://" ) != std::string::npos ) return true;

    return false;
}


//
// スレあぼーん情報を更新した時、全boardbaseクラスに対応するスレ一覧の表示を更新させる
//
// CONFIG::set_abone_number_thread() などでグローバル設定をした後などに呼び出す
//
void Root::update_abone_thread()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->update_abone_thread( true );
}



//
// 全boardbaseクラスに、それに属する全articlebaseクラスのあぼーん状態の更新をさせる
//
void Root::update_abone_all_article()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->update_abone_all_article();
}


//
// 全boardbaseクラスに、それに属する全articlebaseクラスの書き込み時間とスレ立て時間の文字列をリセットさせる
//
void Root::reset_all_since_date()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->reset_all_since_date();
}

void Root::reset_all_write_date()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->reset_all_write_date();
}

void Root::reset_all_access_date()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->reset_all_access_date();
}

