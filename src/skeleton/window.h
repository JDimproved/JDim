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
        // フォーカスアウト時の折りたたみ処理で用いるメンバ変数
        bool m_fold_when_focusout; // フォーカスアウトしたときにウィンドウを畳むか
        bool m_boot;
        bool m_enable_fold; // 「一時的に」折りたたみ可能かどうか切り替える
        bool m_transient;
        int m_mode;
        int m_counter;
        int m_count_focusout; // フォーカス制御用カウンタ
        Gtk::Window* m_dummywin; // set_transient()で使うダミーwindow

        SKELETON::JDVBox m_vbox;

        Gtk::ScrolledWindow* m_scrwin;
        SKELETON::JDVBox* m_vbox_view;

        // ステータスバー
#if GTKMMVER <= 240
        Gtk::Statusbar m_statbar;
#else
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
#endif
        Gtk::Label m_mginfo;

      public:

        JDWindow( const bool fold_when_focusout, const bool need_mginfo = true );
        ~JDWindow();

        Gtk::HBox& get_statbar(){ return  m_statbar; }

        virtual void clock_in();

        void set_spacing( int space );

        // hide 中
        const bool is_hide();

        // 起動中
        const bool is_booting() const { return m_boot; }

        void pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );

        void set_status( const std::string& stat );
        void set_mginfo( const std::string& mginfo );

        // メインウィンドウに対して transient 設定
        void set_transient( bool set );

        // ダイアログ表示などでフォーカスが外れてもウインドウを畳まないようにする
        void set_enable_fold( bool enable );

        virtual void focus_in();
        virtual void focus_out();

      protected:

        SKELETON::JDVBox& get_vbox(){ return m_vbox;}

        // windowの初期設定(サイズ変更や移動など)
        void init_win();

        virtual void switch_admin(){}

        virtual const int get_x_win() = 0;
        virtual const int get_y_win() = 0;
        virtual void set_x_win( int x ) = 0;
        virtual void set_y_win( int y ) = 0;

        virtual const int get_width_win() = 0;
        virtual const int get_height_win() = 0;
        virtual void set_width_win( int width ) = 0;
        virtual void set_height_win( int height ) = 0;

        virtual const bool is_focus_win() = 0;
        virtual void set_focus_win( bool set ) = 0;

        virtual const bool is_maximized_win() = 0;
        virtual void set_maximized_win( bool set ) = 0;

        virtual const bool is_iconified_win() = 0;
        virtual void set_iconified_win( bool set ) = 0;

        virtual const bool is_shown_win() = 0;
        virtual void set_shown_win( bool set ) = 0;

        virtual bool on_focus_in_event( GdkEventFocus* event );
        virtual bool on_focus_out_event( GdkEventFocus* event );
        virtual bool on_delete_event( GdkEventAny* event );
        virtual bool on_window_state_event( GdkEventWindowState* event );
        virtual bool on_configure_event( GdkEventConfigure* event );

      private:

        // 最大化する
        void maximize_win();

        bool slot_idle();
    };
}

#endif
