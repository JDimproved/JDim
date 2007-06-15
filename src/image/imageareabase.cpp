// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareabase.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscgtk.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "httpcode.h"

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
    std::cout << "ImageAreaBase::~ImageArea url = " << m_url << std::endl;
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
// 画像表示
//
void ImageAreaBase::set_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaBase::set_image\n";
#endif

    int w_org = get_img()->get_width();
    int h_org = get_img()->get_height();

    // 画像ロード
    bool stop = false;
    std::string errmsg;
    bool pixbufonly = ( w_org != get_width() || h_org != get_height() );

    Glib::RefPtr< Gdk::PixbufLoader > loader = MISC::get_ImageLoder( m_img->get_cache_path(), stop, pixbufonly, errmsg );
    if( loader ){

        if( m_img->get_mosaic() ) set_mosaic( loader->get_pixbuf() );
        else{
            if( pixbufonly ) set( loader->get_pixbuf()->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST ) );
            else set( loader->get_animation() );
        }
    }
    else{
        set_errmsg( errmsg );
        MISC::ERRMSG( get_errmsg() );
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
    pixbuf2 = pixbuf->scale_simple( get_width() / size_mosaic, get_height() / size_mosaic, Gdk::INTERP_NEAREST );
    set( pixbuf2->scale_simple( get_width(), get_height(), Gdk::INTERP_NEAREST ) );
}
