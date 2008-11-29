// ライセンス: GPL2

//#define _DEBUG
//#define _SHOW_GETBOARD
//#define _SHOW_BOARD
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
#include "jdversion.h"

#include <sstream>
#include <cstring>
#include <sys/types.h> // chmod
#include <sys/stat.h>

enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024  //  bbsmenu.html の最大サイズ
};

// ルート要素名( list_main.xml )
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
{
    m_document.clear();
    clear();
    clear_load_data();
    load_movetable();
    load_cache();
    load_etc();

    // JDのサポートBBS登録
    set_board( JDBBS, "JDサポートBBS" );

    // 2chのスレの過去ログ
    set_board( JD2CHLOG, "2chスレ過去ログ" );

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
    if( url == m_get_board_url && m_get_board ){
#ifdef _SHOW_GETBOARD
        std::cout << "hit cache\n";
#endif
        return m_get_board;
    }
    m_get_board_url = url;
    m_get_board = NULL;

    // 先頭が http:// でなかったら足して再帰呼び出し
    if( count < max_count && url.find( "http://" ) != 0 ){
        BoardBase* board = get_board( "http://" + url , count + 1 );
        m_get_board_url = url;
        return board;
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

                    //移転テーブル保存
                    save_movetable();

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

    if( CACHE::load_rawdata( file_in, m_xml_bbsmenu ) )
    {
        // Domノードを初期化
        m_document.init( m_xml_bbsmenu );

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
    m_document.clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );

    JDLIB::LOADERDATA data;
    data.url = CONFIG::get_url_bbsmenu();
    data.agent = CONFIG::get_agent_for_data(); 
    if( CONFIG::get_use_proxy_for_data() ) data.host_proxy = CONFIG::get_proxy_for_data();
    else data.host_proxy = std::string();
    data.port_proxy = CONFIG::get_proxy_port_for_data();
    data.basicauth_proxy = CONFIG::get_proxy_basicauth_for_data();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

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
    // 文字コードを変換してXML作成
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( "MS932", "UTF-8" );
    int byte_out;
    const char* rawdata_utf8 = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    bbsmenu2xml( rawdata_utf8 );

    if( m_document.hasChildNodes() )
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

    // menu のノードツリーを取得( menu がHTMLなので第二引数は true )
    XML::Document html( menu, true );

    // XML用のノードツリーにルートノードを追加
    m_document.clear();
    XML::Dom* boardlist = m_document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

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
            subdir = boardlist->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
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
            else if( ( regex.exec( "^http://.*/.*/$", url ) && is_2ch( url ) )
                     || is_machi( url )
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

#ifdef _DEBUG
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif
}



//
// XML に含まれる板情報を取り出してデータベースを更新
//
void Root::analyze_board_xml()
{
#ifdef _DEBUG
    std::cout << "Root::analyze_board_xml\n";
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif

    m_move_info = std::string();
    m_analyzing_board_xml = true;
    m_analyzed_path_board.clear();

    XML::DomList boards = m_document.getElementsByTagName( "board" );

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

        //移転テーブル保存
        save_movetable();
    }

    m_analyzing_board_xml = false;
    m_analyzed_path_board.clear();
}


//
// 板のタイプを判定
//
int Root::get_board_type( const std::string& url, std::string& root, std::string& path_board, const bool etc )
{
    JDLIB::Regex regex;
    int type = TYPE_BOARD_UNKNOWN;

    // 2ch
    if( ! etc && is_2ch( url ) ){

        if( regex.exec( "(http://[^/]*)/([^/]*)/$" , url ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH;
        }
    }

    // JBBS
    else if( is_JBBS( url ) ){

        if( regex.exec( "(http://[^/]*)/(.*)/(index2?\\.html?)?$" , url ) ){
            root = "http://jbbs.livedoor.jp";
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_JBBS;
        }
    }

    // まち
    else if( is_machi( url ) ){

        if( regex.exec( "(http://[^/]*)/([^/]*)/(index2?\\.html?)?$" , url ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_MACHI;
        }
    }

    // vipサービス
    else if( is_vip2ch( url ) ){

        if( regex.exec( "(http://[^/]*)/([^/]*)/$" , url ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH_COMPATI;
        }
    }

    // その他は互換型
    else{

        if( regex.exec( "(http://.*)/([^/]*)/([^\\.]+\\.html?)?$" , url ) ){
            root = regex.str( 1 );
            path_board = "/" + regex.str( 2 );

            type = TYPE_BOARD_2CH_COMPATI;
        }
    }

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
    const int stat = is_moved( root, path_board, name, &board );

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
            tmp_msg += url + " has already been registerd as " + board->url_boardbase();
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
const bool Root::move_board( const std::string& url_old, const std::string& url_new, const bool etc )
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
    if( ! m_move_info.empty() ) save_movetable();

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
                     BoardBase** board_old )
{
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        BoardBase* board = *( it );

        if( board->get_path_board() == path_board ){

            // 既にリストに登録されてる
            if( board->get_root() == root ) return BOARD_EXISTS;

            // 名前が同じなら移転
            if( board->get_name() == name ){
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
            if( regex.exec( "http://([^/]+:[^/]+@)(.+)$" , info.url ) )
            {
                info.basicauth = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
                info.url = "http://" + regex.str( 2 );
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

                    //移転テーブル保存
                    save_movetable();

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

// 全ログ検索
std::list< std::string > Root::search_cache( const std::string& query, bool mode_or, bool& stop )
{
    std::list< std::string > m_urllist;

    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        std::list< std::string > m_tmplist;
        m_tmplist = ( *it )->search_cache( query, mode_or, stop );
        m_urllist.splice( m_urllist.end(), m_tmplist );

        if( stop ) break;
    }

    return m_urllist;
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

    // 板一覧に登録されているURL更新
    CORE::core_set_command( "update_bbslist_item" );

    // お気に入りに登録されているURL更新
    CORE::core_set_command( "update_favorite_item" );

    // 履歴メニューのURL更新
    CORE::core_set_command( "update_history" );
}



//
// 2ch型のURLかどうか
//
bool Root::is_2ch( const std::string& url )
{
    std::string hostname = MISC::get_hostname( url );

    if( ( hostname.find( ".2ch.net" ) != std::string::npos && hostname.find( "info.2ch.net" ) == std::string::npos )
        || hostname.find( ".bbspink.com" ) != std::string::npos ) return true;

    return false;
}



//
// JBBS型のURLかどうか
//
bool Root::is_JBBS( const std::string& url )
{
    std::string hostname = MISC::get_hostname( url );

    if( hostname.find( "jbbs.livedoor.jp" ) != std::string::npos
        || hostname.find( "jbbs.shitaraba.com" ) != std::string::npos ) return true;

    return false;
}


//
// まち型のURLかどうか
//
bool Root::is_machi( const std::string& url )
{
    std::string hostname = MISC::get_hostname( url );

    if( hostname.find( ".machi.to" ) != std::string::npos ) return true;

    return false;
}


//
// vipサービスのURLか
//
bool Root::is_vip2ch( const std::string& url )
{
    std::string hostname = MISC::get_hostname( url );

    if( hostname.find( ".vip2ch.com" ) != std::string::npos ) return true;

    return false;
}


//
// 配下の全boardbaseクラスのレスあぼーん状態を更新する
//
void Root::update_abone_all_board()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->update_abone_thread();
}



//
// 配下の全boardbaseクラスに、全articlebaseクラスのあぼーん状態の更新をさせる
//
void Root::update_abone_all_article()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->update_abone_all_article();
}
