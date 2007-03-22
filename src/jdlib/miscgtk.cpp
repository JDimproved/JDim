// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscgtk.h"

#include <errno.h> // ERANGE

// char の初期化用
#define CHAR_BUF 256


// Gdk::Color -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::Color& color )
{
    // R,G,Bを取得
    int l_rgb[3];
    l_rgb[0] = color.get_red();
    l_rgb[1] = color.get_green();
    l_rgb[2] = color.get_blue();

    return color_to_str( l_rgb );
}

// int[3] -> 16進数表記の文字列
std::string MISC::color_to_str( const int* l_rgb )
{
    // 16進数表記で各色の文字列を連結して格納
    char str_value[ CHAR_BUF ];
    snprintf( str_value, CHAR_BUF, "#%04x%04x%04x", l_rgb[ 0 ], l_rgb[ 1 ], l_rgb[ 2 ] );

#ifdef _DEBUG
    std::cout << "MISC::color_to_str" << std::endl;
    std::cout << l_rgb[0] << ", " << l_rgb[1] << ", " << l_rgb[2] << "->"
              << str_value << std::endl;
#endif

    return str_value;
}

// Gdk::Color -> int 変換
guint32 MISC::color_to_int( const Gdk::Color& color )
{
    guint32 red = color.get_red() >> 8;
    guint32 green = color.get_green() >> 8;
    guint32 blue = color.get_blue() >> 8;

    return ( red << 24 ) + ( green << 16 ) + ( blue << 8 );
}


// 使用可能なフォントの一覧をリストで取得
std::list< std::string > MISC::get_font_families()
{
    std::list< std::string > list_out;

    Gtk::DrawingArea dummy;
    std::list< Glib::RefPtr< Pango::FontFamily > > list_families = dummy.get_pango_context()->list_families();
    std::list< Glib::RefPtr< Pango::FontFamily > >::iterator it = list_families.begin();
    for(; it != list_families.end(); ++it ){
#ifdef _DEBUG
        std::cout << (*it)->get_name() << std::endl;
#endif
        list_out.push_back( (*it)->get_name() );
    }

    return list_out;
}
