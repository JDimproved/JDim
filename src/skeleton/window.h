// ライセンス: GPL2

//
// Window クラス
//

#ifndef _JDWINDOW_H
#define _JDWINDOW_H

#include <gtkmm.h>
#include "gtkmmversion.h"

#include "vbox.h"

namespace SKELETON
{
    class JDWindow : public Gtk::Window
    {
        GtkWidget* m_gtkwidget;
        GtkWindow *m_gtkwindow;
        gpointer m_grand_parent_class;

        bool m_win_moved;

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
        std::string m_status;

#if GTKMM_CHECK_VERSION(2,5,0)
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
        Gtk::EventBox m_label_stat_ebox;
        Gtk::EventBox m_mginfo_ebox;
#if !GTKMM_CHECK_VERSION(2,12,0)
        Gtk::Tooltips m_tooltip;
#endif
#else
        Gtk::Statusbar m_statbar;
#endif
        Gtk::Label m_mginfo;

      public:

        JDWindow( const bool fold_when_focusout, const bool need_mginfo = true );
        ~JDWindow();

        Gtk::HBox& get_statbar(){ return  m_statbar; }

        virtual void clock_in();

        // 最大、最小化
        void maximize_win();
        void unmaximize_win();
        void iconify_win();

        void set_spacing( int space );

        // hide 中
        bool is_hide();

        // 起動中
        bool is_booting() const { return m_boot; }

        void pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );

        void set_status( const std::string& stat );
        void set_status_temporary( const std::string& stat );
        void restore_status();
        std::string get_status(){ return m_status; }
        void set_mginfo( const std::string& mginfo );

        // ステータスの色を変える
        void set_status_color( const std::string& color );

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

        virtual int get_x_win() = 0;
        virtual int get_y_win() = 0;
        virtual void set_x_win( const int x ) = 0;
        virtual void set_y_win( const int y ) = 0;

        virtual int get_width_win() = 0;
        virtual int get_height_win() = 0;
        virtual void set_width_win( const int width ) = 0;
        virtual void set_height_win( const int height ) = 0;

        virtual bool is_focus_win() = 0;
        virtual void set_focus_win( const bool set ) = 0;

        virtual bool is_maximized_win() = 0;
        virtual void set_maximized_win( const bool set ) = 0;

        virtual bool is_iconified_win() = 0;
        virtual void set_iconified_win( const bool set ) = 0;

        virtual bool is_full_win() = 0;
        virtual void set_full_win( const bool set ) = 0;

        virtual bool is_shown_win() = 0;
        virtual void set_shown_win( const bool set ) = 0;

        bool on_focus_in_event( GdkEventFocus* event ) override;
        bool on_focus_out_event( GdkEventFocus* event ) override;
        bool on_delete_event( GdkEventAny* event ) override;
        bool on_window_state_event( GdkEventWindowState* event ) override;
        bool on_configure_event( GdkEventConfigure* event ) override;
        bool on_key_press_event( GdkEventKey* event ) override;

      private:

        bool slot_idle();

        void move_win( const int x, const int y );
        void set_win_pos();
    };
}

#endif
