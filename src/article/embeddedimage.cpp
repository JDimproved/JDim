// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "embeddedimage.h"
#include "articleadmin.h"

#include "jdlib/miscgtk.h"
#include "jdlib/miscmsg.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "message/messageadmin.h"

//
// スレッドのランチャ
//
Glib::StaticMutex eimg_launcher_mutex = GLIBMM_STATIC_MUTEX_INIT;
int redraw_counter = 0; // 0 になったとき再描画する

void* eimg_launcher( void* dat )
{
    ++redraw_counter;
    Glib::Mutex::Lock lock( eimg_launcher_mutex );

#ifdef _DEBUG
    std::cout << "start eimg_launcher\n";
#endif

    ARTICLE::EmbeddedImage* eimg = (ARTICLE::EmbeddedImage* )( dat );
    eimg->resize_thread();

    // 再描画
    --redraw_counter;
    if( ! redraw_counter ){
        ARTICLE::get_admin()->set_command( "redraw_current_view" );
        MESSAGE::get_admin()->set_command( "redraw_current_view" );
    }

#ifdef _DEBUG
    std::cout << "end\n";
#endif

    return 0;
}


/////////////////////////////////


using namespace ARTICLE;


EmbeddedImage::EmbeddedImage( const std::string& url )
    : m_url( url ),
      m_img ( DBIMG::get_img( m_url ) ),
      m_thread( 0 ),
      m_width( 0 ),
      m_height( 0 )
{}

EmbeddedImage::~EmbeddedImage()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::~EmbeddedImage url = " << m_url << std::endl;
#endif

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop();
    wait();
}


void EmbeddedImage::stop()
{
#ifdef _DEBUG    
    std::cout << "EmbeddedImage::stop\n";
#endif 

    m_stop = true;
}



void EmbeddedImage::wait()
{
#ifdef _DEBUG    
    std::cout << "EmbeddedImage::wait\n";
#endif 

    MISC::thread_join( m_thread );
    m_thread = 0;
}


void EmbeddedImage::show()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::show url = " << m_url << std::endl;
#endif

    if( m_thread ) return;

    const int max_width = 100;
    const int max_height = 100;
    m_width = 0;
    m_height = 0;

    // 画像読み込み
    if( ! m_img->is_cached() ) return;

    // 縮小比率を計算してサイズ取得
    double scale;
    double scale_w = ( double ) max_width / m_img->get_width();
    double scale_h = ( double ) max_height / m_img->get_height();
    scale = MIN( scale_w, scale_h );

    m_width = (int)( m_img->get_width() * scale );
    m_height = (int)( m_img->get_height() * scale );

    if( ! m_width || ! m_height ) return;

    // スレッド起動して縮小
    m_stop = false;
    if( ! MISC::thread_create( m_thread, eimg_launcher, ( void* )this, MISC::NODETACH ) ){
        MISC::ERRMSG( "EmbeddedImage::show : could not start thread" );
    }
}


// リサイズのスレッド
void EmbeddedImage::resize_thread()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::resize_thread url = " << m_url << std::endl;
#endif

    std::string errmsg;
    bool pixbufonly = true;
    if( m_img->get_type() == DBIMG::T_BMP ) pixbufonly = false; // BMP の場合 pixbufonly = true にすると真っ黒になる
    Glib::RefPtr< Gdk::PixbufLoader > loader = MISC::get_ImageLoder( m_img->get_cache_path(), m_stop, pixbufonly, errmsg );
    if( loader && loader->get_pixbuf() ) m_pixbuf = loader->get_pixbuf()->scale_simple( m_width, m_height, Gdk::INTERP_NEAREST );

    // メインスレッドにリサイズが終わったことを知らせて
    // メインスレッドがpthread_join()を呼び出す
    // そうしないとメモリを食い潰す
    dispatch();
}


//
// ディスパッチャのコールバック関数
//
void EmbeddedImage::callback_dispatch()
{
    wait();
}
