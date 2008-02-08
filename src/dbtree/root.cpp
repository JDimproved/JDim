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

#include "global.h"
#include "command.h"
#include "config/globalconf.h"
#include "cache.h"
#include "jdversion.h"

#include <sstream>
#include <cstring>

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
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ) delete *( it );

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
BoardBase* Root::get_board( const std::string& url, int count )
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
    if( count < max_count && url.find( "http://" ) != 0 ) return get_board( "http://" + url , count + 1 );

    // サーチ
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        BoardBase* board = *( it );
        if( board->equal( url ) ){
            board->read_info(); // 板情報の取得( 詳しくはBoardBase::read_info()をみること )
            m_get_board = board;
            return board;
        }
    }


    // 移転した時はrootを付け変えて再帰呼び出し
    if( count < max_count ){
        std::string new_url = is_board_moved( url );
        if( ! new_url.empty() ) return get_board( new_url, count + 1 );
    }

    // 2ch型の場合、板パスを見てもし一致したら新ホストに移転したと判断して移転テーブルを更新する
    if( is_2ch( url ) ){

        std::list< BoardBase* >::iterator it;
        for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

            BoardBase* board = *( it );

            if( is_2ch( board->get_root() ) && url.find( board->get_path_board() + "/" ) != std::string::npos ){

                // 板移転テーブルを更新

                if( count < max_count ){ 

                    MOVETABLE movetable;
                    movetable.old_root = MISC::get_hostname( url );
                    movetable.new_root = board->get_root();
                    movetable.path_board = board->get_path_board();
                    m_movetable.push_back( movetable );

                    std::ostringstream ss;
                    ss << board->get_name() << std::endl
                       << "旧 URL = " << movetable.old_root + board->get_path_board() << "/" << std::endl
                       << "新 URL  = " << board->url_boardbase() << std::endl;
                    MISC::MSG( ss.str() );

                }
                else{ // 移転テーブルが循環している場合

                    std::string str = "移転テーブルが破損していたので修復しました\n";

                    std::list< MOVETABLE >::iterator it_move = m_movetable.begin();
                    for( ; it_move != m_movetable.end(); ++it_move ){

                        if( is_2ch( ( *it_move ).old_root ) && url.find( ( *it_move ).path_board + "/" ) != std::string::npos ){

                            ( *it_move ).new_root = board->get_root();
                            str += ( *it_move ).old_root + board->get_path_board() + "/ -> " + board->url_boardbase() + "\n";
                        }
                    }

                    MISC::MSG( str );
                }

                //移転テーブル保存
                save_movetable();

                board->read_info();
                m_get_board = board;
                return board;
            }
        }

        // 最後が "/" で終わってなかったら足して再帰呼び出し
        if( count < max_count && url[ url.length() -1 ] != '/' ) return get_board( url + "/" , count + 1 );
    }

    // それでも見つからなかったらNULLクラスを返す
    
#ifdef _DEBUG            
    std::cout << "Root::get_board: not found\nreturn Board_Null\n";
#endif
    
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
        update_boards();
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
    data.agent = CONFIG::get_agent_for2ch(); 
    if( CONFIG::get_use_proxy_for2ch() ) data.host_proxy = CONFIG::get_proxy_for2ch();
    else data.host_proxy = std::string();
    data.port_proxy = CONFIG::get_proxy_port_for2ch();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

    start_load( data );
}


//
// bbsmenu 受信中
//
void Root::receive_data( const char* data, size_t size )
{
    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
}


//
// bbsmenu 受信完了
//
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
        update_boards();

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
                     || is_JBBS( url ) ) element_name = "board";
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
void Root::update_boards()
{
#ifdef _DEBUG
    std::cout << "Root::update_boards\n";
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif

    m_move_info = std::string();

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
}


//
// 直接データベースに板を追加/アップデート
//
void Root::update_board( const std::string& url, const std::string& name, const std::string& basicauth, bool etc )
{
#ifdef _DEBUG
    std::cout << "Root::update_board " << url << " " << name << std::endl;
#endif

    m_move_info = std::string();
    set_board( url, name, basicauth, etc );

    // 移転があった
    if( ! m_move_info.empty() ) save_movetable();
}


//
// 板のタイプに合わせて板情報をセット
// ついでに移転の判定もする
//
// etc == true なら etc.txtに登録された外部板を意味する
//
bool Root::set_board( const std::string& url, const std::string& name, const std::string& basicauth, bool etc )
{
#ifdef _SHOW_BOARD
    std::cout << "Root::set_board " << url << " " << name << std::endl;
#endif

    JDLIB::Regex regex;
    std::string root;
    std::string path_board;

    // 板のタイプを判定
    int type;

    // 2ch
    if( !etc && is_2ch( url ) ){

        if( ! regex.exec( "(http://[^/]*)/([^/]*)/$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_2CH;
    }

    // JBBS
    else if( is_JBBS( url ) ){

        if( ! regex.exec( "(http://[^/]*)/(.*)/(index2?\\.html?)?$" , url ) ) return false;
        root = "http://jbbs.livedoor.jp";
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_JBBS;
    }

    // まち
    else if( is_machi( url ) ){

        if( ! regex.exec( "(http://[^/]*)/([^/]*)/(index2?\\.html?)?$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_MACHI;
    }

    // その他は互換型
    else{

        if( ! regex.exec( "(http://.*)/([^/]*)/([^\\.]+\\.html?)?$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_2CH_COMPATI;
    }

    // 移転チェック
    BoardBase* board = NULL;
    int stat = is_moved( root, path_board, name, &board );

#ifdef _SHOW_BOARD
    std::cout << "root = " << root << " path_board = " << path_board
              << " basicauth = " << basicauth <<" type = " << type << " stat = " << stat << std::endl;
#endif

    // 新板登録
    if( stat == BOARD_NEW ){
        board = DBTREE::BoardFactory( type, root, path_board, name, basicauth );
        if( board ) m_list_board.push_back( board );
    }

    // 移転処理
    else if( stat == BOARD_MOVED && board ){

        std::string old_root = board->get_root();
        std::string old_path_board = board->get_path_board();
        std::string old_url = board->url_boardbase();
        std::string old_path = CACHE::path_board_root( old_url );

        // DB更新
        board->update_root( root ); 

        std::string new_url = board->url_boardbase();
        std::string new_path = CACHE::path_board_root( new_url );

        std::ostringstream ss;
        ss << board->get_name() << std::endl
           << " 旧 URL = " << old_url << std::endl
           << " 新 URL = " << new_url << std::endl;
        MISC::MSG( ss.str() );

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

            // 板移転テーブルを更新
            MOVETABLE movetable;
            movetable.old_root = old_root;
            movetable.new_root = root;
            movetable.path_board = board->get_path_board();
            m_movetable.push_back( movetable );

            // coreに知らせてviewなども更新
            CORE::core_set_command( "update_host", old_root, root );

#ifdef _DEBUG
            std::cout << "movetable was updated.\n";
#endif
        }
    }

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
// 外部板読み込み
//
// etc.txt(Navi2ch互換) を読み込んで外部板登録してペア( 名前, URL )に変換
//
void Root::load_etc()
{
    m_xml_etc.clear();

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

            // 名前
            std::string name;
            name = *( it++ );
            if( it == list_etc.end() ) break;

            // url
            std::string url;
            url = *( it++ );
            if( it == list_etc.end() ) break;

            // basic認証
            std::string basicauth;
            if( regex.exec( "http://([^/]+:[^/]+@)(.+)$" , url ) )
            {
                basicauth = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
                url = "http://" + regex.str( 2 );
            }

#ifdef _DEBUG
            std::cout << "etc board : " << url << " " << name << std::endl;
            std::cout << "id:passwd = " << basicauth << std::endl;
#endif

            // DBに登録
            if( set_board( url, name, basicauth, true ) )
            {
                m_xml_etc.insert( make_pair( name, url ) );
            }
        }
    }

#ifdef _DEBUG
    std::cout << "外部板数：" << m_xml_etc.size() << std::endl;
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

            if( lines.size() == 3 ){

                std::list< std::string >::iterator it2 = lines.begin();

                MOVETABLE movetable;
                movetable.old_root = *(it2++);
                movetable.new_root = *(it2++);
                movetable.path_board = *(it2++);
                m_movetable.push_back( movetable );
            }
        }
    }

#ifdef _DEBUG
    std::cout << "MOVETABLE : \n";
    std::list< MOVETABLE >::iterator it_move;
    for( it_move = m_movetable.begin(); it_move != m_movetable.end(); ++it_move )
        std::cout << ( *it_move ).old_root << ( *it_move ).path_board
                  << " -> " << ( *it_move ).new_root <<  ( *it_move ).path_board << std::endl;
#endif


}



//
// 板が移転したかチェックする
//
// 移転した時は移転後のURLを返す
//
const std::string Root::is_board_moved( const std::string& url )
{
    std::list< MOVETABLE >::iterator it_move = m_movetable.begin();
    for( ; it_move != m_movetable.end(); ++it_move ){

        if( url.find( ( *it_move ).old_root ) == 0
            && url.find( ( *it_move ).path_board + "/" ) != std::string::npos ){

            std::string new_url = ( *it_move ).new_root + ( *it_move ).path_board + "/";

#ifdef _DEBUG            
            std::cout << url << " is moved to " << new_url << std::endl;
#endif
            return new_url;
        }
    }

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
                  << ( *it_move ).new_root << " " << ( *it_move ).path_board << std::endl;
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
