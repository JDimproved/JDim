// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareabase.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "httpcode.h"
#include "cache.h"

using namespace IMAGE;


ImageAreaBase::ImageAreaBase( const std::string& url )
    : m_url( url ),
      m_img ( DBIMG::get_img( m_url ) ),
      m_ready( false ),
      m_width_org( 0 ),
      m_height_org( 0 ),      
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
// 画像をセット
//
void ImageAreaBase::set_image( Glib::RefPtr< Gdk::Pixbuf >& pixbuf, bool mosaic, bool do_scale, double scale )
{
    if( do_scale ){
        m_width = ( int ) ( m_width * scale );
        m_height = ( int ) ( m_height * scale );
    }            
        
    // モザイク表示
    if( mosaic ){
        int size_mosaic = 20;  // モザイク画像は 1/size_mosaic にしてもとのサイズに直す
        if( m_width_org / size_mosaic < 16 ) size_mosaic = MAX( 1, m_width_org / 16 );

        Glib::RefPtr< Gdk::Pixbuf > pixbuf2;
        pixbuf2 = pixbuf->scale_simple( m_width_org / size_mosaic, m_height_org / size_mosaic, Gdk::INTERP_NEAREST );
        set( pixbuf2->scale_simple( m_width, m_height, Gdk::INTERP_NEAREST ) );
    }

    // 通常表示
    else{

        if( do_scale ) set( pixbuf->scale_simple( m_width, m_height, Gdk::INTERP_NEAREST ) );
        else set( CACHE::path_img( m_url ) );
    }
}
