// ライセンス: GPL2

//
// Window クラス
//

#ifndef _JDWINDOW_H
#define _JDWINDOW_H

#include "gtkmmversion.h"

#include "vbox.h"

#include <gtkmm.h>

#include <memory>


namespace SKELETON
{
    class JDWindow : public Gtk::Window
    {
        GtkWidget* m_gtkwidget{};
        GtkWindow* m_gtkwindow{};
        gpointer m_grand_parent_class{};

        bool m_win_moved{};

        // フォーカスアウト時の折りたたみ処理で用いるメンバ変数
        bool m_fold_when_focusout; // フォーカスアウトしたときにウィンドウを畳むか
        bool m_boot;
        bool m_enable_fold; // 「一時的に」折りたたみ可能かどうか切り替える
        bool m_transient{};
        int m_mode;
        int m_counter{};
        int m_count_focusout{}; // フォーカス制御用カウンタ

        SKELETON::JDVBox m_vbox;

        std::unique_ptr<Gtk::ScrolledWindow> m_scrwin;
        std::unique_ptr<SKELETON::JDVBox> m_vbox_view;

        // ステータスバー
        std::string m_status;

        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
        Gtk::EventBox m_label_stat_ebox;
        Gtk::EventBox m_mginfo_ebox;
        Gtk::Label m_mginfo;

        static constexpr const char* s_css_stat_label = "jd-stat-label";
        Glib::RefPtr< Gtk::CssProvider > m_stat_provider = Gtk::CssProvider::create();

      public:

        explicit JDWindow( const bool fold_when_focusout, const bool need_mginfo = true );
        ~JDWindow() noexcept override = default;

        Gtk::HBox& get_statbar(){ return  m_statbar; }

        virtual void clock_in();

        // 最大、最小化
        void maximize_win();
        void unmaximize_win();
        void iconify_win();

        void set_spacing( int space );

        // hide 中
        bool is_hide() const;

        // 起動中
        bool is_booting() const { return m_boot; }

        void pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );

        void set_status( const std::string& stat );
        void set_status_temporary( const std::string& stat );
        void restore_status();
        const std::string& get_status() const noexcept { return m_status; }
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

        virtual int get_x_win() const = 0;
        virtual int get_y_win() const = 0;
        virtual void set_x_win( const int x ) = 0;
        virtual void set_y_win( const int y ) = 0;

        virtual int get_width_win() const = 0;
        virtual int get_height_win() const = 0;
        virtual void set_width_win( const int width ) = 0;
        virtual void set_height_win( const int height ) = 0;

        virtual bool is_focus_win() const = 0;
        virtual void set_focus_win( const bool set ) = 0;

        virtual bool is_maximized_win() const = 0;
        virtual void set_maximized_win( const bool set ) = 0;

        virtual bool is_iconified_win() const = 0;
        virtual void set_iconified_win( const bool set ) = 0;

        virtual bool is_full_win() const = 0;
        virtual void set_full_win( const bool set ) = 0;

        virtual bool is_shown_win() const = 0;
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
