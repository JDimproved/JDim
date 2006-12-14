// ライセンス: GPL2

#ifndef _MESSAGEWIN_H
#define _MESSAGEWIN_H

#include <gtkmm.h>

namespace MESSAGE
{
    class MessageWin : public Gtk::Window
    {
        bool m_maximized;

      public:
        MessageWin();
        ~MessageWin();

      protected:
        virtual bool on_delete_event( GdkEventAny* event );
        virtual bool on_window_state_event( GdkEventWindowState* event );
    };
}


#endif
