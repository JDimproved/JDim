// ライセンス: GPL2

#ifndef _MESSAGEWIN_H
#define _MESSAGEWIN_H

#include <gtkmm.h>

#include "skeleton/vbox.h"

namespace MESSAGE
{
    class MessageWin : public Gtk::Window
    {
        bool m_maximized;

        SKELETON::JDVBox m_vbox;

        // ステータスバー
#if GTKMMVER <= 240
        Gtk::Statusbar m_statbar;
#else
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
#endif

      public:
        MessageWin();
        ~MessageWin();

        void pack_remove( bool unpack, Gtk::Widget& view );
        void set_status( const std::string& stat );

      protected:
        virtual bool on_delete_event( GdkEventAny* event );
        virtual bool on_window_state_event( GdkEventWindowState* event );
    };
}


#endif
