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
        bool m_enable_fold;
        bool m_transient;

        bool m_maximized; // 最大化されている

        int m_mode;
        int m_counter;

        int m_count_focusout; // フォーカス制御用カウンタ

        int m_x;
        int m_y;
        int m_width;
        int m_height;

        SKELETON::JDVBox m_vbox;
        SKELETON::JDVBox m_vbox_view;
        Gtk::ScrolledWindow m_scrwin;
        Gtk::Widget* m_tab;

        Gtk::Window m_dummywin; // set_transient()で使うダミーwindow

        // ステータスバー
#if GTKMMVER <= 240
        Gtk::Statusbar m_statbar;
#else
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
#endif

      public:

        ImageWin();
        ~ImageWin();

        void clock_in();

        // 起動中
        const bool is_booting() const { return m_boot; }

        // hide 中
        const bool is_hide();

        // ダイアログ表示などでフォーカスが外れてもウインドウを畳まないようにする
        void set_enable_fold( bool enable );

        void set_transient( bool set );
        void pack_remove( bool unpack, Gtk::Widget& tab, Gtk::Widget& view );

        void set_status( const std::string& stat );

        void focus_in();
        void focus_out();

      protected:

        virtual bool on_focus_in_event( GdkEventFocus* event );
        virtual bool on_focus_out_event( GdkEventFocus* event );

        virtual bool on_delete_event( GdkEventAny* event );
        virtual bool on_window_state_event( GdkEventWindowState* event );
        virtual bool on_configure_event( GdkEventConfigure* event );

      private:

        bool slot_idle();

        // ウィンドウを折り畳んだときの高さ
        int get_min_height();
    };
}


#endif
