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


// 画像の幅と高さを取得
void MISC::get_img_size( const std::string& filename, int& width, int& height )
{
    width = height = 0;

    MISC::Img_Size imgsize( filename );
    width = imgsize.get_width();
    height = imgsize.get_height();
}


MISC::Img_Size::Img_Size( const std::string& file )
    : m_width( 0 ),
      m_height( 0 )
{
    FILE* f = NULL;
    const size_t bufsize = 1024;
    size_t readsize = 0;
    guint8 data[ bufsize ];

    f = fopen( file.c_str(), "rb" );
    if( f ){

        try {

            Glib::RefPtr< Gdk::PixbufLoader > loader = Gdk::PixbufLoader::create();
            loader->signal_size_prepared().connect( sigc::mem_fun( *this, &Img_Size::slot_size_prepared ) );

            while( ! m_width ){
                readsize = fread( data, 1, bufsize, f );
                if( readsize ) loader->write( data, readsize );
                if( feof( f ) ) break;
            }

            loader->close();
        }
        catch( Glib::Error& err )
        {
//            MISC::ERRMSG( err.what() );
        }

        fclose( f );
    }

#ifdef _DEBUG
    std::cout << "Img_Size::Img_Size read = " << readsize << "  w = " << m_width << " h = " << m_height << std::endl;
#endif
}


void MISC::Img_Size::slot_size_prepared( int w, int h )
{
#ifdef _DEBUG
    std::cout << "Img_Size::slot_size_prepared w = " << w << " h = " << h << std::endl;
#endif

    m_width = w;
    m_height = h;
}


//
// 画像ローダ取得
//
// stop を trueにすると読み込みを停止する
//
Glib::RefPtr< Gdk::PixbufLoader > MISC::get_ImageLoder( const std::string& file, int width, int height, bool& stop, std::string& errmsg )
{
    Glib::RefPtr< Gdk::PixbufLoader > loader;

    const size_t bufsize = 8192;
    size_t readsize = 0;
    guint8 data[ bufsize ];

    FILE* f = fopen( file.c_str(), "rb" );
    if( f ){

        try {

            loader = Gdk::PixbufLoader::create();
            loader->set_size( width, height );

            while( ! stop ){
                readsize = fread( data, 1, bufsize, f );
                if( readsize ) loader->write( data, readsize );
                if( feof( f ) ) break;
            }
            loader->close();
        }
        catch( Glib::Error& err )
        {
            errmsg = err.what();
        }

        fclose( f );
    }

    return loader;
}
