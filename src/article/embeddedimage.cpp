// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "embeddedimage.h"
#include "articleadmin.h"

#include "jdlib/imgloader.h"
#include "jdlib/miscmsg.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "config/globalconf.h"

#include "message/messageadmin.h"

#include <mutex>
#include <system_error>


//
// スレッドのランチャ
//
static std::mutex eimg_launcher_mutex;
int redraw_counter = 0; // 0 になったとき再描画する

static void eimg_launcher( ARTICLE::EmbeddedImage* eimg )
{
    ++redraw_counter;

    // 遅いCPUの場合は同時に画像をリサイズしようとすると固まった様になるので
    // mutexをかけて同時にリサイズしないようにする
    std::lock_guard< std::mutex > lock( eimg_launcher_mutex );

#ifdef _DEBUG
    std::cout << "start eimg_launcher" << std::endl;
#endif

    eimg->resize_thread();

    // 再描画
    --redraw_counter;
    if( ! redraw_counter ){
        ARTICLE::get_admin()->set_command( "redraw_current_view" );
        MESSAGE::get_admin()->set_command( "redraw_current_view" );
    }

#ifdef _DEBUG
    std::cout << "end" << std::endl;
#endif
}


/////////////////////////////////


using namespace ARTICLE;


EmbeddedImage::EmbeddedImage( const std::string& url )
    : m_url( url )
    , m_img{ DBIMG::get_img( m_url ) }
{
    assert( m_img );
}


EmbeddedImage::~EmbeddedImage()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::~EmbeddedImage url = " << m_url << std::endl;
#endif

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop();
    // m_thread.joinable() == true のときスレッドを破棄すると強制終了するため待機処理を入れる
    wait();
}


void EmbeddedImage::stop()
{
#ifdef _DEBUG    
    std::cout << "EmbeddedImage::stop" << std::endl;
#endif 
    if( m_imgloader )
        m_imgloader->request_stop();
}



void EmbeddedImage::wait()
{
#ifdef _DEBUG    
    std::cout << "EmbeddedImage::wait" << std::endl;
#endif 
    if( m_thread.joinable() ) m_thread.join();
}


void EmbeddedImage::show()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::show url = " << m_url << std::endl;
#endif

    if( m_thread.joinable() ) return;

    // 画像読み込み
    if( ! m_img->is_cached() ) return;

    const int width = m_img->get_width_emb();
    const int height = m_img->get_height_emb();

    if( ! width || ! height ) return;

    // スレッド起動して縮小
    try {
        m_thread = std::thread( eimg_launcher, this );
    }
    catch( std::system_error& ) {
        MISC::ERRMSG( "EmbeddedImage::show : could not start thread" );
    }
}


// リサイズのスレッド
void EmbeddedImage::resize_thread()
{
#ifdef _DEBUG
    std::cout << "EmbeddedImage::resize_thread url = " << m_url << std::endl;
#endif

    const int width = m_img->get_width_emb();
    const int height = m_img->get_height_emb();

    bool pixbufonly = true;

    if( m_img->get_type() == DBIMG::T_BMP ) pixbufonly = false; // BMP の場合 pixbufonly = true にすると真っ黒になる
    
    m_imgloader = JDLIB::ImgLoader::get_loader( m_img->get_cache_path() );
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = m_imgloader->get_pixbuf( pixbufonly );
    if( pixbuf ){
        Gdk::InterpType interptype = Gdk::INTERP_NEAREST;
        if( CONFIG::get_imgemb_interp() == 1 ) interptype = Gdk::INTERP_BILINEAR;
        else if( CONFIG::get_imgemb_interp() >= 2 ) interptype = Gdk::INTERP_HYPER;

        m_pixbuf = pixbuf->scale_simple( width, height, interptype );
    }
    m_imgloader.reset();

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
