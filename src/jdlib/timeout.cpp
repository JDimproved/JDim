// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "timeout.h"
#include "miscmsg.h"


using namespace JDLIB;


/*
 * Glib::signal_timeout()はタイムアウトのタイミングを得るために、インターバルに応じたsleepで
 * g_get_current_time()を呼んで時刻取得することで、タイミングを得ている
*/
// private
Timeout::Timeout( const sigc::slot< bool > slot_timeout )
    : m_slot_timeout( slot_timeout )
{
}

Timeout::~Timeout()
{
    m_connection.disconnect();
}

// static
std::unique_ptr<Timeout> Timeout::connect( const sigc::slot< bool > slot_timeout, unsigned int interval )
{
    Timeout* timeout = new Timeout( slot_timeout );
    timeout->m_connection = Glib::signal_timeout().connect( slot_timeout, interval );
    return std::unique_ptr<Timeout>( timeout );
}
