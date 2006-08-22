// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "iconmanager.h"

#include "jd16.h"
#include "jd32.h"
#include "jd48.h"

#include "dir.h"
#include "board.h"
#include "board_update.h"
#include "thread.h"
#include "thread_update.h"
#include "image.h"
#include "link.h"
#include "loading.h"
#include "loading_stop.h"
#include "check.h"
#include "down.h"
#include "update.h"


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

    m_list_icons[ ICON_JD16 ] =  Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16 ), icon_jd16 );
    m_list_icons[ ICON_JD32 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32 ), icon_jd32 );
    m_list_icons[ ICON_JD48 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48 ), icon_jd48 );

    m_list_icons[ ICON_ADD16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_update ), icon_update );
    m_list_icons[ ICON_CHECK16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_check ), icon_check );
    m_list_icons[ ICON_DOWN16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_down ), icon_down );

    m_list_icons[ ICON_DIR16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_dir ), icon_dir );
    m_list_icons[ ICON_BOARD16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board ), icon_board );
    m_list_icons[ ICON_THREAD16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );
    m_list_icons[ ICON_IMAGE16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_image ), icon_image );
    m_list_icons[ ICON_LINK16 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_link ), icon_link );

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
