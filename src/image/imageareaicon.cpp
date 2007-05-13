// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareaicon.h"

#include "dbimg/img.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscthread.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "colorid.h"


//
// スレッドのランチャ
//
// icon_launcherの引数で直接 ImageAreaIcon のポインタを渡すと
// スレッドが起動する前に ImageAreaIcon が delete されると落ちるのでリストを使う

Glib::StaticMutex icon_launcher_mutex = GLIBMM_STATIC_MUTEX_INIT;

void* icon_launcher( void* dat )
{
    Glib::Mutex::Lock lock( icon_launcher_mutex);

#ifdef _DEBUG
    std::cout << "start icon_launcher\n";
#endif

    IMAGE::ImageAreaIcon* icon = ( IMAGE::ImageAreaIcon* )( dat );
    icon->show_image_thread();

#ifdef _DEBUG
    std::cout << "end\n";
#endif

    return 0;
}



//////////////////////////////////

using namespace IMAGE;

ImageAreaIcon::ImageAreaIcon( const std::string& url )
    : ImageAreaBase( url )
    , m_thread( 0 )
    , m_shown( false )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::ImageAreaIcon url = " << url << std::endl;
#endif 

    set_width( ICON_SIZE );
    set_height( ICON_SIZE );

    show_image();
}



ImageAreaIcon::~ImageAreaIcon()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::~ImageAreaIcon url = " << get_url() << std::endl;
#endif 

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop();
    wait();
}


void ImageAreaIcon::stop()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::stop\n";
#endif 

    m_stop = true;
}


void ImageAreaIcon::wait()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::wait\n";
#endif 

    if( m_thread ) pthread_join( m_thread, NULL );
    m_thread = 0;
}


//
// 表示
//
// メインスレッドで大きい画像を開くと反応が無くなるので別の
// スレッドを起動して開く。
//
void ImageAreaIcon::show_image()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::show_image url = " << get_url() << std::endl;
#endif 

    if( m_thread )  return;

    // 既に画像が表示されている
    if( m_shown && get_img()->is_cached() ) return;

    if( m_pixbuf ){
        m_pixbuf.clear();
        m_pixbuf_loading.clear();
        m_pixbuf_err.clear();
    }
    if( m_pixbuf_icon ) m_pixbuf_icon.clear();

    m_shown = false;
    set_ready( false );

    // キャッシュされてない時は読み込みorエラーマークを表示
    if( ! get_img()->is_cached() ){
        show_indicator( ( get_img()->is_loading() || get_img()->get_code() == HTTP_INIT ) );
        set_image();
    }

    // スレッドを起動してバックグラウンドでアイコン表示
    else{

        // 縮小比率を計算
        double scale;
        int w_org = get_img()->get_width();
        int h_org = get_img()->get_height();
        double scale_w = ( double ) ICON_SIZE / w_org;
        double scale_h = ( double ) ICON_SIZE / h_org;
        scale = MIN( scale_w, scale_h );
        set_width( (int)( w_org * scale ) );
        set_height( (int)( h_org * scale ) );

        const int stacksize = 64;        
        int status;
        m_stop = false;
        if( ( status = MISC::thread_create( &m_thread, icon_launcher, ( void* ) this, stacksize ) ) ){
            MISC::ERRMSG( std::string( "ImageAreaIcon::show_image : could not start thread " ) + strerror( status ) );
        }
    }
}


//
// 表示スレッド
//
void ImageAreaIcon::show_image_thread()
{
#ifdef _DEBUG
    std::cout << "ImageAreaIcon::show_image_thread url = " << get_url() << std::endl;
#endif

    std::string errmsg;
    Glib::RefPtr< Gdk::PixbufLoader > loader = MISC::get_ImageLoder( get_img()->get_cache_path(), get_width(), get_height(), m_stop, errmsg );
    if( loader ) m_pixbuf_icon = loader->get_pixbuf();

    if( m_pixbuf_icon ){
        m_imagetype = IMAGE_SHOW_ICON;
        m_shown = true;
    }
    else{

        set_errmsg( errmsg );
        MISC::ERRMSG( get_errmsg() );

        show_indicator( false );
    }

    // 表示
    // スレッドの中でset_image()すると固まるときがあるのでディスパッチャ経由で
    // allback_dispatch() -> set_image() と呼び出す
    dispatch();

#ifdef _DEBUG
    std::cout << "ImageAreaIcon::show_image_thread finished\n";
#endif    
}


//
// インジゲータ画像表示
// 
void ImageAreaIcon::show_indicator( bool loading )
{
#ifdef _DEBUG
        std::cout << "ImageAreaIcon::show_indicator load = "  << loading << std::endl;
#endif        

    if( ! m_pixbuf ){

        m_pixbuf = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width(), get_height() );
        m_pixbuf_err = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width()/4, get_height()/4 );
        m_pixbuf_loading = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width()/4, get_height()/4 );

        assert( m_pixbuf );
        assert( m_pixbuf_err );
        assert( m_pixbuf_loading );

        m_pixbuf->fill( 0xffffff00 );
        m_pixbuf_loading->fill( MISC::color_to_int( Gdk::Color( CONFIG::get_color( COLOR_IMG_LOADING ) ) ) );
        m_pixbuf_err->fill( MISC::color_to_int( Gdk::Color( CONFIG::get_color( COLOR_IMG_ERR ) ) ) );
    }

    // 読み込み中
    if( loading ) m_pixbuf_loading->copy_area( 0, 0, get_width()/4, get_height()/4, m_pixbuf, 4, 4 );

    // エラー
    else  m_pixbuf_err->copy_area( 0, 0, get_width()/4, get_height()/4, m_pixbuf, 4, 4 );

    m_imagetype = IMAGE_SHOW_INDICATOR;
}



//
// ディスパッチャのコールバック関数
//
void ImageAreaIcon::callback_dispatch()
{
    set_image();
    wait();
}



//
// 表示
//
void ImageAreaIcon::set_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaIcon::set_image type = " << m_imagetype << std::endl;
#endif    

    clear();
    if( m_imagetype == IMAGE_SHOW_ICON ){

#ifdef _DEBUG
    std::cout << "show icon\n";
#endif    

        set( m_pixbuf_icon );
    }

    else if( m_imagetype == IMAGE_SHOW_INDICATOR ){

#ifdef _DEBUG
    std::cout << "show indicator\n";
#endif    

        set( m_pixbuf );
    }

    set_ready( true );
}
