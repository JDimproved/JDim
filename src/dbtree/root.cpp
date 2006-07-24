// ライセンス: 最新のGPL

//#define _DEBUG
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

#include "global.h"
#include "command.h"
#include "config/globalconf.h"
#include "cache.h"
#include "jdversion.h"

#include <sstream>

#define SIZE_OF_RAWDATA ( 2 * 1024 * 1024 )  //  bbsmenu.html の最大サイズ

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
{
    m_xml_bbsmenu.clear();
    clear();
    clear_load_data();
    load_movetable();
    load_cache();
    load_etc();

    // JDのサポートBBS登録
    set_board( JDBBS, "JDサポートBBS" );

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
#ifdef _DEBUG
    std::cout << "Root::get_board : count = " << count << " url = " << url << std::endl;
#endif

    if( count > 20 ){
        MISC::ERRMSG( "Root::get_board: could not find board" );
        return m_board_null;
    }

    // 先頭が http:// でなかったら足して再帰呼び出し
    if( url.find( "http://" ) != 0 ) return get_board( "http://" + url , count + 1 );

    // サーチ
    std::list< BoardBase* >::iterator it;
    for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

        BoardBase* board = *( it );
        if( board->equal( url ) ){
            board->read_info(); // 板情報の取得( 詳しくはBoardBase::read_info()をみること )
            return board;
        }
    }

    // 移転した時はrootを付け変えて再帰呼び出し
    std::list< MOVETABLE >::iterator it_move;
    for( it_move = m_movetable.begin(); it_move != m_movetable.end(); ++it_move ){

        if( url.find( ( *it_move ).path_board ) != std::string::npos
            && url.find( ( *it_move ).old_root ) == 0  ){

            std::string new_url = ( *it_move ).new_root + ( *it_move ).path_board + "/";

#ifdef _DEBUG            
            std::cout << url << " is moved to " << new_url << std::endl;
#endif
            return get_board( new_url, count + 1 );
        }
    }

    // 2ch型の場合、板パスを見てもし一致したら新ホストに移転したと判断して移転テーブルを更新する
    if( is_2ch( url ) ){

        std::list< BoardBase* >::iterator it;
        for( it = m_list_board.begin(); it != m_list_board.end(); ++it ){

            BoardBase* board = *( it );

            if( url.find( board->get_path_board() + "/" ) != std::string::npos ){

                // 板移転テーブルを更新
                MOVETABLE movetable;
                movetable.old_root = MISC::get_hostname( url );
                movetable.new_root = board->get_root();
                movetable.path_board = board->get_path_board();
                m_movetable.push_back( movetable );

                std::ostringstream ss;
                ss << board->get_name() << " is moved" << std::endl
                   << "old url = " << movetable.old_root + board->get_path_board() << "/" << std::endl
                   << "new url = " << board->url_boardbase() << std::endl;
                MISC::MSG( ss.str() );

                //移転テーブル保存
                save_movetable();

                board->read_info();
                return board;
            }
        }

        // 最後が "/" で終わってなかったら足して再帰呼び出し
        if( url[ url.length() ] != '/' ) return get_board( url + "/" , count + 1 );
    }

    // それでも見つからなかったらNULLクラスを返す
    
#ifdef _DEBUG            
    std::cout << "not found\nreturn Board_Null\n";
#endif
    
    return m_board_null;
}


// ローカルキャッシュから板一覧XML読み込み
void Root::load_cache()
{
    clear();

    std::string file_in = CACHE::path_xml_listmain();
#ifdef _DEBUG
    std::cout << "Root::load_cache xml  = " << file_in << std::endl;
#endif    

    if( CACHE::load_rawdata( file_in, m_xml_bbsmenu ) ){

        // xml の内容からDBに板を登録
        update_boards( m_xml_bbsmenu );
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
    m_xml_bbsmenu.clear();
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
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( "MS932" );
    int byte_out;
    const char* rawdata_utf8 = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    bbsmenu2xml( rawdata_utf8 );

    if( !m_xml_bbsmenu.empty() ){

        // データベース更新
        update_boards( m_xml_bbsmenu );
        
        // XML保存
        std::string file_out = CACHE::path_xml_listmain();
        CACHE::save_rawdata( file_out, m_xml_bbsmenu.data(), m_xml_bbsmenu.length() );
#ifdef _DEBUG
        std::cout << "Root::receive_finish save to " << file_out << std::endl;
#endif        
        // 板リストview更新
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
    JDLIB::Regex regex;
    m_xml_bbsmenu.clear();

    std::list< std::string > lines =  MISC::get_lines( menu );

    // 行単位でパースしていく
    std::string str_category;
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){

        std::string& line = (*it);

        // カテゴリに入る
        size_t i;
        if( ( i = line.find( "<BR><BR><B>" ) ) != std::string::npos ){

            if( ! regex.exec( " ?<BR><BR><B>(.*)</B><BR>.*", line ) ) continue;

            if( ! str_category.empty() ) m_xml_bbsmenu += "</subdir open=\"0\" >\n";
            str_category = MISC::remove_space( regex.str( 1 ) );

            if( str_category == "チャット"
                || str_category == "ツール類"
                || str_category == "他のサイト" ) str_category = std::string();
            else{
                m_xml_bbsmenu += "<subdir name=\"";
                m_xml_bbsmenu += str_category;
                m_xml_bbsmenu += "\" >\n";
            }
        }

        // URLと板名取得
        if( ! str_category.empty() ){

            if( ! regex.exec( " ?<A HREF=(http://[^>]*/) ?(TARGET=_blank)?>(.*)</A>.*", line ) ) continue;

            bool link = false;
            std::string url = MISC::remove_space( regex.str( 1 ) );
            std::string name = MISC::remove_space( regex.str( 3 ) );

            // url, nameのチェック
            if( !regex.exec( "http://.*/.*/", url ) ) link = true;
            if( name.empty() ) continue;

            // XML に追加
            std::ostringstream st_xml;
            st_xml.clear();
            if( link ) st_xml << "<link url=\"";
            else st_xml << "<board url=\"";
            st_xml << url;
            st_xml << "\" name=\"";
            st_xml << name;
            st_xml <<  "\" />\n";

            m_xml_bbsmenu += st_xml.str();
        }
    }
    if( ! str_category.empty() ) m_xml_bbsmenu += "</subdir open=\"0\" >\n";
}


//
// XML に含まれる板情報を取り出してデータベースを更新
//
void Root::update_boards( const std::string xml )
{
#ifdef _DEBUG
    std::cout << "Root::update_boards\n";
#endif

    JDLIB::Regex regex;

    m_move_info = std::string();

    // XMLを行ごとにばらして<board>タグを見る
    std::list< std::string > lines =  MISC::get_lines( xml );
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){
        
        std::string& line = (*it);

        if( ! regex.exec( " ?<board url=\"(.*)\" ?name=\"(.*)\" ?/>.*", line ) ) continue;

        std::string url = MISC::remove_space( regex.str( 1 ) );
        std::string name = MISC::remove_space( regex.str( 2 ) );

        //板情報セット
        set_board( url, name );
    }

    // 移転があった
    if( ! m_move_info.empty() ){
        
        Gtk::MessageDialog mdiag( "移転一覧\n" + m_move_info  ); mdiag.run();

        //移転テーブル保存
        save_movetable();
    }
}


//
// 板のタイプに合わせて板情報をセット
// ついでに移転とかの判定もする
//
bool Root::set_board( const std::string& url, const std::string& name )
{
#ifdef _SHOW_BOARD
    std::cout << url << " " << name << std::endl;
#endif

    JDLIB::Regex regex;
    std::string root;
    std::string path_board;

    // 板のタイプを判定
    int type;

    // 2ch
    if( is_2ch( url ) ){

        if( ! regex.exec( "(http://[^/]*)/([^/]*)/$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_2CH;
    }

    // JBBS
    else if( is_JBBS( url ) ){

        if( ! regex.exec( "(http://[^/]*)/(.*)/$" , url ) ) return false;
        root = "http://jbbs.livedoor.jp";
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_JBBS;
    }

    // まち
    else if( is_machi( url ) ){

        if( ! regex.exec( "(http://[^/]*)/([^/]*)/$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_MACHI;
    }

    // その他は互換型
    else{

        if( ! regex.exec( "(http://.*)/([^/]*)/$" , url ) ) return false;
        root = regex.str( 1 );
        path_board = "/" + regex.str( 2 );

        type = TYPE_BOARD_2CH_COMPATI;
    }

    // 移転チェック
    BoardBase* board = NULL;
    int stat = is_moved( root, path_board, name, &board );

#ifdef _SHOW_BOARD
    std::cout << "root = " << root << " path_board = " << path_board <<" type = " << type << " stat = " << stat << std::endl;
#endif

    // 新板登録
    if( stat == BOARD_NEW ){
        board = DBTREE::BoardFactory( type, root, path_board, name );
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
        ss << board->get_name() << " is moved" << std::endl
           << " old url = " << old_url << std::endl
           << " new url = " << new_url << std::endl;
        MISC::MSG( ss.str() );

        // もしキャッシュが存在したら移動して移転テーブル更新
        if( CACHE::is_file_exists( old_path ) == CACHE::EXIST_DIR ){

            // キャッシュがある場合はダイアログに表示
            m_move_info += ss.str();

            // 移動先に同名のファイルかフォルダ何かあったらリネームしてバックアップをとっておく
            if( CACHE::is_file_exists( new_path ) != CACHE::EXIST_ERROR ){

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
// etc.txt(Navi2ch互換) を読み込んで外部板登録してXMLに変換
//
void Root::load_etc()
{
    m_xml_etc.clear();

    std::string file_etctxt = CACHE::path_etcboard();
    std::string etcboard;
    if( CACHE::load_rawdata( file_etctxt, etcboard ) ){

        std::list< std::string > list_etc = MISC::get_lines( etcboard );
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
                
#ifdef _DEBUG
            std::cout << "etc board : " << url << " " << name << std::endl;
#endif

            // DBに登録
            if( set_board( url, name ) ){

                m_xml_etc += "<board url=\"";
                m_xml_etc += url;
                m_xml_etc += "\" name=\"";
                m_xml_etc += name;
                m_xml_etc +=  "\" />\n";
            }

        }
    }
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
}



//
// 2ch型のURLかどうか
//
bool Root::is_2ch( const std::string& url )
{
    std::string hostname = MISC::get_hostname( url );

    if( hostname.find( ".2ch.net" ) != std::string::npos
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
// 配下の全boardbaseクラスに、全articlebaseクラスのあぼーん状態の更新をさせる
//
void Root::update_abone_all_article()
{
    std::list< BoardBase* >::iterator it = m_list_board.begin();
    for( ; it != m_list_board.end(); ++it ) ( *it )->update_abone_all_article();
}
