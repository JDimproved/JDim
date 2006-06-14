// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "cache.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#include "dbtree/interface.h"

#include <gtkmm.h>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <fcntl.h>


// 設定ファイル
std::string CACHE::path_conf()
{
    std::string home = getenv( "HOME" );

    return home + "/.jdrc";
}


// セッション情報ファイル
std::string CACHE::path_session()
{
    return CACHE::path_root() + "session.info";
}


// ロックファイル
std::string CACHE::path_lock()
{
    return CACHE::path_root() + "JDLOCK";
}


// パスワード設定ファイル
std::string CACHE::path_passwd( const std::string& basename )
{
    return CACHE::path_root() + basename + ".conf";
}


// キャッシュルートの絶対パス
std::string CACHE::path_root()
{

    std::string root = MISC::remove_space( CONFIG::get_path_cacheroot() );

    if( root[ root.length() -1 ] != '/' ) root = root + "/";

    if( root[ 0 ] == '~' ){
        std::string home = getenv( "HOME" );
        root.replace( 0, 1, home );
    }

    return root;
}


// 板リスト
std::string CACHE::path_xml_listmain()
{
    return CACHE::path_root() +  "list_main.xml";
}

std::string CACHE::path_xml_listmain_bkup()
{
    return CACHE::path_xml_listmain() + ".bkup";
}


// お気に入り
std::string CACHE::path_xml_favorite()
{
    return CACHE::path_root() +  "favorite.xml";
}

std::string CACHE::path_xml_favorite_bkup()
{
    return CACHE::path_xml_favorite() + ".bkup";
}


// 外部板設定ファイル( navi2ch 互換 )
std::string CACHE::path_etcboard()
{
    return CACHE::path_root() +  "etc.txt";
}


// 履歴
std::string CACHE::path_xml_history()
{
    return CACHE::path_root() +  "history.xml";
}


// 板履歴
std::string CACHE::path_xml_history_board()
{
    return CACHE::path_root() +  "history_board.xml";
}


// 板移転情報
std::string CACHE::path_movetable()
{
    return CACHE::path_root() + "move.info";
}


// キーボード設定
std::string CACHE::path_keyconf()
{
    return CACHE::path_root() + "key.conf";
}


// マウスジェスチャ設定
std::string CACHE::path_mouseconf()
{
    return CACHE::path_root() + "mouse.conf";
}


// マウスボタン設定
std::string CACHE::path_buttonconf()
{
    return CACHE::path_root() + "button.conf";
}


// 板のルートパス
std::string CACHE::path_board_root( const std::string& url )
{
    // http:// を取り除く
    std::string url_board = DBTREE::url_boardbase( url );

#ifdef _DEBUG
    std::cout << "CACHE::path_board_root " << url << " -> " << url_board << std::endl;
#endif

    size_t i = url_board.find( "://" );
    if( i == std::string::npos ) return std::string();

    return CACHE::path_root() + url_board.substr( i + 3 );
}


std::string CACHE::path_article_summary( const std::string& url )
{
    return CACHE::path_board_root( url ) + "article-summary";
}

// board情報( navi2ch互換用 )
std::string CACHE::path_board_info( const std::string& url )
{
    return CACHE::path_board_root( url ) + "board.info";
}


// board情報( jd 用 )
std::string CACHE::path_jdboard_info( const std::string& url )
{
    return CACHE::path_board_root( url ) + "jdboard.info";
}


std::string CACHE::path_article_info_root( const std::string& url )
{
    return CACHE::path_board_root( url ) + "info/";
}


// スレの情報ファイル( navi2ch互換、実際には使用しない )
std::string CACHE::path_article_info( const std::string& url, const std::string& id )
{
    std::string id_str = id;

    // idに拡張子が付いてたら取る
    size_t i = id.find( "." );
    if( i != std::string::npos ) id_str = id.substr( 0, i );

    return CACHE::path_article_info_root( url ) + id_str;
}


// スレの拡張情報ファイル
std::string CACHE::path_article_ext_info( const std::string& url, const std::string& id )
{
    return CACHE::path_article_info( url, id ) + ".info";
}


std::string CACHE::path_dat( const std::string& url )
{
    return CACHE::path_board_root( url ) + DBTREE::article_id( url );
}



//
// 画像キャッシュのルートパス
//
std::string CACHE::path_img_root()
{
    return CACHE::path_root() + "image/";
}


//
// 画像キャッシュのinfoファイルのルートパス
//
std::string CACHE::path_img_info_root()
{
    return path_img_root() + "info/";
}


//
// 画像キャッシュファイルの名前
//
std::string CACHE::filename_img( const std::string& url )
{
    std::string file = MISC::tolower_str( url );
    file = MISC::replace_str( file, "http://", "" );
    file = MISC::replace_str( file, "/", "-" );
    file = MISC::url_encode( file.c_str(), file.length() );
    file = MISC::replace_str( file, "%", "" );
    
    return file;
}


//
// 画像キャッシュファイルのパス
//
std::string CACHE::path_img( const std::string& url )
{
    return CACHE::path_img_root() + filename_img( url );
}


//
// 画像infoファイルのパス
//
std::string CACHE::path_img_info( const std::string& url )
{
    return CACHE::path_img_info_root() + filename_img( url ) + ".info";
}



//
// キャッシュのルートディレクトリをmkdir
//
// 例えば  "/home/hoge/.jd/"　を作成
//
bool CACHE::mkdir_root()
{
    std::string path_root = CACHE::path_root();
    if( ! CACHE::jdmkdir( path_root ) ){
        MISC::ERRMSG( "can't create " + path_root );
        return false;
    }
    
    return true;
}



//
// 画像キャッシュのルートディレクトリをmkdir
//
// 例えば  "/home/hoge/.jd/image/" と "/home/hoge/.jd/image/info/" を作成
//
bool CACHE::mkdir_imgroot()
{
    // root
    std::string path_img_root = CACHE::path_img_root();
    if( ! CACHE::jdmkdir( path_img_root ) ){
        MISC::ERRMSG( "can't create " + path_img_root );
        return false;
    }

    // info ディレクトリ
    std::string path_info_root = CACHE::path_img_info_root();
    if( ! CACHE::jdmkdir( path_info_root ) ){
        MISC::ERRMSG( "can't create " + path_info_root );
        return false;
    }
    
    return true;
}

//
// キャッシュのひとつ上のディレクトリをmkdir
//
// 例えばキャッシュのルートディレクトリが "/home/hoge/.jd/hoge.2ch.net/hogeboard/" だったら
// "/home/hoge/.jd/hoge.2ch.net/" を作成する
//
bool CACHE::mkdir_parent_of_board( const std::string& url )
{
    std::string path_tmp = CACHE::path_board_root( url );

    size_t i = path_tmp.rfind( "/", path_tmp.length() -2 );
    if( i == std::string::npos ) return false;        
        
    std::string path_parent = path_tmp.substr( 0, i );

    if( ! CACHE::jdmkdir( path_parent ) ){
        MISC::ERRMSG( "can't create " + path_parent );
        return false;
    }
    
    return true;
}


//
// ある板のキャッシュのルートディレクトリをmkdir
//
// 例えば "/home/hoge/.jd/hoge.2ch.net/hogeboard/" と "/home/hoge/.jd/hoge.2ch.net/hogeboard/info/" を作成
//
bool CACHE::mkdir_boardroot( const std::string& url )
{
    // root
    std::string path_board_root = CACHE::path_board_root( url );
    if( ! CACHE::jdmkdir( path_board_root ) ){
        MISC::ERRMSG( "can't create " + path_board_root );
        return false;
    }

    // info ディレクトリ
    std::string path_info_root = CACHE::path_article_info_root( url );
    if( ! CACHE::jdmkdir( path_info_root ) ){
        MISC::ERRMSG( "can't create " + path_info_root );
        return false;
    }
    
    return true;
}




size_t CACHE::load_rawdata( const std::string& path, std::string& str )
{
    str.clear();
    
    std::ifstream fin;
    fin.open( path.c_str() );
    if( !fin.is_open() ) return 0;
    getline( fin, str, '\0' );
    fin.close();

    return str.length();
}


size_t CACHE::load_rawdata( const std::string& path, char* data, size_t n )
{
    size_t count = 0;
    std::ifstream fin;
    fin.open( path.c_str() );
    if( !fin.is_open() ) return 0;
    fin.read( data, n );
    count = fin.gcount();
    fin.close();

    return count;
}


bool CACHE::save_rawdata( const std::string& path, const std::string& str )
{
    return save_rawdata( path, str.c_str(), str.length() );
}



bool CACHE::save_rawdata( const std::string& path, const char* data, size_t n )
{
    std::ofstream fout;
    fout.open( path.c_str() );
    if( !fout.is_open() ){
        MISC::ERRMSG( "can't open " + path );
        return false;
    }
    fout.write( data, n );
    fout.close();

    return true;
}


long CACHE::is_file_exists( const std::string& path )
{
    struct stat buf_stat;

    if( stat( path.c_str(), &buf_stat ) != 0 ) return EXIST_ERROR;

    if( S_ISREG( buf_stat.st_mode ) ) return EXIST_FILE;
    if( S_ISDIR( buf_stat.st_mode ) ) return EXIST_DIR;

    return EXIST;
}


size_t CACHE::get_filesize( const std::string& path )
{
    struct stat buf_stat;

    if( stat( path.c_str(), &buf_stat ) != 0 ) return 0;
    if( S_ISREG( buf_stat.st_mode ) ) return buf_stat.st_size;
    return 0;
}



//
// mkdir
//
bool CACHE::jdmkdir( const std::string& path )
{
#ifdef _DEBUG
    std::cout << "CACHE::jdmkdir : path = " + path << std::endl;
#endif    

    if( CACHE::is_file_exists( path ) == EXIST_DIR ) return true;

    std::string target = path;
    
    if( path.find( "~/" ) == 0 ){

        std::string homedir = getenv( "HOME" );
        if( homedir.empty() ) return false;

        target = homedir + path.substr( 2 );
    }

    if( target[ 0 ] != '/' ) return false;
    if( target[ target.length() -1 ] != '/' ) target += "/";

#ifdef _DEBUG
    std::cout << "target = " << target << std::endl;
#endif
    
    // ルートからディレクトリがあるかチェックしていく。無ければ作る
    size_t i = 0;

    while( ( i = target.find( "/", i ) ) != std::string::npos ){

        ++i;
        std::string currentdir = target.substr( 0, i );

#ifdef _DEBUG
        std::cout << "mkdir " << currentdir << std::endl;
#endif
        
        if( CACHE::is_file_exists( currentdir ) == EXIST_DIR ) continue;

        if( mkdir( currentdir.c_str(), 0755 ) != 0 ){
            MISC::ERRMSG( "mkdir failed " + currentdir );
            return false;
        }
    }

    return true;
}



//
// file copy
//
bool CACHE::jdcopy( const std::string& file_from, const std::string& file_to )
{
    struct stat buf_stat;
    if( stat( file_from.c_str(), &buf_stat ) != 0 ) return false;

#ifdef _DEBUG
    std::cout << "CACHE::jdcopy : from = " << file_from << std::endl;
    std::cout << "to = " << file_to << std::endl;
    std::cout << "size = " << buf_stat.st_size << std::endl;
#endif    

    // 32Mより大きい画像はエラー出す
    if( buf_stat.st_size > 32 * 1024 * 1024 ){
            MISC::ERRMSG( "CACHE::jdcopy: size is too big : " + file_from );
            return false;
    }
    
    char* data = (char*)malloc( sizeof( char ) *  buf_stat.st_size );

    load_rawdata( file_from, data, buf_stat.st_size );
    save_rawdata( file_to, data, buf_stat.st_size );
    
    free( data );

    return true;
}


//
// 保存ダイアログを表示
//
// 戻り値は保存先(失敗したらempty())
//
std::string CACHE::open_save_diag( const std::string& file_from, const std::string& file_to )
{
    if( file_from.empty() ) return std::string();
    if( file_to.empty() ) return std::string();

    Gtk::FileChooserDialog diag( "save", Gtk::FILE_CHOOSER_ACTION_SAVE );
    diag.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
    diag.add_button( Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT );

    std::string name = MISC::get_filename( file_to );
    std::string dir = MISC::get_dir( file_to );
    if( dir.empty() ) dir = MISC::get_dir( file_from );

#ifdef _DEBUG
    std::cout << "CACHE::open_copy_diag\n";
    std::cout << "from = " << file_from  << std::endl;
    std::cout << "dir = " << dir << std::endl;
    std::cout << "name = " << name << std::endl;
#endif

    diag.set_current_folder( dir );
    diag.set_current_name( name );
    
    if( diag.run() == Gtk::RESPONSE_ACCEPT ){

        diag.hide();
        
        std::string path_to = diag.get_filename();

#ifdef _DEBUG
        std::cout << "to   = " << path_to  << std::endl;
#endif

        // 既にファイルがある場合は問い合わせる
        if( CACHE::is_file_exists( path_to ) == CACHE::EXIST_FILE ){

            Gtk::MessageDialog mdiag( "ファイルが存在します。ファイル名を変更しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );

            if( mdiag.run() ==  Gtk::RESPONSE_OK ) return CACHE::open_save_diag( file_from,  path_to );

            return std::string();
        }

        if( CACHE::jdcopy( file_from, path_to ) ) return path_to;
    }

    return std::string();
}


//
// dir ディレクトリ内のレギュラーファイルのリストを取得
//
std::list< std::string > CACHE::get_filelist( const std::string& dir )
{
    std::list< std::string > list_files;
#ifdef _DEBUG
    std::cout << "CACHE::get_filelist " << dir << std::endl;
#endif

    DIR *dirp = opendir( dir.c_str() );
    if( !dirp ) return list_files;

    struct dirent *direntry;
    while( ( direntry = readdir( dirp ) ) ){

        std::string filename = dir + direntry->d_name;

        if( is_file_exists( filename ) == EXIST_FILE ){

#ifdef _DEBUG
            std::cout << filename << std::endl;
#endif
            list_files.push_back( direntry->d_name );
        }
    }

    closedir( dirp );

    return list_files;
}


//
// dir ディレクトリ内のレギュラーファイルの合計サイズを取得
//
size_t CACHE::get_dirsize( const std::string& dir )
{
    size_t total_size = 0;

#ifdef _DEBUG
    std::cout << "CACHE::get_dirsize " << dir << std::endl;
#endif

    DIR *dirp = opendir( dir.c_str() );
    if( !dirp ) return 0;

    struct dirent *direntry;
    while( ( direntry = readdir( dirp ) ) ){
        std::string filename = dir + direntry->d_name;
        total_size += CACHE::get_filesize( filename );
    }

    closedir( dirp );

    return total_size;
}
