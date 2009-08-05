// ライセンス: GPL2

#ifndef _TIMEOUT_H
#define _TIMEOUT_H

#include <gtkmm.h>
#ifdef _WIN32
#include <windows.h>
#undef DELETE // conflict with Gtk::Stock::DELETE
#endif

namespace JDLIB
{
    class Timeout
    {
        sigc::slot< bool > m_slot_timeout;
#ifdef _WIN32
        Glib::RefPtr<Glib::MainContext> m_context;
        UINT m_idevent;
        
        static UINT s_idcapacity;
        static HWND s_hwnd;
        static std::vector< Timeout* > s_timeouts;
#else
        sigc::connection m_connection;
#endif

    public:
        ~Timeout();
        
        static Timeout* connect( const sigc::slot< bool > slot_timeout,
            unsigned int interval );

    private:
        Timeout( const sigc::slot< bool > slot_timeout );
        
#ifdef _WIN32
        void slot_timeout();
        static VOID CALLBACK slot_timeout_win32( HWND, UINT, UINT, DWORD );
#endif
    };
}

#endif // _TIMEOUT_H
