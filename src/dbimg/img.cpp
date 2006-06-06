// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "img.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "command.h"
#include "httpcode.h"
#include "cache.h"
#include "session.h"

#include <gtkmm.h>
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
    clear();
    read_info();
}




Img::~Img()
{
#ifdef _DEBUG
    std::cout << "Img::~Img url = " << m_url << std::endl;
#endif 

    if( m_fout ) fclose( m_fout );
    m_fout = NULL;
}


void Img::clear()
{
    m_mosaic = CONFIG::get_use_mosaic();
    m_zoom_to_fit = CONFIG::get_zoom_to_fit();
    m_size = 100;
    m_protect = false;
    memset( m_sign, 0, sizeof( m_sign ) );
    m_type = T_UNKNOWN;
}



const bool Img::is_cached()
{
    if( is_loading() ) return false;

    return ( get_code() == HTTP_OK );
}



//
// ロード開始
//
// receive_data()　と receive_finish() がコールバックされる
//
void Img::download_img()
{
#ifdef _DEBUG
    std::cout << "Img::download_img url = " << m_url <<  std::endl;
#endif

    if( is_loading() ) return;
    if( is_cached() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;

    // ダウンロード開始
    std::string path = CACHE::path_img( m_url );
    m_fout = fopen( path.c_str(), "wb" );
    if( m_fout == NULL ){
        MISC::ERRMSG( "fopen failed : " + path );
        return;
    }

    clear();
               
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
bool Img::save( const std::string& path_to )
{
    std::string dir = MISC::get_dir( path_to );
    if( dir.empty() ) dir = SESSION::dir_img_save();

    std::string name = MISC::get_filename( path_to );
    if( name.empty() ) name = MISC::get_filename( m_url );    

    std::string save_to = CACHE::open_save_diag( CACHE::path_img( m_url ), dir + name );

    if( ! save_to.empty() ){
        SESSION::set_dir_img_save( MISC::get_dir( save_to ) );
        return true;
    }

    return false;
}



//
// モザイクon/off
//
void Img::set_mosaic( bool mosaic )
{
    m_mosaic = mosaic;
    save_info();
}



//
// 保護モード
//
void Img::set_protect( bool protect )
{
    m_protect = protect;
    save_info();
}



//
// データ受信
//
void Img::receive_data( const char* data, size_t size )
{
    if( ! size ) return;

    if( m_fout ) fwrite( data, 1, size, m_fout );
    if( m_sign[ 0 ] == 0 ) memcpy( m_sign, data, MIN( sizeof( m_sign ) -1, size ) );

#ifdef _DEBUG
    std::cout << "Img::receive_data code = " << code() << " "
              << current_length() << " / " << total_length() << std::endl;
#endif
}



//
// ロード終了
//
void Img::receive_finish()
{
    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    m_type = T_NOIMG;

    // シグネチャを見て画像かどうかをチェック
    if( get_code() == HTTP_OK ){

        // jpeg は FF D8
        if( m_sign[ 0 ] == 0xFF
            && m_sign[ 1 ] == 0xD8 ) m_type = T_JPG;

        // png は 0x89 0x50 0x4e 0x47 0xd 0xa 0x1a 0xa
        else if( m_sign[ 0 ] == 0x89
                 && m_sign[ 1 ] == 0x50
                 && m_sign[ 2 ] == 0x4e
                 && m_sign[ 3 ] == 0x47
                 && m_sign[ 4 ] == 0x0d
                 && m_sign[ 5 ] == 0x0a
                 && m_sign[ 6 ] == 0x1a
                 && m_sign[ 7 ] == 0x0a ) m_type = T_PNG;

        // gif
        else if( m_sign[ 0 ] == 'G'
                 && m_sign[ 1 ] == 'I'
                 && m_sign[ 2 ] == 'F' ) m_type = T_GIF;

        if( m_type == T_NOIMG ){
            set_code( HTTP_CANCEL );
            set_str_code( "画像ファイルではありません" );
        }
    }

    if( m_type == T_NOIMG ){

        // キャッシュを消しておく
        std::string path = CACHE::path_img( m_url );
        if( CACHE::is_file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
    }

    save_info();

    CORE::core_set_command( "redraw", m_url );
    CORE::core_set_command( "redraw_article" );

#ifdef _DEBUG
    std::cout << "Img::receive_finish code = " << code() << std::endl
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

    JDLIB::ConfLoader cf( CACHE::path_img_info( m_url ), std::string() );

    m_refurl = cf.get_option( "refurl", "" );
    set_code( cf.get_option( "code", HTTP_ERR ) );
    set_str_code( cf.get_option( "str_code", "" ) );
    set_total_length( cf.get_option( "byte", 0 ) );
    m_mosaic = cf.get_option( "mosaic", CONFIG::get_use_mosaic() );
    m_protect = cf.get_option( "protect", 0 );
    m_type = cf.get_option( "type", T_UNKNOWN );

    if( ! total_length() ) set_total_length( CACHE::get_filesize( CACHE::path_img( m_url ) ) );
    set_current_length( total_length() );

#ifdef _DEBUG
    std::cout << "refurl = " << m_refurl << std::endl;
    std::cout << "code = " << code() << std::endl;
    std::cout << "str_code = " << str_code() << std::endl;
    std::cout << "byte = " << current_length() << std::endl;
    std::cout << "mosaic = " << m_mosaic << std::endl;
    std::cout << "protect = " << m_protect << std::endl;
    std::cout << "type = " << m_type << std::endl;
#endif
}


//
// 情報保存
//
void Img::save_info()
{
    if( is_loading() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;

    std::string path_info = CACHE::path_img_info( m_url );
    std::ostringstream oss;
    oss << "url = " << m_url << std::endl
        << "refurl = " << m_refurl << std::endl
        << "code = " << get_code() << std::endl
        << "str_code = " << get_str_code() << std::endl
        << "byte = " << current_length() << std::endl
        << "mosaic = " << m_mosaic << std::endl
        << "protect = " << m_protect << std::endl
        << "type = " << m_type << std::endl;

#ifdef _DEBUG
    std::cout << "Img::save_info file = " << path_info << std::endl;
    std::cout << oss.str() << std::endl;
#endif

    CACHE::save_rawdata( path_info, oss.str() );
}
