// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "cache.h"

#include "skeleton/msgdiag.h"
#include "skeleton/filediag.h"

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
    return CACHE::path_root() + "jd.conf";
}


// 旧設定ファイル
std::string CACHE::path_conf_old()
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
// キャッシュ構造は navi2ch の上位互換なので path_cacheroot = "~/.navi2ch/" とすればnavi2chとキャッシュを共有できる
std::string CACHE::path_root()
{
    std::string root;

    if( getenv( "JD_CACHE" ) ) root = getenv( "JD_CACHE" );
    else root = "~/.jd/";

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


// ユーザーコマンド設定ファイル
std::string CACHE::path_usrcmd()
{
    return CACHE::path_root() +  "usrcmd.txt";
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
    std::string boardbase = DBTREE::url_boardbase( url );
    return path_board_root_fast( boardbase );
}


//
// 板のルートパス(高速版)
//
// DBTREE::url_boardbase( url ) を使わないであらかじめ boardbase を与える
//
std::string CACHE::path_board_root_fast( const std::string& boardbase )
{
    // http:// を取り除く
    size_t i = boardbase.find( "://" );
    if( i == std::string::npos ) return std::string();

    return CACHE::path_root() + boardbase.substr( i + 3 );
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
// 保護画像キャッシュのルートパス
//
std::string CACHE::path_img_protect_root()
{
    return CACHE::path_root() + "image_protect/";
}


//
// 保護画像キャッシュのinfoファイルのルートパス
//
std::string CACHE::path_img_protect_info_root()
{
    return path_img_protect_root() + "info/";
}


//
// 画像あぼーんinfoファイルのルートパス
//
std::string CACHE::path_img_abone_root()
{
    return path_root() + "image_abone/";
}


//
// 書き込みログ
//
std::string CACHE::path_postlog()
{
    return CACHE::path_root() +  "post.log";
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
// 画像キャッシュ情報ファイルの名前
//
std::string CACHE::filename_img_info( const std::string& url )
{
    return filename_img( url ) + ".info";
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
    return CACHE::path_img_info_root() + filename_img_info( url );
}


//
// 保護画像キャッシュファイルのパス
//
std::string CACHE::path_img_protect( const std::string& url )
{
    return CACHE::path_img_protect_root() + filename_img( url );
}


//
// 保護画像infoファイルのパス
//
std::string CACHE::path_img_protect_info( const std::string& url )
{
    return CACHE::path_img_protect_info_root() + filename_img_info( url );
}


//
// 画像あぼーんinfoファイルのパス
//
std::string CACHE::path_img_abone( const std::string& url )
{
    return CACHE::path_img_abone_root() + filename_img_info( url );
}


//
// アスキーアートファイル
//
std::string CACHE::path_aalist()
{
    return CACHE::path_root() +  "aalist.txt";
}


//
// アスキーアートファイル格納用ディレクトリ .jd/aa/
//
std::string CACHE::path_aadir()
{
    return CACHE::path_root() +  "aa/";
}


//
// css
//
std::string CACHE::path_css()
{
    return CACHE::path_root() +  "jd.css";
}


//
// res.html
//
std::string CACHE::path_reshtml()
{
    return CACHE::path_root() +  "Res.html";
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
// 例えば  "/home/hoge/.jd/image/" と "/home/hoge/.jd/image/info/" などを作成
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
    
    // abone ディレクトリ
    std::string path_abone_root = CACHE::path_img_abone_root();
    if( ! CACHE::jdmkdir( path_abone_root ) ){
        MISC::ERRMSG( "can't create " + path_abone_root );
        return false;
    }

    return true;
}


//
// 保護画像キャッシュのルートディレクトリをmkdir
//
bool CACHE::mkdir_imgroot_favorite()
{
    // root
    std::string path_img_root = CACHE::path_img_protect_root();
    if( ! CACHE::jdmkdir( path_img_root ) ){
        MISC::ERRMSG( "can't create " + path_img_root );
        return false;
    }

    // info ディレクトリ
    std::string path_info_root = CACHE::path_img_protect_info_root();
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


bool CACHE::save_rawdata( const std::string& path, const std::string& str, bool append )
{
    return save_rawdata( path, str.c_str(), str.length(), append );
}



bool CACHE::save_rawdata( const std::string& path, const char* data, size_t n, bool append )
{
    std::ofstream fout;
    if( append ) fout.open( path.c_str(), std::ios::app );
    else fout.open( path.c_str() );
    if( !fout.is_open() ){
        MISC::ERRMSG( "can't open " + path );
        return false;
    }
    fout.write( data, n );
    fout.close();

    return true;
}


long CACHE::file_exists( const std::string& path )
{
    struct stat buf_stat;

    if( path.empty() ) return EXIST_ERROR;
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


time_t CACHE::get_filemtime( const std::string& path )
{
    struct stat buf_stat;

    if( stat( path.c_str(), &buf_stat ) != 0 ) return 0;
    if( S_ISREG( buf_stat.st_mode ) ) return buf_stat.st_mtime;
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

    if( CACHE::file_exists( path ) == EXIST_DIR ) return true;

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
        
        if( CACHE::file_exists( currentdir ) == EXIST_DIR ) continue;

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
// mv
//
bool CACHE::jdmv( const std::string& file_from, const std::string& file_to )
{
    if( CACHE::jdcopy( file_from, file_to ) ){
        unlink( file_from.c_str() );
        return true;
    }

    return false;
}


//
// ファイル選択ダイアログを表示する
//
// parent == NULL の時はメインウィンドウをparentにする
// open_path はデフォルトの参照先
// 戻り値は選択されたファイルのpath
//
std::string CACHE::open_load_diag( Gtk::Window* parent, const std::string& open_path, const int type )
{
    std::string dir = MISC::get_dir( open_path );
    if( dir.empty() ) dir = getenv( "HOME" );

    SKELETON::FileDiag diag( parent, "ファイルを開く", Gtk::FILE_CHOOSER_ACTION_OPEN );

    diag.set_current_folder( dir );

    Gtk::FileFilter filter;
    switch( type ) 
    {
        case FILE_TYPE_TEXT:
            filter.set_name( "全てのテキストファイル" );
            filter.add_mime_type( "text/plain" );
            diag.add_filter( filter );
            break;
    }

    Gtk::FileFilter all;
    all.set_name( "全てのファイル" );
    all.add_pattern( "*" );
    diag.add_filter( all );

    if( diag.run() == Gtk::RESPONSE_ACCEPT )
    {
        diag.hide();

        return diag.get_filename();
    }

    return std::string();
}



//
// 保存ダイアログを表示して file_from を 保存する
//
// parent == NULL の時はメインウィンドウをparentにする
// file_toはデフォルトの保存先
// 戻り値は保存先(保存に失敗したらempty())
//
std::string CACHE::open_save_diag( Gtk::Window* parent, const std::string& file_from, const std::string& file_to )
{
    if( file_from.empty() ) return std::string();
    if( file_to.empty() ) return std::string();

    SKELETON::FileDiag diag( parent, "save", Gtk::FILE_CHOOSER_ACTION_SAVE );

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
        if( CACHE::file_exists( path_to ) == CACHE::EXIST_FILE ){

            SKELETON::MsgDiag mdiag( parent, "ファイルが存在します。ファイル名を変更しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );

            if( mdiag.run() ==  Gtk::RESPONSE_OK ) return CACHE::open_save_diag( parent, file_from,  path_to );

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

        if( file_exists( filename ) == EXIST_FILE ){

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
int64_t CACHE::get_dirsize( const std::string& dir )
{
    int64_t total_size = 0;

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

#ifdef _DEBUG
    std::cout << "size = " << total_size << std::endl;
#endif

    return total_size;
}
