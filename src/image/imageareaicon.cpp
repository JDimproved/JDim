// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "imageareaicon.h"

#include "dbimg/img.h"

#include "jdlib/miscmsg.h"

#include "cache.h"
#include "global.h"
#include "httpcode.h"


typedef void* ( *FUNC )( void * );

using namespace IMAGE;

ImageAreaIcon::ImageAreaIcon( const std::string& url )
    : ImageAreaBase( url )
    , m_thread_running( false )
    , m_shown( false )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::ImageAreaIcon url = " << url << std::endl;
#endif 

    m_disp.connect( sigc::mem_fun( *this, &ImageAreaIcon::slot_set_image ) );

    set_width( ICON_SIZE );
    set_height( ICON_SIZE );

    show_image_thread();
}



ImageAreaIcon::~ImageAreaIcon()
{}


//
// 表示
//
// メインスレッドで大きい画像を開くと反応が無くなるので別の
// スレッドを起動して開く。スレッドはデタッチしておく
//
void ImageAreaIcon::show_image()
{
    if( m_thread_running ) return; // スレッド動作中

    // 既に画像が表示されている
    if( m_shown && is_cached() ) return;

    m_shown = false;

    int status;
    if( ( status = pthread_create( &m_thread, NULL,  ( FUNC ) launcher, ( void * ) this ) )){
        MISC::ERRMSG( "ImageAreaIcon::show_image : could not start thread" );
    }
    else{
        m_thread_running = true;
        pthread_detach( m_thread );
    }
}


//
// スレッドのランチャ (static)
//
void* ImageAreaIcon::launcher( void* dat )
{
    ImageAreaIcon* tt = ( ImageAreaIcon * ) dat;
    tt->show_image_thread();
    return 0;
}



//
// 表示スレッド
//
void ImageAreaIcon::show_image_thread()
{
#ifdef _DEBUG
    std::cout << "ImageAreaIcon::show_image_thread url = " << get_url() << std::endl;
#endif

    set_ready( false );

    // キャッシュされてない時は読み込みorエラーマークを表示
    if( ! is_cached() ){

        // インジゲータ画像作成
        if( ! m_pixbuf ){

            m_pixbuf = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width(), get_height() );
            m_pixbuf_err = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width()/4, get_height()/4 );
            m_pixbuf_loading = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, get_width()/4, get_height()/4 );

            assert( m_pixbuf );
            assert( m_pixbuf_err );
            assert( m_pixbuf_loading );

            m_pixbuf->fill( 0xffffff00 );
            m_pixbuf_loading->fill( 0xffbf0000 );
            m_pixbuf_err->fill( 0xff000000 );
        }

        // 読み込み中
        if( get_img()->is_loading()
            || get_img()->get_code() == HTTP_INIT ) m_pixbuf_loading->copy_area( 0, 0, get_width()/4, get_height()/4, m_pixbuf, 4, 4 );

        // エラー
        else  m_pixbuf_err->copy_area( 0, 0, get_width()/4, get_height()/4, m_pixbuf, 4, 4 );

        // 表示
        // ImageAreaIcon::slot_set_image()を呼び出す
        m_disp.emit();
    }

    // 画像を読み込んで縮小表示
    else{

        try{

            // 画像ロード
            std::string path_cache = CACHE::path_img( get_url() );
            Glib::RefPtr< Gdk::Pixbuf > pixbuf;
            pixbuf = Gdk::Pixbuf::create_from_file( path_cache );
            set_width_org( pixbuf->get_width() );
            set_height_org( pixbuf->get_height() );

            // 縮小比率を計算して縮小
            
            double scale;
            double scale_w = ( double ) ICON_SIZE / get_width_org();
            double scale_h = ( double ) ICON_SIZE / get_height_org();
            scale = MIN( scale_w, scale_h );
            set_width( (int)( get_width_org() * scale ) );
            set_height( (int)( get_height_org() * scale ) );
            m_pixbuf_icon = pixbuf->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST );

            // 表示
            // mageAreaIcon::slot_set_image()を呼び出す
            m_disp.emit();

            m_shown = true;
        }
        catch( Glib::Error& err )
        {
            set_errmsg( err.what() );
            MISC::ERRMSG( get_errmsg() );
        }
    }

#ifdef _DEBUG
    std::cout << "ImageAreaIcon::show_image_thread finished\n";
#endif    

    m_thread_running = false;
    set_ready( true );
}


//
// 表示
//
// スレッドの中でset()すると固まるときがあるのでディスパッチャで
// メインスレッドに戻してからセットする
//
void ImageAreaIcon::slot_set_image()
{
    clear();
    if( m_pixbuf_icon ) set( m_pixbuf_icon );
    else if( m_pixbuf ) set( m_pixbuf );
}
