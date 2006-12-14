// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imagearea.h"

#include "dbimg/img.h"

#include "jdlib/miscmsg.h"

#include "cache.h"

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif

using namespace IMAGE;


ImageAreaMain::ImageAreaMain( const std::string& url )
    : ImageAreaBase( url )
{
#ifdef _DEBUG    
    std::cout << "ImageAreaMain::ImageAreaMain url = " << url << std::endl;
#endif 

    show_image();
}



//
// 表示
//
void ImageAreaMain::show_image()
{
#ifdef _DEBUG
    std::cout << "ImageAreaMain::show_image url = " << get_url() << std::endl;
#endif    

    set_errmsg( std::string() );
    int width_max = get_width();
    int height_max = get_height();

    if( get_parent() && get_parent()->get_parent() ){     // 親(EventBox)の親(ScrolledWindow)がいるときはそのサイズ
        const int mrg = 32;
        width_max = get_parent()->get_parent()->get_width() - mrg;
        height_max = get_parent()->get_parent()->get_height() - mrg;
    }

    // まだrealizeしてなくてウィンドウサイズが取得できていないのでImageViewMain::clock_in()経由で後でもう一度呼ぶ
    if( ! is_ready()  && ( width_max <= 1 || height_max <= 1 ) ) return;

    bool mosaic = get_img()->get_mosaic();
    bool zoom_to_fit = get_img()->is_zoom_to_fit();
    int size = get_img()->get_size();

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
        double scale = 0;

        // 画面サイズに合わせる
        if( zoom_to_fit ){
            double scale_w = ( double ) width_max / get_width();
            double scale_h = ( double ) height_max / get_height();
            scale = MIN( scale_w, scale_h );
            if( scale < 1 ) do_scale = true;
        }

        else{

            // サイズ変更
            if( size != 100 ){
                scale = size/100.;
                do_scale = true;
            }
        }

        set_image( pixbuf, mosaic, do_scale, scale );

        //データベースのサイズ情報更新
        get_img()->set_size( get_width() * 100 / get_width_org() );
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
