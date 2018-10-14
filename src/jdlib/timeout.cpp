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
JDLIB::StaticMutex Timeout::s_lock = JDLIB_STATIC_MUTEX_INIT;
std::map< UINT_PTR, Timeout* > Timeout::s_timeouts;
#endif

/*
 * Glib::signal_timeout()はタイムアウトのタイミングを得るために、インターバルに応じたsleepで
 * g_get_current_time()を呼んで時刻取得することで、タイミングを得ている
 * マイクロ秒オーダのインターバルでは、Windowsでの時刻取得にかかる負荷は無視できないので、
 * Windowsタイマーに基づいてタイムアウトを発生させる
*/
// private
Timeout::Timeout( const sigc::slot< bool > slot_timeout )
    : m_slot_timeout( slot_timeout )
{
#ifdef _WIN32
    m_identifer = 0;
    m_context = Glib::MainContext::get_default();
#endif
}

Timeout::~Timeout()
{
#ifdef _WIN32
    if( m_identifer != 0 ){
        JDLIB::LockGuard lock( s_lock );
        KillTimer( NULL, m_identifer );
        s_timeouts.erase( m_identifer );
        m_identifer = 0;
    }
#else
    m_connection.disconnect();
#endif
}

// static
Timeout* Timeout::connect( const sigc::slot< bool > slot_timeout, unsigned int interval )
{
    Timeout* timeout = new Timeout( slot_timeout );
#ifdef _WIN32
    JDLIB::LockGuard lock( s_lock );
    // use global windows timer
    UINT_PTR ident = SetTimer( NULL, 0, interval, slot_timeout_win32 );
    if( ident != 0 ) {
        // register object into static domain
        s_timeouts.insert( std::map< UINT_PTR, Timeout* >::value_type( ident, timeout ));
        timeout->m_identifer = ident;
    } else {
        std::stringstream msg;
        msg << "Set timer : " << std::hex << GetLastError();
        MISC::ERRMSG( msg.str() );
    }
#else
    timeout->m_connection = Glib::signal_timeout().connect( slot_timeout, interval );
#endif
    return timeout;
}

#ifdef _WIN32
void Timeout::slot_timeout_callback()
{
    m_context->acquire();
    m_slot_timeout();
    m_context->release();
}

// static
VOID CALLBACK Timeout::slot_timeout_win32( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
    Timeout* timeout = s_timeouts[ idEvent ];
    if( timeout != NULL ){
        timeout->slot_timeout_callback();
    }
}
#endif
