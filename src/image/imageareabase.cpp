// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareabase.h"

#include "jdlib/jdmutex.h"
#include "jdlib/miscmsg.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "config/globalconf.h"

//
// スレッドのランチャ
//
static JDLIB::StaticMutex imgarea_launcher_mutex = JDLIB_STATIC_MUTEX_INIT;

void* imgarea_launcher( void* dat )
{
    JDLIB::LockGuard lock( imgarea_launcher_mutex );

#ifdef _DEBUG
    std::cout << "start imgarea_launcher" << std::endl;
#endif

    IMAGE::ImageAreaBase* area = ( IMAGE::ImageAreaBase* )( dat );
    area->load_image_thread();

#ifdef _DEBUG
    std::cout << "end" << std::endl;
#endif

    return 0;
}


//////////////////////////////////


using namespace IMAGE;


// interptype :
// 0 -> INTERP_NEAREST
// 1 -> INTERP_BILINEAR
// 3 -> INTERP_HYPER
ImageAreaBase::ImageAreaBase( const std::string& url, const int interptype )
    : m_url( url ),
      m_img ( DBIMG::get_img( m_url ) ),
      m_ready( false ),
      m_width( 0 ),
      m_height( 0 )
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::ImageAreaBase url = " << m_url << " type = " << interptype << std::endl;
#endif 

    assert( m_img );

    m_interptype = Gdk::INTERP_NEAREST;
    if( interptype == 1 ) m_interptype = Gdk::INTERP_BILINEAR;
    else if( interptype >= 2 ) m_interptype = Gdk::INTERP_HYPER;

    set_alignment( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER );
}



ImageAreaBase::~ImageAreaBase()
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::~ImageAreaBase url = " << m_url << std::endl;
#endif 

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop();
    wait();
}


void ImageAreaBase::stop()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaBase::stop" << std::endl;
#endif 
    if( m_imgloader )
        m_imgloader->request_stop();
}


void ImageAreaBase::wait()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaBase::wait" << std::endl;
#endif 

    m_thread.join();

#ifdef _DEBUG    
    std::cout << "ImageAreaBase::wait ok" << std::endl;
#endif 
}


//
// 幅、高さセット
//
// 0にするとGdk::Pixbuf::create()で落ちるので注意
//
void ImageAreaBase::set_width( const int width )
{
    m_width = MAX( 1, width );
}

void ImageAreaBase::set_height( const int height )
{
    m_height = MAX( 1, height );
}



//
// 画像読み込み
//
void ImageAreaBase::load_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::load_image" << std::endl;
#endif

    // 大きい画像を表示しようとするとJDが固まるときがあるのでスレッドを使用する
    // ランチャ経由で load_image_thread() を動かす
    if( ! m_thread.create( imgarea_launcher, ( void* ) this, JDLIB::NODETACH ) ) {
        MISC::ERRMSG( "ImageAreaBase::load_image : could not start thread" );
    }
}


//
// 画像読み込みスレッド
//
void ImageAreaBase::load_image_thread()
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::load_image_thread url = " << get_url() << std::endl;
#endif

    const int w_org = get_img()->get_width();
    const int h_org = get_img()->get_height();

    // アニメーションoff
    bool pixbufonly = ( w_org != get_width() || h_org != get_height() );

    // pixbufonly = trueにすると プログレッシブJPGではモザイクがかかったようになる
    const int minsize = w_org/4;
    if( pixbufonly && get_width() > minsize  && m_img->get_type() == DBIMG::T_JPG ) pixbufonly = false;

    // BMP の場合 pixbufonly = true にすると真っ黒になる
    if( pixbufonly && m_img->get_type() == DBIMG::T_BMP ) pixbufonly = false;

    std::string errmsg;
    if( ! create_imgloader( pixbufonly, errmsg ) ){
        set_errmsg( errmsg );
        MISC::ERRMSG( get_errmsg() );
    }

    // 表示
    // スレッドの中でset_image()すると固まるときがあるのでディスパッチャ経由で
    // callback_dispatch() -> set_image() の順に呼び出す
    dispatch();

#ifdef _DEBUG
    std::cout << "ImageAreaBase::load_image_thread finished" << std::endl;
#endif    
}


const bool ImageAreaBase::create_imgloader( const bool pixbufonly, std::string& errmsg )
{
    m_imgloader = JDLIB::ImgLoader::get_loader( m_img->get_cache_path() );
    bool ret = m_imgloader->load( pixbufonly );
    if( ! ret ) {
        errmsg = m_imgloader->get_errmsg();
    }
    return ret;
}


//
// ディスパッチャのコールバック関数
//
void ImageAreaBase::callback_dispatch()
{
    set_image();
    wait();
}


//
// Gtk::Image::set()を使用して画像表示
//
void ImageAreaBase::set_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::set_image" << std::endl;
#endif

    clear();

    const int w_org = get_img()->get_width();
    const int h_org = get_img()->get_height();

    Glib::RefPtr< Gdk::Pixbuf > pixbuf = m_imgloader->get_pixbuf();
    if( pixbuf ){
        if( m_img->get_mosaic() ){
            // モザイク
            set_mosaic( pixbuf );
        }else if( w_org != get_width() || h_org != get_height() ){
            // 拡大縮小
            set( pixbuf->scale_simple( get_width(), get_height(), m_interptype ) );
        }else{
            // 通常
            set( m_imgloader->get_animation() );
        }
    }
    m_imgloader.reset();

    set_ready( true );
}


//
// モザイク表示
//
void ImageAreaBase::set_mosaic( Glib::RefPtr< Gdk::Pixbuf > pixbuf )
{
    const int moswidth = get_img()->get_width_mosaic();
    const int mosheight = get_img()->get_height_mosaic();

    if( moswidth && mosheight ){

        Glib::RefPtr< Gdk::Pixbuf > pixbuf2;
        pixbuf2 = pixbuf->scale_simple( moswidth, mosheight, Gdk::INTERP_NEAREST );
        set( pixbuf2->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST ) );
    }
}
