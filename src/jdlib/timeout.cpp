// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "timeout.h"
#include "miscmsg.h"

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif

using namespace JDLIB;

#ifdef _WIN32
// static
HWND Timeout::s_hwnd;
UINT Timeout::s_idcapacity;
std::vector< Timeout* > Timeout::s_timeouts;
#endif

// private
Timeout::Timeout( const sigc::slot< bool > slot_timeout )
{
    m_slot_timeout = slot_timeout;
#ifdef _WIN32
    m_context = Glib::MainContext::get_default();
    if( ! s_hwnd )
    {
        Glib::ListHandle< Glib::RefPtr<Gdk::Window> > windows = Gdk::Window::get_toplevels();
        if( ! windows.empty() )
        {
            // Gdk::Window will be get named gdkWindowTemp
            Glib::RefPtr<Gdk::Window> p_win = *( windows.begin() );
            s_hwnd = static_cast<HWND>( GDK_WINDOW_HWND( p_win->gobj() ));
            s_idcapacity = 0;
        }
        else
        {
            MISC::ERRMSG( "hwnd empty" );
            assert( s_hwnd );
        }
    }
#endif
}

Timeout::~Timeout()
{
#ifdef _WIN32
    KillTimer( s_hwnd, m_idevent );
    s_timeouts[ m_idevent ] = NULL;
#else
    m_connection.disconnect();
#endif
}

// static
Timeout* Timeout::connect( const sigc::slot< bool > slot_timeout, unsigned int interval )
{
    Timeout* timeout = new Timeout( slot_timeout );
#ifdef _WIN32
    bool hasBlank = false;
    for( UINT i=0; i < s_idcapacity; i++ )
    {
        if( s_timeouts[i] == NULL )
        {
            timeout->m_idevent = i;
            s_timeouts[i] = timeout;
            hasBlank = true;
            break;
        }
    }
    if( ! hasBlank )
    {
        timeout->m_idevent = s_idcapacity++;
        s_timeouts.push_back( timeout );
    }
    SetTimer( s_hwnd, timeout->m_idevent, interval, slot_timeout_win32 );
#else
    timeout->m_connection = Glib::signal_timeout().connect( slot_timeout, interval );
#endif
    return timeout;
}

#ifdef _WIN32
void Timeout::slot_timeout_call()
{
    m_context->acquire();
    m_slot_timeout();
    m_context->release();
}

// static
VOID CALLBACK Timeout::slot_timeout_win32( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
    Timeout* timeout = s_timeouts[ idEvent ];
    timeout->slot_timeout_call();
}
#endif

