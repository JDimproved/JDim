// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscgtk.h"
#include "imgloader.h"


enum
{
    CHAR_BUF = 256 // char の初期化用
};


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

#if GTKMM_CHECK_VERSION(3,0,0)
// Gdk::RGBA -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::RGBA& rgba )
{
    int l_rgb[ 3 ];
    l_rgb[ 0 ] = rgba.get_red_u();
    l_rgb[ 1 ] = rgba.get_green_u();
    l_rgb[ 2 ] = rgba.get_blue_u();
    return color_to_str( l_rgb );
}
#endif // GTKMM_CHECK_VERSION(3,0,0)


// htmlカラー (#ffffffなど) -> 16進数表記の文字列
std::string MISC::htmlcolor_to_str( const std::string& _htmlcolor )
{
    std::string htmlcolor = _htmlcolor;
    int rgb[ 3 ];

    if( htmlcolor == "red" )          htmlcolor = "#ff0000";
    else if( htmlcolor == "fuchsia" ) htmlcolor = "#ff00ff";
    else if( htmlcolor == "purple" )  htmlcolor = "#800080";
    else if( htmlcolor == "maroon" )  htmlcolor = "#800000";
    else if( htmlcolor == "yellow" )  htmlcolor = "#ffff00";
    else if( htmlcolor == "lime" )    htmlcolor = "#00ff00";
    else if( htmlcolor == "green" )   htmlcolor = "#008000";
    else if( htmlcolor == "olive" )   htmlcolor = "#808000";
    else if( htmlcolor == "blue" )    htmlcolor = "#0000ff";
    else if( htmlcolor == "aqua" )    htmlcolor = "#00ffff";
    else if( htmlcolor == "teal" )    htmlcolor = "#008080";
    else if( htmlcolor == "navy" )    htmlcolor = "#000080";
    else if( htmlcolor == "white" )   htmlcolor = "#ffffff";
    else if( htmlcolor == "silver" )  htmlcolor = "#c0c0c0";
    else if( htmlcolor == "gray" )    htmlcolor = "#808080";
    else if( htmlcolor == "black" )   htmlcolor = "#000000";

    int offset = 0;
    if( htmlcolor.find( "#" ) == 0 ) offset = 1;

    std::string tmpstr = htmlcolor.substr( offset, 2 );
    rgb[ 0 ] = strtol( std::string( "0x" + tmpstr + tmpstr  ).c_str(), NULL, 16 );

    tmpstr = htmlcolor.substr( 2 + offset, 2 );
    rgb[ 1 ] = strtol( std::string( "0x" + tmpstr + tmpstr  ).c_str(), NULL, 16 );

    tmpstr = htmlcolor.substr( 4 + offset, 2 );
    rgb[ 2 ] = strtol( std::string( "0x" + tmpstr + tmpstr  ).c_str(), NULL, 16 );

#ifdef _DEBUG
    std::cout << "MISC::htmlcolor_to_gdkcolor color = " << htmlcolor 
              << " r = " << rgb[ 0 ] << " g = " << rgb[ 1 ] << " b = " << rgb[ 2 ] << std::endl;
#endif

    return color_to_str( rgb );
}


// Gdk::Color -> int 変換
guint32 MISC::color_to_int( const Gdk::Color& color )
{
    guint32 red = color.get_red() >> 8;
    guint32 green = color.get_green() >> 8;
    guint32 blue = color.get_blue() >> 8;

    return ( red << 24 ) + ( green << 16 ) + ( blue << 8 );
}


// 使用可能なフォントの一覧を取得
std::set< std::string > MISC::get_font_families()
{
    std::set< std::string > set_out;

    Gtk::DrawingArea dummy;
    std::list< Glib::RefPtr< Pango::FontFamily > > list_families = dummy.get_pango_context()->list_families();
    std::list< Glib::RefPtr< Pango::FontFamily > >::iterator it = list_families.begin();
    for(; it != list_families.end(); ++it ){
#ifdef _DEBUG
        std::cout << (*it)->get_name() << std::endl;
#endif
        set_out.insert( (*it)->get_name() );
    }

    return set_out;
}


// gtk::entryのフォント名を取得
std::string MISC::get_entry_font()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    return entry.get_style_context()->get_font().to_string();
#else
    return entry.get_style()->get_font().to_string();
#endif
}


// gtk::entryの文字色を16進数表記の文字列で取得
std::string MISC::get_entry_color_text()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    auto rgba = entry.get_style_context()->get_color( Gtk::STATE_FLAG_NORMAL );
    return color_to_str( rgba );
#else
    return color_to_str( entry.get_style()->get_text( Gtk::STATE_NORMAL ) );
#endif
}


// gtk::entryの背景色を16進数表記の文字列で取得
std::string MISC::get_entry_color_base()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    // REVIEW: get_background_color()が期待通りに背景色を返さない環境があった
    auto context = entry.get_style_context();
    Gdk::RGBA rgba;
    if( !context->lookup_color( "theme_base_color", rgba ) ) {
#ifdef _DEBUG
        std::cout << "ERROR:MISC::get_entry_color_base() "
                  << "lookup theme_base_color failed." << std::endl;
#endif
    }
    return color_to_str( rgba );
#else
    return color_to_str( entry.get_style()->get_base( Gtk::STATE_NORMAL ) );
#endif
}



// str をクリップボードにコピー
void MISC::CopyClipboard( const std::string& str )
{
    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    clip->set_text( str );
    clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
    clip->set_text( str );
}

