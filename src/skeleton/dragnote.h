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
    typedef sigc::signal< void > SIG_DRAG_END;

    class DragableNoteBook : public Gtk::Notebook
    {
        SIG_TAB_CLOSE m_sig_tab_close;
        SIG_TAB_RELOAD m_sig_tab_reload;
        SIG_TAB_MENU  m_sig_tab_menu;

        SIG_DRAG_BEGIN m_sig_drag_begin;
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
        SIG_DRAG_END sig_drag_end() { return m_sig_drag_end; }

        DragableNoteBook();

        void clock_in();
        void focus_out();

        // タブ作成
        SKELETON::TabLabel* create_tablabel( const std::string& url );

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

        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );

        // タブからくるシグナルにコネクトする
        void slot_motion_event();
        void slot_leave_event();

        void slot_drag_begin();
        void slot_drag_drop();
        void slot_drag_end();
    };
}

#endif
