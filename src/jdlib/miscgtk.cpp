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


// gtk::entryのフォント名を取得
std::string MISC::get_entry_font()
{
    Gtk::Entry entry;
    return entry.get_style()->get_font().to_string();
}


// gtk::entryの文字色を16進数表記の文字列で取得
std::string MISC::get_entry_color_text()
{
    Gtk::Entry entry;
    return color_to_str( entry.get_style()->get_text( Gtk::STATE_NORMAL ) );
}


// gtk::entryの背景色を16進数表記の文字列で取得
std::string MISC::get_entry_color_base()
{
    Gtk::Entry entry;
    return color_to_str( entry.get_style()->get_base( Gtk::STATE_NORMAL ) );
}



// str をクリップボードにコピー
void MISC::CopyClipboard( const std::string& str )
{
    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    clip->set_text( str );
    clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
    clip->set_text( str );
}


// 画像の幅と高さを取得
void MISC::get_img_size( const std::string& filename, int& width, int& height )
{
    width = height = 0;

    MISC::ImgLoader imgloader( filename );
    imgloader.get_size();
    width = imgloader.get_width();
    height = imgloader.get_height();
}


//
// PixbufLoaderローダ取得
//
// stop を trueにすると読み込みを停止する
//
Glib::RefPtr< Gdk::PixbufLoader > MISC::get_ImageLoder( const std::string& file, bool& stop, bool pixbufonly, std::string& errmsg )
{
    MISC::ImgLoader imgloader( file );
    if( ! imgloader.load( stop, pixbufonly, false ) ) errmsg = imgloader.get_errmsg();
    return imgloader.get_loader();
}



MISC::ImgLoader::ImgLoader( const std::string& file )
    : m_file( file ),
      m_width( 0 ),
      m_height( 0 ),
      m_stop( false ),
      m_y( 0 )
{}
    

// 画像サイズ取得
bool MISC::ImgLoader::get_size()
{
    bool stop = false;
    return load( stop, true, true );
}


// 画像読み込み
// stop を trueにすると読み込みを停止する
// 動画でpixbufonly = true の時はアニメーションさせない
// sizeonly = true の時はサイズの取得のみ
bool MISC::ImgLoader::load( bool& stop, bool pixbufonly, bool sizeonly )
{
    if( sizeonly && m_width && m_height ) return true;
    if( m_loader ) return true;

    m_pixbufonly = pixbufonly;

#ifdef _DEBUG
    std::cout << "MISC::ImgLoader sizeonly = " << sizeonly
              << " file = " << m_file << std::endl;
    size_t total = 0;
#endif

    bool ret = true;

    FILE* f = NULL;
    const size_t bufsize = 8192;
    size_t readsize = 0;
    guint8 data[ bufsize ];

    f = fopen( m_file.c_str(), "rb" );
    if( f ){

        try {

            m_loader = Gdk::PixbufLoader::create();

#if GTKMMVER > 240
            if( sizeonly ) m_loader->signal_size_prepared().connect( sigc::mem_fun( *this, &ImgLoader::slot_size_prepared ) );
#endif

            m_loader->signal_area_updated().connect( sigc::mem_fun( *this, &ImgLoader::slot_area_updated ) );

            while( ! m_stop && ! stop  ){

                readsize = fread( data, 1, bufsize, f );
                if( readsize ) m_loader->write( data, readsize );

#ifdef _DEBUG
                total += readsize;
                std::cout << readsize << " / " << total << std::endl;
#endif
                if( feof( f ) ) break;

#if GTKMMVER <= 240 // gdkのバージョンが古い場合はpixbufを取得してサイズを得る

                if( sizeonly && m_loader->get_pixbuf() ){
                    m_width = m_loader->get_pixbuf()->get_width();
                    m_height = m_loader->get_pixbuf()->get_height();
                    m_stop = true;
                }
#endif
            }

            m_loader->close();
        }
        catch( Glib::Error& err )
        {
            if( ! m_stop ){
                m_errmsg = err.what();
                ret = false;
            }
        }

        fclose( f );
    }

#ifdef _DEBUG
    std::cout << "ImgLoader::load read = " << total << "  w = " << m_width << " h = " << m_height << std::endl;
#endif

    return ret;
}


void MISC::ImgLoader::slot_size_prepared( int w, int h )
{
#ifdef _DEBUG
    std::cout << "ImgLoader::slot_size_prepared w = " << w << " h = " << h << std::endl;
#endif

    m_width = w;
    m_height = h;
    m_stop = true;
}


void MISC::ImgLoader::slot_area_updated(int x, int y, int w, int h )
{
    if( m_pixbufonly ){

#ifdef _DEBUG
        std::cout << "ImgLoader::slot_area_updated x = " << x << " y = " << y << " w = " << w << " h = " << h << std::endl;
#endif

        if( y < m_y ) m_stop = true;
        m_y = y;
    }
}
