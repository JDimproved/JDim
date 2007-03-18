// ライセンス: GPL2

//
// Window クラス
//

#ifndef _JDWINDOW_H
#define _JDWINDOW_H

#include <gtkmm.h>

#include "vbox.h"

namespace SKELETON
{
    class JDWindow : public Gtk::Window
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
        Gtk::Label m_mginfo;
        Gtk::ScrolledWindow m_stat_scrbar;

      public:

        JDWindow();
        ~JDWindow();

        Gtk::ScrolledWindow& get_statbar(){ return  m_stat_scrbar; }

        virtual void clock_in(){}

        void set_spacing( int space );

        const bool is_maximized() const { return m_maximized; }
        void set_maximized( bool set );

        void pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );

        void set_status( const std::string& stat );
        void set_mginfo( const std::string& mginfo );

        virtual void focus_in(){}
        virtual void focus_out(){}

      protected:

        virtual bool on_window_state_event( GdkEventWindowState* event );
    };
}

#endif
