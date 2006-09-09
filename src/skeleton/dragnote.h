// ライセンス: 最新のGPL
//
// タブをドラッグしてページを入れ替え可能なNoteBook


#ifndef _DRAGNOTE_H
#define _DRAGNOTE_H

#include "tooltip.h"

#include "control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class TabLabel;

    typedef sigc::signal< void, int > SIG_TAB_CLOSE;
    typedef sigc::signal< void, int > SIG_TAB_RELOAD;
    typedef sigc::signal< void, int, int , int > SIG_TAB_MENU;

    // D&D
    typedef sigc::signal< void, int > SIG_DRAG_BEGIN;
    typedef sigc::signal< void > SIG_DRAG_MOTION;
    typedef sigc::signal< void, int > SIG_DRAG_DROP;
    typedef sigc::signal< void > SIG_DRAG_END;

    class DragableNoteBook : public Gtk::Notebook
    {
        SIG_TAB_CLOSE m_sig_tab_close;
        SIG_TAB_RELOAD m_sig_tab_reload;
        SIG_TAB_MENU  m_sig_tab_menu;

        SIG_DRAG_BEGIN m_sig_drag_begin;
        SIG_DRAG_MOTION m_sig_drag_motion;
        SIG_DRAG_DROP m_sig_drag_drop;
        SIG_DRAG_END m_sig_drag_end;

        int m_page;
        bool m_drag;
        bool m_dblclick;

        // 入力コントローラ
        CONTROL::Control m_control;

        Tooltip m_tooltip;

        bool m_adjust_reserve; // adjust予約
        int m_pre_width;

      public:

        SIG_TAB_CLOSE sig_tab_close() { return m_sig_tab_close; }
        SIG_TAB_RELOAD sig_tab_reload(){ return m_sig_tab_reload; }
        SIG_TAB_MENU sig_tab_menu() { return m_sig_tab_menu; }

        SIG_DRAG_BEGIN sig_drag_begin() { return m_sig_drag_begin; }
        SIG_DRAG_MOTION sig_drag_motion() { return m_sig_drag_motion; }
        SIG_DRAG_DROP sig_drag_drop() { return m_sig_drag_drop; }
        SIG_DRAG_END sig_drag_end() { return m_sig_drag_end; }

        DragableNoteBook();

        void clock_in();
        void focus_out();

        // タブ取得
        TabLabel* get_tablabel( int page );
        TabLabel* get_tablabel( const std::string& url );

        void set_dragable( bool dragable );

        // マウスの下にあるタブの番号
        int get_page_under_mouse();

        // タブ幅調整
        bool adjust_tabwidth( bool force );

      protected:

        // コントローラ
        CONTROL::Control& get_control(){ return m_control; }

        // 呼び出される順番は
        //
        // (1) on_button_press_event
        // (2) on_drag_begin
        // (3) on_button_release_event
        // (4) on_drag_drop
        // (5) on_drag_end
        //
        virtual void on_drag_begin( const Glib::RefPtr< Gdk::DragContext>& context );
        virtual bool on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual bool on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context );

        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );

        virtual bool on_motion_notify_event( GdkEventMotion* event );
        virtual bool on_leave_notify_event( GdkEventCrossing* event );
    };
}

#endif
