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
    : ImageAreaBase( url )
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
#ifdef _DEBUG
    std::cout << "ImageAreaPopup::show_image url = " << get_url() << std::endl;
#endif    

    if( is_loading() ) return;
    if( is_ready() ) return;

    if( ! get_img()->is_cached() ) return;

    set_errmsg( std::string() );
    int width_max = CONFIG::get_imgpopup_width();
    int height_max = CONFIG::get_imgpopup_height();

    clear();

    // 縮小比率を計算
    double scale;
    int w_org = get_img()->get_width();
    int h_org = get_img()->get_height();
    double scale_w = ( double ) width_max / w_org;
    double scale_h = ( double ) height_max / h_org;
    scale = MIN( scale_w, scale_h );

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
