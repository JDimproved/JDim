// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "img.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscgtk.h"
#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "command.h"
#include "httpcode.h"
#include "cache.h"
#include "session.h"

#include <sstream>

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


using namespace DBIMG;


Img::Img( const std::string& url )
    : SKELETON::Loadable()
    ,m_url( url )
    ,m_fout( 0 )
{
#ifdef _DEBUG
    std::cout << "Img::Img url = " << m_url <<  std::endl;
#endif

    reset();
}




Img::~Img()
{
#ifdef _DEBUG
    std::cout << "Img::~Img url = " << m_url << std::endl;
#endif 

    if( m_fout ) fclose( m_fout );
    m_fout = NULL;
}


//
// リセット( 情報をクリアしてinfoファイル再読み込み )
//
void Img::reset()
{
    clear();
    read_info();
}


// 情報クリア
void Img::clear()
{
    m_protect = false;

    m_refurl = std::string();
    clear_load_data(); // HTTPコードのクリア

    m_mosaic = CONFIG::get_use_mosaic();
    m_zoom_to_fit = CONFIG::get_zoom_to_fit();
    m_size = 100;
    m_type = T_UNKNOWN;
    m_width = 0;
    m_height = 0;
    m_abone = false;
}



const bool Img::is_cached()
{
    if( is_loading() ) return false;

    return ( get_code() == HTTP_OK );
}


//
// あぼーん状態セット
//
// キャッシュに無くてもinfoを作るので is_cached() でチェックしない
//
void Img::set_abone( bool abone )
{
    if( m_abone == abone ) return;

#ifdef _DEBUG
    std::cout << "Img::set_abone = " << abone << std::endl;
#endif

    if( abone ) clear();
    m_abone = abone;
    save_info();
}


std::string Img::get_cache_path()
{
    if( m_protect ) return CACHE::path_img_protect( m_url );

    return CACHE::path_img( m_url );
}


//
// ロード開始
//
// receive_data()　と receive_finish() がコールバックされる
//
void Img::download_img( const std::string& refurl )
{
#ifdef _DEBUG
    std::cout << "Img::download_img url = " << m_url
              << " refurl = " << refurl <<  std::endl;
#endif

    if( is_loading() ) return;
    if( is_cached() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;

    // ダウンロード開始
    std::string path = get_cache_path();
    m_fout = fopen( path.c_str(), "wb" );
    if( m_fout == NULL ){
        MISC::ERRMSG( "fopen failed : " + path );
        return;
    }

    clear();
    m_refurl = refurl;
               
    JDLIB::LOADERDATA data;
    data.url = m_url;
    data.agent = CONFIG::get_agent_for_data();
    if( CONFIG::get_use_proxy_for_data() ) data.host_proxy = CONFIG::get_proxy_for_data();
    else data.host_proxy = std::string();
    data.port_proxy = CONFIG::get_proxy_port_for_data();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout_img();

    if( !start_load( data ) ) receive_finish();
    else CORE::core_set_command( "redraw", m_url );
}



//
// キャッシュ保存
//
// path_to はデフォルトのファイル名
//
bool Img::save( Gtk::Window* parent, const std::string& path_to )
{
    if( ! is_cached() ) return false;

    std::string dir = MISC::get_dir( path_to );
    if( dir.empty() ) dir = SESSION::dir_img_save();

    std::string name = MISC::get_filename( path_to );
    if( name.empty() ) name = MISC::get_filename( m_url );    

    std::string save_to = CACHE::open_save_diag( parent, get_cache_path(), dir + name );

    if( ! save_to.empty() ){
        SESSION::set_dir_img_save( MISC::get_dir( save_to ) );
        return true;
    }

    return false;
}


//
// 高さ、幅
//
const int Img::get_width()
{
    if( ! is_cached() ) return 0;

    if( ! m_width ){
        MISC::get_img_size( get_cache_path(), m_width, m_height );
        if( m_width ) save_info();
    }

    return m_width;
}

const int Img::get_height()
{
    if( ! is_cached() ) return 0;

    if( ! m_height ){
        MISC::get_img_size( get_cache_path(), m_width, m_height );
        if( m_height ) save_info();
    }

    return m_height;
}


//
// モザイクon/off
//
void Img::set_mosaic( bool mosaic )
{
    if( ! is_cached() ) return;

    m_mosaic = mosaic;
    save_info();

    // 再描画
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}



//
// 保護モード
//
void Img::set_protect( bool protect )
{
    if( ! is_cached() ) return;

    if( m_protect == protect ) return;

    if( protect ){
        CACHE::jdmv( CACHE::path_img( m_url ), CACHE::path_img_protect( m_url ) );
        CACHE::jdmv( CACHE::path_img_info( m_url ), CACHE::path_img_protect_info( m_url ) );
    }
    else{
        CACHE::jdmv( CACHE::path_img_protect( m_url ), CACHE::path_img( m_url ) );
        CACHE::jdmv( CACHE::path_img_protect_info( m_url ), CACHE::path_img_info( m_url ) );
    }

    m_protect = protect;
}



//
// データ受信
//
void Img::receive_data( const char* data, size_t size )
{
    if( ! size ) return;

    // 先頭のシグネチャを見て画像かどうかをチェック
    if( m_type == T_UNKNOWN && get_code() == HTTP_OK ){

#ifdef _DEBUG
        assert( size > 8 );
#endif        
        // ファイル判定用のシグネチャ
        const unsigned char *sign = ( const unsigned char* )data;

        // jpeg は FF D8
        if( sign[ 0 ] == 0xFF
            && sign[ 1 ] == 0xD8 ) m_type = T_JPG;

        // png は 0x89 0x50 0x4e 0x47 0xd 0xa 0x1a 0xa
        else if( sign[ 0 ] == 0x89
                 && sign[ 1 ] == 0x50
                 && sign[ 2 ] == 0x4e
                 && sign[ 3 ] == 0x47
                 && sign[ 4 ] == 0x0d
                 && sign[ 5 ] == 0x0a
                 && sign[ 6 ] == 0x1a
                 && sign[ 7 ] == 0x0a ) m_type = T_PNG;

        // gif
        else if( sign[ 0 ] == 'G'
                 && sign[ 1 ] == 'I'
                 && sign[ 2 ] == 'F' ) m_type = T_GIF;

        // 画像ファイルではない
        else{
            m_type = T_NOIMG;
            stop_load();
        }

        // 指定サイズよりも大きい
        if( m_type != T_NOIMG && total_length() > (size_t)CONFIG::get_max_img_size() * 1024 * 1024 ){
            m_type = T_LARGE;
            stop_load();
        }
    }

    if( m_fout &&
        ( m_type != T_NOIMG && m_type != T_LARGE ) ) fwrite( data, 1, size, m_fout );

#ifdef _DEBUG
    std::cout << "Img::receive_data code = " << get_code() << " "
              << current_length() << " / " << total_length() << std::endl
              << "type = " << m_type << std::endl;
#endif
}


//
// ロード終了
//
void Img::receive_finish()
{
    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    // 指定サイズよりも大きい
    if( total_length() > (size_t)CONFIG::get_max_img_size() * 1024 * 1024 ){
        m_type = T_LARGE;
        set_code( HTTP_CANCEL );
    }

    // エラーメッセージのセット
    if( m_type == T_NOIMG ){
        set_code( HTTP_CANCEL );
        set_str_code( "画像ファイルではありません" );
        set_current_length( 0 );
    }

    else if( m_type == T_LARGE ){
        set_code( HTTP_CANCEL );
        std::stringstream ss;
        ss << "ファイルサイズが大きすぎます ( " << ( total_length() / 1024 / 1024 ) << " M )";
        set_str_code( ss.str() );
        set_current_length( 0 );
    }
    else if( m_type == T_UNKNOWN ) set_current_length( 0 );

    set_total_length( current_length() );

    // 読み込み失敗の場合はファイルを消しておく
    if( ! total_length() ){
        std::string path = get_cache_path();
        if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
    }

    // 読み込み失敗の場合でもエラーメッセージを残すので info　は保存する
    save_info();

    CORE::core_set_command( "redraw", m_url );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );

#ifdef _DEBUG
    std::cout << "Img::receive_finish code = " << get_code() << std::endl
              << "total byte = " << total_length() << std::endl
              << "type = " << m_type << std::endl;
#endif
}




//
// キャッシュ情報読み込み
//
void Img::read_info()
{
#ifdef _DEBUG
    std::cout << "Img::read_info\n";
#endif

    std::string filename = CACHE::filename_img_info( m_url );
    std::string path_info = CACHE::path_img_info_root() + filename;
    bool exist = false;

    do{

        // 通常
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ){ 
            m_protect = false;
            exist = true;
            break;
        }

        // 保護の場合
        path_info = CACHE::path_img_protect_info_root() + filename;
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ){
            m_protect = true;
            exist = true;
            break;
        }

        // あぼーんの場合
        path_info = CACHE::path_img_abone_root() + filename;
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ){
            m_abone = true;
            exist = false;
            break;
        }

    }while( 0 );

    if( exist ){

        JDLIB::ConfLoader cf( path_info, std::string() );

        m_refurl = cf.get_option( "refurl", "" );
        set_code( cf.get_option( "code", HTTP_INIT ) );
        set_str_code( cf.get_option( "str_code", "" ) );
        set_total_length( cf.get_option( "byte", 0 ) );
        m_mosaic = cf.get_option( "mosaic", CONFIG::get_use_mosaic() );
        m_type = cf.get_option( "type", T_UNKNOWN );
        m_width = cf.get_option( "width", 0 );
        m_height = cf.get_option( "height", 0 );
        m_abone = cf.get_option( "abone", 0 );

        if( ! total_length() ) set_total_length( CACHE::get_filesize( get_cache_path() ) );
        set_current_length( total_length() );
    }

#ifdef _DEBUG
    std::cout << "path_info = " << path_info << std::endl;
    std::cout << "exist = " << exist << std::endl;
    std::cout << "protect = " << m_protect << std::endl;
    std::cout << "refurl = " << m_refurl << std::endl;
    std::cout << "code = " << get_code() << std::endl;
    std::cout << "str_code = " << get_str_code() << std::endl;
    std::cout << "byte = " << total_length() << std::endl;
    std::cout << "mosaic = " << m_mosaic << std::endl;
    std::cout << "type = " << m_type << std::endl;
    std::cout << "width = " << m_width << std::endl;
    std::cout << "height = " << m_height << std::endl;
    std::cout << "abone = " << m_abone << std::endl;
#endif
}


//
// 情報保存
//
void Img::save_info()
{
    if( is_loading() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;
    if( ! CACHE::mkdir_imgroot_favorite() ) return;

    std::string path_info;

    if( ! is_cached() && ! m_abone ){

        // あぼーん情報ファイルがあったら消しておく
        path_info = CACHE::path_img_abone( m_url );
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ){
#ifdef _DEBUG
            std::cout << "unlink " << path_info << std::endl;
#endif 
            unlink( path_info.c_str() );
        }

        if( get_code() == HTTP_INIT ) return;
    }

    if( m_protect ) path_info = CACHE::path_img_protect_info( m_url );
    if( m_abone ) path_info = CACHE::path_img_abone( m_url );
    else path_info = CACHE::path_img_info( m_url );

    std::ostringstream oss;
    oss << "url = " << m_url << std::endl
        << "refurl = " << m_refurl << std::endl
        << "code = " << get_code() << std::endl
        << "str_code = " << get_str_code() << std::endl
        << "byte = " << total_length() << std::endl
        << "mosaic = " << m_mosaic << std::endl
        << "type = " << m_type << std::endl
        << "width = " << m_width << std::endl
        << "height = " << m_height << std::endl
        << "abone = " << m_abone << std::endl;

#ifdef _DEBUG
    std::cout << "Img::save_info file = " << path_info << std::endl;
    std::cout << "protect = " << m_protect << std::endl;
    std::cout << oss.str() << std::endl;
#endif

    CACHE::save_rawdata( path_info, oss.str() );
}
