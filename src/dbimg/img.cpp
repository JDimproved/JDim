// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "img.h"
#include "imginterface.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/imgloader.h"
#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "command.h"
#include "httpcode.h"
#include "cache.h"
#include "session.h"
#include "global.h"
#include "urlreplacemanager.h"

#include <sstream>
#include <cstring>

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


enum
{
    MAX_REDIRECT = 5  // 最大リダイレクト回数
};


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

    set_priority_low();

    m_imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( url );

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


void Img::clock_in()
{
    // ロード待ち
    if( m_wait ){

        --m_wait_counter;
        if( m_wait_counter <= 0 ) download_img( m_refurl, m_mosaic, 0 );
    }
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
    m_width_emb = 0;
    m_height_emb = 0;
    m_width_mosaic = 0;
    m_height_mosaic = 0;
    m_abone = false;
    m_wait = false;
    m_wait_counter = 0;
}


//
// ロード待ち状態セット/リセット
//
bool Img::set_wait( const std::string& refurl, const bool mosaic, const int waitsec )
{
    if( waitsec && ! m_wait ){

#ifdef _DEBUG
        std::cout << "Img::set_wait url = " << m_url << std::endl;
#endif
        m_wait = true;
        m_wait_counter = (1000/TIMER_TIMEOUT) * waitsec;
        DBIMG::set_clock_in( this );

        m_refurl = refurl;
        m_mosaic = mosaic;

        CORE::core_set_command( "redraw", m_url );
        CORE::core_set_command( "redraw_article" );
        CORE::core_set_command( "redraw_message" );

        return true;
    }

    return false;
}


void Img::reset_wait()
{
    if( m_wait ){
#ifdef _DEBUG
        std::cout << "Img::reset_wait url = " << m_url << std::endl;
#endif
        m_wait = false;
        m_wait_counter = 0;
        DBIMG::reset_clock_in( this );

        CORE::core_set_command( "redraw", m_url );
        CORE::core_set_command( "redraw_article" );
        CORE::core_set_command( "redraw_message" );
    }
}


// スレ埋め込み画像の高さ、幅
int Img::get_width_emb()
{
    if( ! m_width_emb ) set_embedded_size();

    return m_width_emb;
}

int Img::get_height_emb()
{
    if( ! m_height_emb ) set_embedded_size();

    return m_height_emb;
}


// モザイク処理時に縮小するサイズ
int Img::get_width_mosaic()
{
    if( ! m_width_mosaic ) set_mosaic_size();

    return m_width_mosaic;
}

int Img::get_height_mosaic()
{
    if( ! m_height_mosaic ) set_mosaic_size();

    return m_height_mosaic;
}


bool Img::is_cached()
{
    if( is_loading() ) return false;
    if( is_wait() ) return false;
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
// refurl : 参照元のスレのアドレス
// mosaic : モザイク表示するか
// waitsec: 指定した秒数経過後にロード開始
//
void Img::download_img( const std::string refurl, const bool mosaic, const int waitsec )
{
    // ダウンロード初回(リダイレクトでは無い)
    if( ! m_count_redirect ) m_url_alt = std::string();

#ifdef _DEBUG
    std::cout << "Img::download_img url = ";
    if( ! m_url_alt.empty() ) std::cout << m_url_alt << "  count = " << m_count_redirect << std::endl;
    else std::cout << m_url << std::endl;
    std::cout << "refurl = " << refurl <<  " wait = " << m_wait << " waitsec = " << waitsec << std::endl;
#endif

    if( is_loading() ) return;
    if( is_cached() ) return;
    if( ! CACHE::mkdir_imgroot() ) return;
    if( get_type() == T_LARGE ) return;
    if( is_wait() && waitsec ) return;

    // ロード待ち状態にセット
    if( set_wait( refurl, mosaic, waitsec ) ) return;

    // ロード待ち状態解除
    reset_wait();

#ifdef _DEBUG
    std::cout << "start...\n";
#endif

    // ダウンロード開始
    std::string path = get_cache_path();
    m_fout = fopen( to_locale_cstr( path ), "wb" );
    if( m_fout == NULL ){
        m_type = T_OPENFAILED;
        receive_finish();
        return;
    }

    clear();
    m_refurl = refurl;
    m_mosaic = mosaic;

    JDLIB::LOADERDATA data;
    data.init_for_data();

    if( ! m_url_alt.empty() ) data.url = m_url_alt;
    else data.url = m_url;

    // Urlreplaceによるリファラ指定
    std::string referer;
    if( CORE::get_urlreplace_manager()->referer( m_url, referer ) ) data.referer = referer;

    if( !start_load( data ) ) receive_finish();
    else CORE::core_set_command( "redraw", m_url );
}


//
// ロード停止
//
void Img::stop_load()
{
#ifdef _DEBUG
    std::cout << "Img::stop_load url = " << m_url << std::endl;
#endif

    SKELETON::Loadable::stop_load();

    if( m_wait ){

        set_code( HTTP_TIMEOUT );
        set_str_code( "stop loading" );

        reset_wait();
    }
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

    std::string save_to = CACHE::copy_file( parent, get_cache_path(), dir + name, CACHE::FILE_TYPE_ALL );

    if( ! save_to.empty() ){
        SESSION::set_dir_img_save( MISC::get_dir( save_to ) );
        return true;
    }

    return false;
}


//
// モザイクon/off
//
void Img::set_mosaic( const bool mosaic )
{
    if( ! is_cached() ) return;

    m_mosaic = mosaic;
    save_info();

    // 再描画
    CORE::core_set_command( "redraw_bbslist" );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


//
// サイズの大きいファイルを表示
//
void Img::show_large_img()
{
    if( m_type != T_LARGE ) return;

    const int size = 256;
    char data[ size ];
    CACHE::load_rawdata( get_cache_path(), data, size );
    m_type = DBIMG::get_image_type( ( const unsigned char* ) data );
    if( m_type != T_NOIMG ){

        set_code( HTTP_OK );
        set_str_code( "" );

        CORE::core_set_command( "redraw", m_url );
        CORE::core_set_command( "redraw_article" );
        CORE::core_set_command( "redraw_message" );
    }
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
bool Img::is_fake()
{
    if( ! is_cached() ) return false;
    if( m_imgctrl & CORE::IMGCTRL_GENUINE ) return false;

    bool ret = false;

    if( DBIMG::get_type_ext( m_url ) != m_type ) ret = true;
    else if( ! m_url_alt.empty() && DBIMG::get_type_ext( m_url_alt ) != m_type ) ret = true;

#ifdef _DEBUG
    std::cout << "Img::is_fake url = " << m_url << " ret = " << ret << std::endl;
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

    // Created は OK にしておく
    if( get_code() == HTTP_CREATED ) set_code( HTTP_OK );

    // 先頭のシグネチャを見て画像かどうかをチェック
    if( m_type == T_UNKNOWN && get_code() == HTTP_OK ){

#ifdef _DEBUG
        assert( size > 8 );
#endif        

        m_type = DBIMG::get_image_type( ( const unsigned char* ) data );

        if( m_type == T_NOIMG ){

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
            std::cout << "no image : size = " << size << std::endl;
            std::cout << data << std::endl;
#endif
        }
    }

    if( m_fout && m_type != T_NOIMG && m_type != T_NOT_FOUND ){

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
              << "strcode = " << get_str_code() << std::endl
              << "total byte = " << total_length() << std::endl
              << "contenttype = " << get_contenttype() << std::endl
              << "cookies : " << std::endl;

//    if( cookies().size() ){
//        std::list< std::string >::iterator it = cookies().begin();
//        for( ; it != cookies().end() ; ++it ) std::cout << *it << std::endl;
//    }
#endif

    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    // Created は OK にしておく
    if( get_code() == HTTP_CREATED ) set_code( HTTP_OK );

    // データが無い
    if( get_code() == HTTP_OK && ! current_length() ) m_type = T_NODATA;

    // リダイレクト
    if( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM ){
#ifdef _DEBUG
        std::cout << "301/302 redirect url = " << location() << std::endl;
#endif
        // アドレスに "404", ".htm" が含まれていたら not found と仮定
        std::string url_tmp = MISC::tolower_str( location() );
        if( url_tmp.find( "404" ) != std::string::npos && url_tmp.find( ".htm" ) != std::string::npos ) m_type = T_NOT_FOUND;

        else if( ! location().empty() && m_count_redirect < MAX_REDIRECT ){

#ifdef _DEBUG
            std::cout << "exec redirect\n";
#endif
            ++m_count_redirect;
            m_url_alt = location();
            download_img( m_refurl, m_mosaic, 0 );
            return;
        }
        else m_type = T_NODATA;
    }
    m_count_redirect = 0;

    if( get_code() != HTTP_OK ) set_current_length( 0 );

    // 画像サイズ取得
    if( get_code() == HTTP_OK && current_length() ){
        JDLIB::ImgLoader::get_loader( get_cache_path() )->get_size( m_width, m_height );
        if( ! m_width || ! m_height ) m_type = T_NOSIZE;
    }


    //////////////////////////////////////////////////
    // エラーメッセージのセット

    if( m_type == T_NOIMG ){
        set_code( HTTP_ERR );
        set_str_code( "画像ファイルではありません (" + get_contenttype() + ")" );
        set_current_length( 0 );
    }

    else if( m_type == T_NOT_FOUND ){
        set_code( HTTP_NOT_FOUND );
        set_str_code( "404 Not Found" );
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

    else if( m_type == T_NOSIZE ){

        set_code( HTTP_ERR );
        set_str_code( "画像サイズを取得出来ません" );
        set_current_length( 0 );
    }

    else if( get_code() == HTTP_OK && m_type == T_UNKNOWN ){
        set_code( HTTP_ERR );
        set_str_code( "未知の画像形式です" );
        set_current_length( 0 );
    }

    // 画像やファイルサイズが大きい
    else if( current_length() > (size_t) CONFIG::get_max_img_size() * 1024 * 1024
             || m_width * m_height > CONFIG::get_max_img_pixel() * 1000 * 1000
        ){

        m_type = T_LARGE;

        set_code( HTTP_ERR );
        std::stringstream ss;
        ss << "サイズが大きすぎます ( " << ( total_length() / 1024 / 1024 ) << " M, " << m_width << " x " << m_height
           << " )  ※コンテキストメニューから表示可能";
        set_str_code( ss.str() );
    }

    set_total_length( current_length() );

    // 読み込み失敗の場合はファイルを消しておく
    if( ! total_length() ){
        std::string path = get_cache_path();
        if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );
#ifdef _DEBUG
        std::cout << "unlink cache\n";
#endif
    }

#ifdef _DEBUG
    std::cout << "type = " << m_type << std::endl
              << "refurl = " << m_refurl << std::endl;
#endif

    // 画像が小さい場合はモザイクを解除
    if( m_width && m_height ){

        const int minsize = MAX( 1, CONFIG::get_mosaic_size() );
        if( m_width >= m_height && m_width < minsize ) m_mosaic = false;
        else if( m_width < m_height && m_height < minsize ) m_mosaic = false;
    }

    // 拡張子が偽装されている時はモザイク表示にする
    if( is_fake() ) m_mosaic = true;

    // 読み込み失敗の場合でもエラーメッセージを残すので info　は保存する
    save_info();

    CORE::core_set_command( "redraw", m_url );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


// 埋め込み画像のサイズを計算
void Img::set_embedded_size()
{
    if( ! m_width || ! m_height ) return;

    // 縮小比率を計算してサイズ取得
    double scale;
    double scale_w = ( double ) CONFIG::get_embimg_width() / m_width;
    double scale_h = ( double ) CONFIG::get_embimg_height() / m_height;
    scale = MIN( scale_w, scale_h );
    if( scale < 1.0 ){
        m_width_emb = (int)( m_width * scale );
        m_height_emb = (int)( m_height * scale );
    }
    else{
        m_width_emb = m_width;
        m_height_emb = m_height;
    }

#ifdef _DEBUG
    std::cout << "Img::set_embedded_size w = " << m_width_emb << " h = " << m_height_emb << std::endl;
#endif
}


// モザイク処理時に縮小するサイズを経産
void Img::set_mosaic_size()
{
    if( ! m_width || ! m_height ) return;

    // 一旦元画像の横幅を mossize ピクセルまで縮めてから描画サイズに戻す
    const int mossize = MAX( 1, CONFIG::get_mosaic_size() );
    int moswidth = m_width;
    int mosheight = m_height;

    if( moswidth >= mosheight ){

        const int dev = MAX( 1, moswidth / mossize );
        mosheight /= dev;
        moswidth = mossize;
    }
    else{

        const int dev = MAX( 1, mosheight / mossize );
        mosheight = mossize;
        moswidth /= dev;
    }

    m_width_mosaic = MAX( 1, moswidth );
    m_height_mosaic = MAX( 1, mosheight );
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

        path_info = std::string();

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
            unlink( to_locale_cstr( path_info ) );
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
