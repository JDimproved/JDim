// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "img.h"
#include "imginterface.h"

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

// 最大リダイレクト回数
#define MAX_REDIRECT 5  


// 情報ファイルから値を読み込み
// dbtree/articlebase.cpp からコピペ
#define GET_INFOVALUE(target,targetname) \
do { \
target = std::string(); \
option = targetname; \
it2 = it; \
while( it2 != lines.end() && ( *it2 ).find( option ) != 0 ) ++it2; \
if( it2 != lines.end() ){ \
    target = ( *it2 ).substr( option.length() ); \
    it = ++it2; \
} } while( false )


using namespace DBIMG;


Img::Img( const std::string& url )
    : SKELETON::Loadable()
    ,m_url( url )
    ,m_count_redirect( 0 )
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
    if( ! total_length() ) return false;

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
void Img::download_img( const std::string refurl )
{
    // ダウンロード初回(リダイレクトでは無い)
    if( ! m_count_redirect ) m_url_alt = std::string();

#ifdef _DEBUG
    std::cout << "Img::download_img url = ";
    if( ! m_url_alt.empty() ) std::cout << m_url_alt << "(" << m_count_redirect << ")" << std::endl;
    else std::cout << m_url << std::endl;
    std::cout << "refurl = " << refurl <<  std::endl;
#endif

    if( is_loading() ) return;
    if( is_cached() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;

    // ダウンロード開始
    std::string path = get_cache_path();
    m_fout = fopen( path.c_str(), "wb" );
    if( m_fout == NULL ){
        m_type = T_OPENFAILED;
        receive_finish();
        return;
    }

    clear();
    m_refurl = refurl;
               
    JDLIB::LOADERDATA data;

    if( ! m_url_alt.empty() ) data.url = m_url_alt;
    else data.url = m_url;

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


// 拡張子が偽装されているか
const bool Img::is_fake()
{
    if( ! is_cached() ) return false;

    bool ret = false;
    std::string url = m_url;
    if( ! m_url_alt.empty() ) url = m_url_alt;

    if( DBIMG::is_jpg( url ) && m_type != T_JPG ) ret = true;
    if( DBIMG::is_png( url ) && m_type != T_PNG ) ret = true;
    if( DBIMG::is_gif( url ) && m_type != T_GIF ) ret = true;

#ifdef _DEBUG
    std::cout << "Img::is_fake url = " << url << " ret = " << ret << std::endl;
#endif

    return ret;
}


//
// データ受信
//
void Img::receive_data( const char* data, size_t size )
{
    if( ! size ) return;

#ifdef _DEBUG
    std::cout << "Img::receive_data code = " << get_code() << std::endl
              << "size / total = " << current_length() << " / " << total_length() << std::endl;
#endif

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

            // リダイレクトしたら 404 を疑う
            // データに "404" "not" "found" という文字列が含まれていたら not found と仮定
            if( ! m_url_alt.empty() ){

                // std::stringにいきなりデータを入れるのはなんとなく恐いので strncasecmp() を使用
                unsigned char notfound = 0;
                for( unsigned int i = 0; i < size; ++i ){
                    if( strncasecmp( data + i, "404", 3 ) == 0 ) notfound |= 1;
                    if( strncasecmp( data + i, "not", 3 ) == 0 ) notfound |= 2;
                    if( strncasecmp( data + i, "found", 5 ) == 0 ) notfound |= 4;
                }
                if( notfound == 7 )  m_type = T_NOT_FOUND;
            }

            stop_load();

#ifdef _DEBUG
            std::cout << data << std::endl;
#endif
        }

        // 指定サイズよりも大きい
        if( m_type != T_NOIMG && m_type != T_NOT_FOUND && total_length() > (size_t)CONFIG::get_max_img_size() * 1024 * 1024 ){
            m_type = T_LARGE;
            stop_load();
        }
    }

    if( m_fout &&
        ( m_type != T_NOIMG && m_type != T_NOT_FOUND && m_type != T_LARGE ) ){

        if( fwrite( data, 1, size, m_fout ) != size ){
            m_type = T_WRITEFAILED; // 書き込み失敗
            stop_load();
        }
    }

#ifdef _DEBUG
    std::cout << "type = " << m_type << std::endl;
#endif
}


//
// ロード終了
//
void Img::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Img::receive_finish code = " << get_code() << std::endl
              << "total byte = " << total_length() << std::endl
              << "cookies : " << std::endl;

    if( cookies().size() ){
        std::list< std::string >::iterator it = cookies().begin();
        for( ; it != cookies().end() ; ++it ) std::cout << *it << std::endl;
    }
#endif

    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    // データが無い
    if( get_code() == HTTP_OK && ! current_length() ) m_type = T_NODATA;

    // リダイレクト
    if( get_code() == HTTP_REDIRECT ){
#ifdef _DEBUG
        std::cout << "302 redirect url = " << location() << std::endl;
#endif
        // アドレスに "404", ".htm" が含まれていたら not found と仮定
        std::string url_tmp = MISC::tolower_str( location() );
        if( url_tmp.find( "404" ) != std::string::npos && url_tmp.find( ".htm" ) != std::string::npos ) m_type = T_NOT_FOUND;

        else if( ! location().empty() && m_count_redirect < MAX_REDIRECT ){
            ++m_count_redirect;
            m_url_alt = location();
            download_img( m_refurl );
            return;
        }
        else m_type = T_NODATA;
    }
    m_count_redirect = 0;


    //////////////////////////////////////////////////
    // エラーメッセージのセット

    if( m_type == T_NOIMG ){
        set_code( HTTP_ERR );
        set_str_code( "画像ファイルではありません" );
        set_current_length( 0 );
    }

    else if( m_type == T_NOT_FOUND ){
        set_code( HTTP_NOT_FOUND );
        set_str_code( "404 Not Found" );
        set_current_length( 0 );
    }

    else if( m_type == T_LARGE ){
        set_code( HTTP_ERR );
        std::stringstream ss;
        ss << "ファイルサイズが大きすぎます ( " << ( total_length() / 1024 / 1024 ) << " M )";
        set_str_code( ss.str() );
        set_current_length( 0 );
    }

    else if( m_type == T_OPENFAILED ){
        set_code( HTTP_ERR );
        set_str_code( "ファイルのオープンに失敗しました" );
        set_current_length( 0 );
    }

    else if( m_type == T_WRITEFAILED ){
        set_code( HTTP_ERR );
        set_str_code( "ハードディスクへの書き込みに失敗しました" );
        set_current_length( 0 );
    }

    else if( m_type == T_NODATA ){
        set_code( HTTP_ERR );
        set_str_code( "サーバ上にファイルが存在しません" );
        set_current_length( 0 );
    }

    else if( get_code() == HTTP_OK && m_type == T_UNKNOWN ){
        set_code( HTTP_ERR );
        set_str_code( "未知の画像形式です" );
        set_current_length( 0 );
    }

    set_total_length( current_length() );

    // 読み込み失敗の場合はファイルを消しておく
    if( ! total_length() ){
        std::string path = get_cache_path();
        if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
#ifdef _DEBUG
        std::cout << "unlink cache\n";
#endif
    }

#ifdef _DEBUG
    std::cout << "type = " << m_type << std::endl
              << "refurl = " << m_refurl << std::endl;
#endif

    // 拡張子が偽装されている時はモザイク表示にする
    if( is_fake() ) m_mosaic = true;

    // 読み込み失敗の場合でもエラーメッセージを残すので info　は保存する
    save_info();

    CORE::core_set_command( "redraw", m_url );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
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
    m_abone = false;

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

/*
        JDLIB::ConfLoader cf( path_info, std::string() );

        m_refurl = cf.get_option( "refurl", "" );

        set_code( cf.get_option( "code", HTTP_INIT ) );

        set_str_code( cf.get_option( "str_code", "" ) );

        set_total_length( cf.get_option( "byte", 0 ) );

        m_mosaic = cf.get_option( "mosaic", CONFIG::get_use_mosaic() );

        m_type = cf.get_option( "type", T_UNKNOWN );

        m_width = cf.get_option( "width", 0 );

        m_height = cf.get_option( "height", 0 );
*/
        // TODO : JDLIB::ConfLoaderFast を作る
        std::string str_info, str_tmp;
        std::list< std::string > list_tmp;
        std::list< std::string >::iterator it_tmp;
        CACHE::load_rawdata( path_info, str_info );

        std::list< std::string > lines = MISC::get_lines( str_info );
        std::list < std::string >::iterator it = lines.begin(), it2;
        std::string option; // GET_INFOVALUE　で使用

        GET_INFOVALUE( m_refurl, "refurl = " );

        set_code( HTTP_INIT );
        GET_INFOVALUE( str_tmp, "code = " );
        if( ! str_tmp.empty() ) set_code( atoi( str_tmp.c_str() ) );
        
        GET_INFOVALUE( str_tmp, "str_code = " );
        set_str_code( str_tmp );

        set_total_length( 0 );
        GET_INFOVALUE( str_tmp, "byte = " );
        if( ! str_tmp.empty() ) set_total_length( atoi( str_tmp.c_str() ) );

        m_mosaic = CONFIG::get_use_mosaic();
        GET_INFOVALUE( str_tmp, "mosaic = " );
        if( ! str_tmp.empty() ) m_mosaic =  atoi( str_tmp.c_str() );

        m_type = T_UNKNOWN;
        GET_INFOVALUE( str_tmp, "type = " );
        if( ! str_tmp.empty() ) m_type =  atoi( str_tmp.c_str() );

        m_width = 0;
        GET_INFOVALUE( str_tmp, "width = " );
        if( ! str_tmp.empty() ) m_width =  atoi( str_tmp.c_str() );

        m_height = 0;
        GET_INFOVALUE( str_tmp, "height = " );
        if( ! str_tmp.empty() ) m_height =  atoi( str_tmp.c_str() );

        if( ! total_length() ){
            set_total_length( CACHE::get_filesize( get_cache_path() ) );
            if( total_length() ) save_info();
        }
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
        << "height = " << m_height << std::endl;

#ifdef _DEBUG
    std::cout << "Img::save_info file = " << path_info << std::endl;
    std::cout << "protect = " << m_protect << std::endl;
    std::cout << oss.str() << std::endl;
#endif

    CACHE::save_rawdata( path_info, oss.str() );
}
