// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imagearea.h"

#include "dbimg/img.h"

#include "config/globalconf.h"

#include "session.h"

#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif

using namespace IMAGE;


ImageAreaMain::ImageAreaMain( const std::string& url )
    : ImageAreaBase( url, CONFIG::get_imgmain_interp() )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaMain::ImageAreaMain url = " << url << std::endl;
#endif 
}


ImageAreaMain::~ImageAreaMain()
{
#ifdef _DEBUG    
    std::cout << "ImageAreaMain::~ImageAreaMain url = " << get_url() << std::endl;
#endif 
}


//
// 表示
//
void ImageAreaMain::show_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaMain::show_image url = " << get_url() << std::endl;
#endif    

    if( is_loading() )  return;

    set_errmsg( std::string() );
    int width_max = 0;
    int height_max = 0;

    if( get_parent() && get_parent()->get_parent() ){     // 親(EventBox)の親(ScrolledWindow)がいるときはそのサイズ
        const int mrg = 32;
        width_max = get_parent()->get_parent()->get_width() - mrg;
        height_max = get_parent()->get_parent()->get_height() - mrg;
    }
    else{
        width_max = Gtk::Image::get_width();
        height_max = Gtk::Image::get_height();
    }

    // まだrealizeしてなくてウィンドウサイズが取得できていないのでImageViewMain::clock_in()経由で後でもう一度呼ぶ
    if( ! is_ready()  && ( width_max <= 1 || height_max <= 1 ) ) return;

    bool zoom_to_fit = get_img()->is_zoom_to_fit();
    int size = get_img()->get_size();

    // スケール調整
    double scale = 1;
    int w_org = get_img()->get_width();
    int h_org = get_img()->get_height();
    set_width( w_org );
    set_height( h_org );

    // 画面サイズに合わせる
    if( zoom_to_fit && w_org && h_org ){
        double scale_w = ( double ) width_max / w_org;
        double scale_h = ( double ) height_max / h_org;

        if( SESSION::get_img_fit_mode() == SESSION::IMG_FIT_NORMAL ) scale = MIN( scale_w, scale_h );
        else scale = scale_w;

        if( scale < 1 ){
            set_width( (int)( w_org * scale ) );
            set_height( (int)( h_org * scale ) );
        }
    }

    // サイズ変更
    else if( size != 100 ){
        scale = size/100.;
        set_width( (int)( w_org * scale ) );
        set_height( (int)( h_org * scale ) );
    }

    //データベースのサイズ情報更新
    get_img()->set_size( get_width() * 100 / get_img()->get_width() );

    load_image();
}
