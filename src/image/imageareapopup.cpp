// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageareapopup.h"

#include "dbimg/img.h"

#include "config/globalconf.h"

#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


using namespace IMAGE;

ImageAreaPopup::ImageAreaPopup( const std::string& url )
    : ImageAreaBase( url, CONFIG::get_imgpopup_interp() )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaPopup::ImageAreaPopup url = " << url << std::endl;
#endif 
}


ImageAreaPopup::~ImageAreaPopup()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaPopup::~ImageAreaPopup url = " << get_url() << std::endl;
#endif 
}


//
// 表示
//
void ImageAreaPopup::show_image()
{
    if( is_loading() ) return;
    if( ! get_img()->is_cached() ) return;

#ifdef _DEBUG
    std::cout << "ImageAreaPopup::show_image url = " << get_url() << std::endl;
#endif    

    set_errmsg( std::string() );
    const int width_max = CONFIG::get_imgpopup_width();
    const int height_max = CONFIG::get_imgpopup_height();

    // 縮小比率を計算
    const int w_org = get_img()->get_width();
    const int h_org = get_img()->get_height();
    const double scale_w = ( double ) width_max / w_org;
    const double scale_h = ( double ) height_max / h_org;
    const double scale = MIN( scale_w, scale_h );

    if( scale < 1 ){
        set_width( (int)( w_org * scale ) );
        set_height( (int)( h_org * scale ) );
    }
    else{
        set_width( w_org );
        set_height( h_org );
    }

    load_image();
}
