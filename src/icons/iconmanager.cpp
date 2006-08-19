// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "iconmanager.h"

#include "icons/icon_jd16.h"
#include "icons/icon_jd32.h"
#include "icons/icon_jd48.h"

#include "icons/icon_add16.h"
#include "icons/icon_check16.h"
#include "icons/icon_down16.h"


ICON::ICON_Manager* instance_icon_manager = NULL;


ICON::ICON_Manager* ICON::get_icon_manager()
{
    if( ! instance_icon_manager ) instance_icon_manager = new ICON::ICON_Manager();
    assert( instance_icon_manager );

    return instance_icon_manager;
}


void ICON::delete_icon_manager()
{
    if( instance_icon_manager ) delete instance_icon_manager;
    instance_icon_manager = NULL;
}


Glib::RefPtr< Gdk::Pixbuf > ICON::get_icon( int id )
{
    return get_icon_manager()->get_icon( id );
}


///////////////////////////////////////////////

using namespace ICON;


ICON_Manager::ICON_Manager()
{

    m_list_icons.resize( NUM_ICONS );

    m_list_icons[ ICON_JD16 ] =  Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16_png ), icon_jd16_png );
    m_list_icons[ ICON_JD32 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32_png ), icon_jd32_png );
    m_list_icons[ ICON_JD48 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48_png ), icon_jd48_png );

    m_list_icons[ ICON_ADD16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_add16_png ), icon_add16_png );
    m_list_icons[ ICON_CHECK16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_check16_png ), icon_check16_png );
    m_list_icons[ ICON_DOWN16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_down16_png ), icon_down16_png );

    // ストックアイコンからpixbufを作る方法が分からなかったので、ダミーのwidgetを作ってrender_icon()を呼ぶ
    Gtk::DrawingArea dummywidget;
    m_list_icons[ ICON_DIR16 ] = dummywidget.render_icon( Gtk::Stock::OPEN, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON_BOARD16 ] = dummywidget.render_icon( Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON_THREAD16 ] = dummywidget.render_icon( Gtk::Stock::COPY, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON_IMAGE16 ] = dummywidget.render_icon( Gtk::Stock::NO, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON_LINK16 ] = dummywidget.render_icon( Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_MENU );

    m_list_icons[ ICON_TRANSPARENT ] = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    m_list_icons[ ICON_TRANSPARENT ]->fill( 0 );
}


ICON_Manager::~ICON_Manager()
{
#ifdef _DEBUG
    std::cout << "ICON::~ICON_Manager\n";
#endif
}


Glib::RefPtr< Gdk::Pixbuf > ICON_Manager::get_icon( int id )
{
    return m_list_icons[ id ];
}
