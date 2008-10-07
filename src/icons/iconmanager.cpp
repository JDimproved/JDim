// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "iconmanager.h"

#include "jd16.h"
#include "jd32.h"
#include "jd48.h"
#include "jd96.h"

#include "bkmark_update.h"
#include "bkmark.h"
#include "bkmark_thread.h"

#include "update.h"
#include "newthread.h"
#include "newthread_hour.h"
#include "check.h"
#include "down.h"
#include "write.h"
#include "post.h"
#include "post_refer.h"
#include "loading.h"
#include "loading_stop.h"

#include "dir.h"
#include "favorite.h"
#include "board.h"
#include "board_update.h"
#include "board_updated.h"
#include "thread.h"
#include "thread_update.h"
#include "thread_updated.h"
#include "thread_old.h"
#include "image.h"
#include "link.h"

#if GTKMMVER <= 240
#include "play.h"
#endif


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

    m_list_icons[ ICON::JD16 ] =  Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16 ), icon_jd16 );
    m_list_icons[ ICON::JD32 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32 ), icon_jd32 );
    m_list_icons[ ICON::JD48 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48 ), icon_jd48 );
    m_list_icons[ ICON::JD96 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd96 ), icon_jd96 );

    m_list_icons[ ICON::BKMARK_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_update ), icon_bkmark_update );
    m_list_icons[ ICON::BKMARK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark ), icon_bkmark );
    m_list_icons[ ICON::BKMARK_THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_thread ), icon_bkmark_thread );

    m_list_icons[ ICON::UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_update ), icon_update );
    m_list_icons[ ICON::NEWTHREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread ), icon_newthread );
    m_list_icons[ ICON::NEWTHREAD_HOUR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread_hour ), icon_newthread_hour );
    m_list_icons[ ICON::CHECK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_check ), icon_check );
    m_list_icons[ ICON::DOWN ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_down ), icon_down );
    m_list_icons[ ICON::WRITE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_write ), icon_write );
    m_list_icons[ ICON::POST ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post ), icon_post );
    m_list_icons[ ICON::POST_REFER ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post_refer ), icon_post_refer );
    m_list_icons[ ICON::LOADING ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading ), icon_loading );
    m_list_icons[ ICON::LOADING_STOP ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading_stop ), icon_loading_stop );

    m_list_icons[ ICON::DIR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_dir ), icon_dir );
    m_list_icons[ ICON::FAVORITE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_favorite ), icon_favorite );
    m_list_icons[ ICON::BOARD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board ), icon_board );
    m_list_icons[ ICON::BOARD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_update ), icon_board_update );
    m_list_icons[ ICON::BOARD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_updated ), icon_board_updated );
    m_list_icons[ ICON::THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );
    m_list_icons[ ICON::THREAD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_update ), icon_thread_update );
    m_list_icons[ ICON::THREAD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_updated ), icon_thread_updated );
    m_list_icons[ ICON::THREAD_OLD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_old ), icon_thread_old );
    m_list_icons[ ICON::IMAGE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_image ), icon_image );
    m_list_icons[ ICON::LINK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_link ), icon_link );

#if GTKMMVER <= 240
    m_list_icons[ ICON::PLAY ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_play ), icon_play );
#endif

    m_list_icons[ ICON::TRANSPARENT ] = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    m_list_icons[ ICON::TRANSPARENT ]->fill( 0 );
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
