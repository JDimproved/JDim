// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareaicon.h"

#include "dbimg/img.h"
#include "dbimg/imginterface.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscgtk.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "colorid.h"

using namespace IMAGE;

ImageAreaIcon::ImageAreaIcon( const std::string& url )
    : ImageAreaBase( url, 0 ) // アイコンは常に Gdk::INTERP_NEAREST
    , m_shown( false )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::ImageAreaIcon url = " << url << std::endl;
#endif 

    set_width( ICON_SIZE );
    set_height( ICON_SIZE );
}



ImageAreaIcon::~ImageAreaIcon()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaIcon::~ImageAreaIcon url = " << get_url() << std::endl;
#endif 
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

    if( is_loading() )  return;

    // 既に画像が表示されている
    if( m_shown && get_img()->is_cached() ) return;

    if( m_pixbuf ){
        m_pixbuf.reset();
        m_pixbuf_loading.reset();
        m_pixbuf_err.reset();
    }
    if( m_pixbuf_icon ) m_pixbuf_icon.reset();

    m_shown = false;
    set_ready( false );

    // キャッシュされてない時は読み込みorエラーマークを表示
    if( ! get_img()->is_cached() ){
        show_indicator( ( get_img()->is_loading() || get_img()->is_wait() || get_img()->get_code() == HTTP_INIT ) );
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

        load_image();
    }
}


//
// 表示スレッド
//
void ImageAreaIcon::load_image_thread()
{
#ifdef _DEBUG
    std::cout << "ImageAreaIcon::load_image_thread url = " << get_url() << std::endl;
#endif

    bool pixbufonly = true;
    if( get_img()->get_type() == DBIMG::T_BMP ) pixbufonly = false; // BMP の場合 pixbufonly = true にすると真っ黒になる


    std::string errmsg;
    if( create_imgloader( pixbufonly, errmsg ) ){
        Glib::RefPtr< Gdk::Pixbuf > pixbuf = m_imgloader->get_pixbuf();
        if( pixbuf ) 
            m_pixbuf_icon = pixbuf->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST );
    }
    m_imgloader.reset();

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
    // callback_dispatch() -> set_image() の順に呼び出す
    dispatch();

#ifdef _DEBUG
    std::cout << "ImageAreaIcon::load_image_thread finished" << std::endl;
#endif    
}


//
// インジケータ幅、高さ
int ImageAreaIcon::width_indicator()
{
    return MAX( 1, get_width()/4 );
}

int ImageAreaIcon::height_indicator()
{
    return MAX( 1, get_height()/4 );
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
        m_pixbuf_err = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, width_indicator(), height_indicator() );
        m_pixbuf_loading = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, width_indicator(), height_indicator() );

        assert( m_pixbuf );
        assert( m_pixbuf_err );
        assert( m_pixbuf_loading );

        m_pixbuf->fill( 0xffffff00 );
        m_pixbuf_loading->fill( MISC::color_to_int( Gdk::Color( CONFIG::get_color( COLOR_IMG_LOADING ) ) ) );
        m_pixbuf_err->fill( MISC::color_to_int( Gdk::Color( CONFIG::get_color( COLOR_IMG_ERR ) ) ) );
    }

    // 読み込み中
    if( loading ) m_pixbuf_loading->copy_area( 0, 0, width_indicator(), height_indicator(), m_pixbuf, 4, 4 );

    // エラー
    else  m_pixbuf_err->copy_area( 0, 0, width_indicator(), height_indicator(), m_pixbuf, 4, 4 );

    m_imagetype = IMAGE_SHOW_INDICATOR;
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
    std::cout << "show icon" << std::endl;
#endif    

        set( m_pixbuf_icon );
    }

    else if( m_imagetype == IMAGE_SHOW_INDICATOR ){

#ifdef _DEBUG
    std::cout << "show indicator" << std::endl;
#endif    

        set( m_pixbuf );
    }

    set_ready( true );
}
