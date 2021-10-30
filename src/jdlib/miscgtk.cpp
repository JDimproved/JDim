// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscgtk.h"
#include "miscutil.h"
#include "imgloader.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <vector>


enum
{
    CHAR_BUF = 256 // char の初期化用
};


// 色キーワードの検索で使う色名と16進表記を対応付ける配列
// https://developer.mozilla.org/en-US/docs/Web/CSS/color_value
// 二分探索するため配列の要素は予めnameで辞書順にソートしておく
static struct color_map {
    const char *name;
    const char *rgb;
} constexpr const color_names[] = {
    { "aliceblue",      "#F0F8FF" }, // since CSS3
    { "antiquewhite",   "#FAEBD7" }, // since CSS3
    { "aqua",           "#00ffff" }, // since CSS1
    { "aquamarine",     "#7FFFD4" }, // since CSS3
    { "azure",          "#F0FFFF" }, // since CSS3
    { "beige",          "#F5F5DC" }, // since CSS3
    { "bisque",         "#FFE4C4" }, // since CSS3
    { "black",          "#000000" }, // since CSS1
    { "blanchedalmond", "#FFEBCD" }, // since CSS3
    { "blue",           "#0000ff" }, // since CSS1
    { "blueviolet",     "#8A2BE2" }, // since CSS3
    { "brown",          "#A52A2A" }, // since CSS3
    { "burlywood",      "#DEB887" }, // since CSS3
    { "cadetblue",      "#5F9EA0" }, // since CSS3
    { "chartreuse",     "#7FFF00" }, // since CSS3
    { "chocolate",      "#D2691E" }, // since CSS3
    { "coral",          "#FF7F50" }, // since CSS3
    { "cornflowerblue", "#6495ED" }, // since CSS3
    { "cornsilk",       "#FFF8DC" }, // since CSS3
    { "crimson",        "#DC143C" }, // since CSS3
    { "cyan",           "#00FFFF" }, // since CSS3
    { "darkblue",       "#00008B" }, // since CSS3
    { "darkcyan",       "#008B8B" }, // since CSS3
    { "darkgoldenrod",  "#B8860B" }, // since CSS3
    { "darkgray",       "#A9A9A9" }, // since CSS3
    { "darkgreen",      "#006400" }, // since CSS3
    { "darkgrey",       "#A9A9A9" }, // since CSS3
    { "darkkhaki",      "#BDB76B" }, // since CSS3
    { "darkmagenta",    "#8B008B" }, // since CSS3
    { "darkolivegreen", "#556B2F" }, // since CSS3
    { "darkorange",     "#FF8C00" }, // since CSS3
    { "darkorchid",     "#9932CC" }, // since CSS3
    { "darkred",        "#8B0000" }, // since CSS3
    { "darksalmon",     "#E9967A" }, // since CSS3
    { "darkseagreen",   "#8FBC8F" }, // since CSS3
    { "darkslateblue",  "#483D8B" }, // since CSS3
    { "darkslategray",  "#2F4F4F" }, // since CSS3
    { "darkslategrey",  "#2F4F4F" }, // since CSS3
    { "darkturquoise",  "#00CED1" }, // since CSS3
    { "darkviolet",     "#9400D3" }, // since CSS3
    { "deeppink",       "#FF1493" }, // since CSS3
    { "deepskyblue",    "#00BFFF" }, // since CSS3
    { "dimgray",        "#696969" }, // since CSS3
    { "dimgrey",        "#696969" }, // since CSS3
    { "dodgerblue",     "#1E90FF" }, // since CSS3
    { "firebrick",      "#B22222" }, // since CSS3
    { "floralwhite",    "#FFFAF0" }, // since CSS3
    { "forestgreen",    "#228B22" }, // since CSS3
    { "fuchsia",        "#ff00ff" }, // since CSS1
    { "gainsboro",      "#DCDCDC" }, // since CSS3
    { "ghostwhite",     "#F8F8FF" }, // since CSS3
    { "gold",           "#FFD700" }, // since CSS3
    { "goldenrod",      "#DAA520" }, // since CSS3
    { "gray",           "#808080" }, // since CSS1
    { "green",          "#008000" }, // since CSS1
    { "greenyellow",    "#ADFF2F" }, // since CSS3
    { "grey",           "#808080" }, // since CSS3
    { "honeydew",       "#F0FFF0" }, // since CSS3
    { "hotpink",        "#FF69B4" }, // since CSS3
    { "indianred",      "#CD5C5C" }, // since CSS3
    { "indigo",         "#4B0082" }, // since CSS3
    { "ivory",          "#FFFFF0" }, // since CSS3
    { "khaki",          "#F0E68C" }, // since CSS3
    { "lavender",       "#E6E6FA" }, // since CSS3
    { "lavenderblush",  "#FFF0F5" }, // since CSS3
    { "lawngreen",      "#7CFC00" }, // since CSS3
    { "lemonchiffon",   "#FFFACD" }, // since CSS3
    { "lightblue",      "#ADD8E6" }, // since CSS3
    { "lightcoral",     "#F08080" }, // since CSS3
    { "lightcyan",      "#E0FFFF" }, // since CSS3
    { "lightgoldenrodyellow",   "#FAFAD2" }, // since CSS3
    { "lightgray",      "#D3D3D3" }, // since CSS3
    { "lightgreen",     "#90EE90" }, // since CSS3
    { "lightgrey",      "#D3D3D3" }, // since CSS3
    { "lightpink",      "#FFB6C1" }, // since CSS3
    { "lightsalmon",    "#FFA07A" }, // since CSS3
    { "lightseagreen",  "#20B2AA" }, // since CSS3
    { "lightskyblue",   "#87CEFA" }, // since CSS3
    { "lightslategray", "#778899" }, // since CSS3
    { "lightslategrey", "#778899" }, // since CSS3
    { "lightsteelblue", "#B0C4DE" }, // since CSS3
    { "lightyellow",    "#FFFFE0" }, // since CSS3
    { "lime",           "#00ff00" }, // since CSS1
    { "limegreen",      "#32CD32" }, // since CSS3
    { "linen",          "#FAF0E6" }, // since CSS3
    { "magenta",        "#FF00FF" }, // since CSS3
    { "maroon",         "#800000" }, // since CSS1
    { "mediumaquamarine",   "#66CDAA" }, // since CSS3
    { "mediumblue",     "#0000CD" }, // since CSS3
    { "mediumorchid",   "#BA55D3" }, // since CSS3
    { "mediumpurple",   "#9370DB" }, // since CSS3
    { "mediumseagreen", "#3CB371" }, // since CSS3
    { "mediumslateblue","#7B68EE" }, // since CSS3
    { "mediumspringgreen",  "#00FA9A" }, // since CSS3
    { "mediumturquoise","#48D1CC" }, // since CSS3
    { "mediumvioletred","#C71585" }, // since CSS3
    { "midnightblue",   "#191970" }, // since CSS3
    { "mintcream",      "#F5FFFA" }, // since CSS3
    { "mistyrose",      "#FFE4E1" }, // since CSS3
    { "moccasin",       "#FFE4B5" }, // since CSS3
    { "navajowhite",    "#FFDEAD" }, // since CSS3
    { "navy",           "#000080" }, // since CSS1
    { "oldlace",        "#FDF5E6" }, // since CSS3
    { "olive",          "#808000" }, // since CSS1
    { "olivedrab",      "#6B8E23" }, // since CSS3
    { "orange",         "#ffa500" }, // since CSS2.1
    { "orangered",      "#FF4500" }, // since CSS3
    { "orchid",         "#DA70D6" }, // since CSS3
    { "palegoldenrod",  "#EEE8AA" }, // since CSS3
    { "palegreen",      "#98FB98" }, // since CSS3
    { "paleturquoise",  "#AFEEEE" }, // since CSS3
    { "palevioletred",  "#DB7093" }, // since CSS3
    { "papayawhip",     "#FFEFD5" }, // since CSS3
    { "peachpuff",      "#FFDAB9" }, // since CSS3
    { "peru",           "#CD853F" }, // since CSS3
    { "pink",           "#FFC0CB" }, // since CSS3
    { "plum",           "#DDA0DD" }, // since CSS3
    { "powderblue",     "#B0E0E6" }, // since CSS3
    { "purple",         "#800080" }, // since CSS1
    { "rebeccapurple",  "#663399" }, // since CSS4
    { "red",            "#ff0000" }, // since CSS1
    { "rosybrown",      "#BC8F8F" }, // since CSS3
    { "royalblue",      "#4169E1" }, // since CSS3
    { "saddlebrown",    "#8B4513" }, // since CSS3
    { "salmon",         "#FA8072" }, // since CSS3
    { "sandybrown",     "#F4A460" }, // since CSS3
    { "seagreen",       "#2E8B57" }, // since CSS3
    { "seashell",       "#FFF5EE" }, // since CSS3
    { "sienna",         "#A0522D" }, // since CSS3
    { "silver",         "#c0c0c0" }, // since CSS1
    { "skyblue",        "#87CEEB" }, // since CSS3
    { "slateblue",      "#6A5ACD" }, // since CSS3
    { "slategray",      "#708090" }, // since CSS3
    { "slategrey",      "#708090" }, // since CSS3
    { "snow",           "#FFFAFA" }, // since CSS3
    { "springgreen",    "#00FF7F" }, // since CSS3
    { "steelblue",      "#4682B4" }, // since CSS3
    { "tan",            "#D2B48C" }, // since CSS3
    { "teal",           "#008080" }, // since CSS1
    { "thistle",        "#D8BFD8" }, // since CSS3
    { "tomato",         "#FF6347" }, // since CSS3
    { "turquoise",      "#40E0D0" }, // since CSS3
    { "violet",         "#EE82EE" }, // since CSS3
    { "wheat",          "#F5DEB3" }, // since CSS3
    { "white",          "#ffffff" }, // since CSS1
    { "whitesmoke",     "#F5F5F5" }, // since CSS3
    { "yellow",         "#ffff00" }, // since CSS1
    { "yellowgreen",    "#9ACD32" }, // since CSS3
};

static bool color_map_less( const color_map& a, const color_map& b )
{
    return std::strcmp( a.name, b.name ) < 0;
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

// Gdk::RGBA -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::RGBA& rgba )
{
    int l_rgb[ 3 ];
    l_rgb[ 0 ] = rgba.get_red_u();
    l_rgb[ 1 ] = rgba.get_green_u();
    l_rgb[ 2 ] = rgba.get_blue_u();
    return color_to_str( l_rgb );
}


// htmlカラー (#ffffffなど) -> 16進数表記の文字列
//
// 入力文字列は大文字小文字を区別しない
// 未知のキーワードや不正な値は変換して返す
std::string MISC::htmlcolor_to_str( const std::string& _htmlcolor )
{
    std::string htmlcolor = MISC::tolower_str( _htmlcolor );

    if( htmlcolor[0] != '#' ) {
        const color_map key{ htmlcolor.c_str(), nullptr };
        auto it = std::lower_bound( std::begin( color_names ), std::end( color_names ), key, color_map_less );

        // キーワードに一致しないときは空文字列を返す
        if( it == std::end( color_names ) ) return {};
        htmlcolor = it->rgb;
    }

    const int digits = ( htmlcolor.size() == 4 ) ? 1 : 2;
    constexpr int len = 3;
    int rgb[len];

    for( int i = 0; i < len; ++i ) {
        constexpr int offset = 1;
        std::string tmpstr = htmlcolor.substr( offset + ( i * digits ), digits );
        for( int j = 0; j < ( len - digits ); ++j ) tmpstr.append( tmpstr );
        // 不正な値はstrtol()の挙動に従って変換される
        rgb[i] = std::strtol( tmpstr.c_str(), nullptr, 16 );
    }

#ifdef _DEBUG
    std::cout << "MISC::htmlcolor_to_gdkcolor color = " << htmlcolor 
              << " r = " << rgb[ 0 ] << " g = " << rgb[ 1 ] << " b = " << rgb[ 2 ] << std::endl;
#endif

    return color_to_str( rgb );
}


// Gdk::RGBA -> int 変換
guint32 MISC::color_to_int( const Gdk::RGBA& color )
{
    guint32 red = color.get_red_u() >> 8;
    guint32 green = color.get_green_u() >> 8;
    guint32 blue = color.get_blue_u() >> 8;

    return ( red << 24 ) + ( green << 16 ) + ( blue << 8 );
}


// 使用可能なフォントの一覧を取得
std::set< std::string > MISC::get_font_families()
{
    std::set< std::string > set_out;

    Gtk::DrawingArea dummy;
    const std::vector<Glib::RefPtr<Pango::FontFamily>> list_families = dummy.get_pango_context()->list_families();
    for( const auto& family : list_families ) {
#ifdef _DEBUG
        std::cout << family->get_name() << std::endl;
#endif
        set_out.insert( family->get_name() );
    }

    return set_out;
}


// gtk::entryのフォント名を取得
std::string MISC::get_entry_font()
{
    Gtk::Entry entry;
    return entry.get_style_context()->get_font().to_string();
}


// gtk::entryの文字色を16進数表記の文字列で取得
std::string MISC::get_entry_color_text()
{
    Gtk::Entry entry;
    auto rgba = entry.get_style_context()->get_color( Gtk::STATE_FLAG_NORMAL );
    return color_to_str( rgba );
}


// gtk::entryの背景色を16進数表記の文字列で取得
std::string MISC::get_entry_color_base()
{
    Gtk::Entry entry;
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
}



// str をクリップボードにコピー
void MISC::CopyClipboard( const std::string& str )
{
    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    clip->set_text( str );
    clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
    clip->set_text( str );
}


// ウインドウの左上隅を基準としたマウスポインターの座標を取得
Glib::RefPtr<Gdk::Window> MISC::get_pointer_at_window( const Glib::RefPtr<const Gdk::Window>& window, int& x, int& y )
{
    assert( window );
    Gdk::ModifierType unused;
    const auto device = Gdk::Display::get_default()->get_default_seat()->get_pointer();
    return window->get_device_position( device, x, y, unused );
}
