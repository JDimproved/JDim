// ライセンス: GPL2
//
// Paned widget の制御クラス
//

#ifndef _PANECONTROL_H
#define _PANECONTROL_H

#include <gtkmm.h>

namespace SKELETON
{
    enum
    {
        PANE_NORMAL = 0,
        PANE_MAX_PAGE1, // PAGE1 最大化
        PANE_MAX_PAGE2  // PAGE2 最大化
    };

    enum
    {
        PANE_CLICK_NORMAL,       // セパレータクリックで折り畳まない
        PANE_CLICK_FOLD_PAGE1,   // セパレータクリックでPAGE1を折り畳む
        PANE_CLICK_FOLD_PAGE2    // セパレータクリックでPAGE2を折り畳む
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

        int m_mode;
        int m_pos;

        int m_pre_size;

      public:

        PaneControl( Gtk::Paned& paned );
        virtual ~PaneControl();

        SIG_PANE_MODECHANGED& sig_pane_modechanged() { return m_sig_pane_modechanged; }

        void clock_in();

        // セパレータの位置の取得やセット
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


      protected:

        Gtk::Paned& get_paned(){ return m_paned; }
        virtual int get_size() = 0;
    };


    /////////////////////////////////////

    class HPaneControl : public PaneControl
    {
      public:

        HPaneControl( Gtk::Paned& paned ) : PaneControl( paned ) {}
        ~HPaneControl(){}

      protected:
        virtual int get_size(){ return get_paned().get_width(); }
    };


    /////////////////////////////////////

    class VPaneControl : public PaneControl
    {
      public:

        VPaneControl( Gtk::Paned& paned ) : PaneControl( paned ) {}
        ~VPaneControl(){}

      protected:
        virtual int get_size(){ return get_paned().get_height(); }
    };


}

#endif

