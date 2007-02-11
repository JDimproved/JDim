// ライセンス: GPL2

//
// 画像ウィンドウ
//

#ifndef _IMAGEWIN_H
#define _IMAGEWIN_H

#include <gtkmm.h>

#include "skeleton/vbox.h"

namespace IMAGE
{
    class ImageWin : public Gtk::Window
    {
        bool m_boot;
        bool m_maximized;

        int m_mode;

        int m_x;
        int m_y;
        int m_width;
        int m_height;

        SKELETON::JDVBox m_vbox;
        Gtk::ScrolledWindow m_scrwin;
        Gtk::Widget* m_tab;

      public:

        ImageWin();
        ~ImageWin();

        // 起動中
        const bool is_booting() const { return m_boot; }

        // フォーカス状態
        const bool has_focus();

        void pack_remove( bool unpack, Gtk::Widget& tab, Gtk::Widget& view );

        void focus_in();
        void focus_out();

        void show_win();
        void hide_win();

      protected:

        virtual bool on_expose_event( GdkEventExpose* event );

        virtual bool on_focus_in_event( GdkEventFocus* event );
        virtual bool on_focus_out_event( GdkEventFocus* event );

        virtual bool on_delete_event( GdkEventAny* event );
        virtual bool on_window_state_event( GdkEventWindowState* event );
        virtual bool on_configure_event( GdkEventConfigure* event );

      private:

        // ウィンドウを折り畳んだときの高さ
        int get_min_height();
    };
}


#endif
