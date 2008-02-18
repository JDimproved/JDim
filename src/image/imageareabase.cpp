// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareabase.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscgtk.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"


//
// スレッドのランチャ
//
Glib::StaticMutex imgarea_launcher_mutex = GLIBMM_STATIC_MUTEX_INIT;

void* imgarea_launcher( void* dat )
{
    Glib::Mutex::Lock lock( imgarea_launcher_mutex);

#ifdef _DEBUG
    std::cout << "start imgarea_launcher\n";
#endif

    IMAGE::ImageAreaBase* area = ( IMAGE::ImageAreaBase* )( dat );
    area->load_image_thread();

#ifdef _DEBUG
    std::cout << "end\n";
#endif

    return 0;
}


//////////////////////////////////


using namespace IMAGE;


ImageAreaBase::ImageAreaBase( const std::string& url )
    : m_url( url ),
      m_img ( DBIMG::get_img( m_url ) ),
      m_ready( false ),
      m_width( 0 ),
      m_height( 0 )
{
    assert( m_img );

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
    std::cout << "ImageAreaBase::stop\n";
#endif 

    m_stop = true;
}


void ImageAreaBase::wait()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaBase::wait\n";
#endif 

    m_thread.join();

#ifdef _DEBUG    
    std::cout << "ImageAreaBase::wait ok\n";
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
    std::cout << "ImageAreaBase::load_image\n";
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

    int w_org = get_img()->get_width();
    int h_org = get_img()->get_height();

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
    std::cout << "ImageAreaBase::load_image_thread finished\n";
#endif    
}


bool ImageAreaBase::create_imgloader( bool pixbufonly, std::string& errmsg )
{
    m_stop = false;
    m_imgloader = MISC::get_ImageLoder( m_img->get_cache_path(), m_stop, pixbufonly, errmsg );
    return ( m_imgloader );
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
    std::cout << "ImageAreaBase::set_image\n";
#endif

    clear();

    int w_org = get_img()->get_width();
    int h_org = get_img()->get_height();

    if( m_imgloader && m_imgloader->get_pixbuf() ){

        // モザイク
        if( m_img->get_mosaic() ) set_mosaic( m_imgloader->get_pixbuf() );
        else{

            // 拡大縮小
            if( w_org != get_width() || h_org != get_height() )
                set( m_imgloader->get_pixbuf()->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST ) );

            // 通常
            else set( m_imgloader->get_animation() );
        }
    }

    set_ready( true );
}


//
// モザイク表示
//
void ImageAreaBase::set_mosaic( Glib::RefPtr< Gdk::Pixbuf > pixbuf )
{
    int size_mosaic = 20;  // モザイク画像は 1/size_mosaic にしてもとのサイズに直す
    if( get_width() / size_mosaic < 16 ) size_mosaic = MAX( 1, get_width() / 16 );

    Glib::RefPtr< Gdk::Pixbuf > pixbuf2;
    pixbuf2 = pixbuf->scale_simple( MAX( 1, get_width() / size_mosaic ),
                                    MAX( 1, get_height() / size_mosaic ), Gdk::INTERP_NEAREST );
    set( pixbuf2->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST ) );
}
