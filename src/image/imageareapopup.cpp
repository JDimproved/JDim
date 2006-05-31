// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "imageareapopup.h"

#include "dbimg/img.h"

#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

#include "cache.h"

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


using namespace IMAGE;

ImageAreaPopup::ImageAreaPopup( const std::string& url )
    : ImageAreaBase( url )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaPopup::ImageAreaPopup url = " << url << std::endl;
#endif 

    show_image();
}



//
// 表示
//
void ImageAreaPopup::show_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaPopup::show_image url = " << get_url() << std::endl;
#endif    

    set_errmsg( std::string() );
    bool mosaic = get_img()->get_mosaic();
    int width_max = CONFIG::get_imgpopup_width();
    int height_max = CONFIG::get_imgpopup_height();

    clear();

    try{

        // 画像ロード
        std::string path_cache = CACHE::path_img( get_url() );
        Glib::RefPtr< Gdk::Pixbuf > pixbuf;
        pixbuf = Gdk::Pixbuf::create_from_file( path_cache );
        set_width_org( pixbuf->get_width() );
        set_height_org( pixbuf->get_height() );
        set_width( get_width_org() );
        set_height( get_height_org() );

        // スケール調整
        bool do_scale = false;
        double scale;

        double scale_w = ( double ) width_max / get_width();
        double scale_h = ( double ) height_max / get_height();
        scale = MIN( scale_w, scale_h );
        if( scale < 1 ) do_scale = true;

        set_image( pixbuf, mosaic, do_scale, scale );
    }
    catch( Glib::Error& err )
    {
        set_errmsg( err.what() );
        MISC::ERRMSG( get_errmsg() );
        set_width( width_max );
        set_height( width_max );
    }

    set_ready( true );
}
