// ライセンス: GPL2
//
// Paned widget の制御クラス
//

#ifndef _PANECONTROL_H
#define _PANECONTROL_H

#include <gtkmm.h>

namespace SKELETON
{
    // コンストラクタで指定する m_fixmode の値
    enum
    {
        PANE_FIXSIZE_PAGE1, // ウィンドウリサイズ時にPAGE1のサイズを固定する
        PANE_FIXSIZE_PAGE2  // ウィンドウリサイズ時にPAGE2のサイズを固定する
    };

    // set_mode()で指定する m_mode の値
    enum
    {
        PANE_NORMAL = 0, // どちらのページも折り畳んでいない通常状態
        PANE_MAX_PAGE1, // PAGE1 最大化中
        PANE_MAX_PAGE2  // PAGE2 最大化中
    };

    // set_click_fold()で指定する m_click_fold の値
    enum
    {
        PANE_CLICK_NORMAL,       // セパレータをクリックしても折り畳まない
        PANE_CLICK_FOLD_PAGE1,   // セパレータをクリックするとPAGE1を折り畳む
        PANE_CLICK_FOLD_PAGE2    // セパレータをクリックするとPAGE2を折り畳む
    };

    // ペーン表示が切り替えられた
    typedef sigc::signal< void, int > SIG_PANE_MODECHANGED;

    class PaneControl
    {
        SIG_PANE_MODECHANGED m_sig_pane_modechanged;
        Gtk::Paned& m_paned;

        int m_click_fold;

        bool m_clicked;
        bool m_drag;
        bool m_on_paned;

        int m_fixmode;
        int m_mode;
        int m_pos;

        int m_pre_size;

      public:

        PaneControl( Gtk::Paned& paned, int fixmode );
        virtual ~PaneControl() noexcept;

        SIG_PANE_MODECHANGED& sig_pane_modechanged() { return m_sig_pane_modechanged; }

        void clock_in();

        // セパレータの位置の取得やセット
        void update_position();
        int get_position();
        void set_position( int position );

        // PANE_MAX_PAGE1 などを指定
        void set_mode( int mode );
        const int get_mode() const { return m_mode; }

        // PANE_CLICK_FOLD_PAGE1 などを指定
        void set_click_fold( int mode ){ m_click_fold = mode; }
        const int get_click_fold() const { return m_click_fold; }

        void add_remove1( bool unpack, Gtk::Widget& child );
        void add_remove2( bool unpack, Gtk::Widget& child );

        void button_press_event( GdkEventButton* event );
        void button_release_event( GdkEventButton* event );
        void motion_notify_event( GdkEventMotion* event );
        void enter_notify_event( GdkEventCrossing* event );
        void leave_notify_event( GdkEventCrossing* event );

      protected:

        const bool is_on_paned() const { return m_on_paned; }
        Gtk::Paned& get_paned(){ return m_paned; }
        virtual int get_size() = 0;
        virtual bool is_separater_clicked( GdkEventButton* event ) = 0;
    };


    /////////////////////////////////////

    class HPaneControl : public PaneControl
    {
      public:

        HPaneControl( Gtk::Paned& paned, int fixmode ) : PaneControl( paned, fixmode ) {}
        ~HPaneControl() noexcept {}

      protected:

        int get_size() override { return get_paned().get_width(); }

        bool is_separater_clicked( GdkEventButton* event ) override
        {
            if( is_on_paned() && event->type == GDK_BUTTON_PRESS && event->button == 1
                && event->x >= 0 && event->x <= 8 ) return true;
            return false;
        }
    };


    /////////////////////////////////////

    class VPaneControl : public PaneControl
    {
      public:

        VPaneControl( Gtk::Paned& paned, int fixmode ) : PaneControl( paned, fixmode ) {}
        ~VPaneControl() noexcept {}

      protected:

        int get_size() override { return get_paned().get_height(); }

        bool is_separater_clicked( GdkEventButton* event ) override
        {
            if( is_on_paned() && event->type == GDK_BUTTON_PRESS && event->button == 1
                && event->y >= 0 && event->y <= 8 ) return true;
            return false;
        }
    };
}

#endif

