// ライセンス: GPL2
//
// タブをドラッグしてページを入れ替え可能なNoteBook


#ifndef _DRAGNOTE_H
#define _DRAGNOTE_H

#include "tabnote.h"
#include "tooltip.h"

#include "control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class View;
    class ToolBar;
    class TabLabel;
    class IconPopup;

    typedef sigc::signal< void, GtkNotebookPage*, int > SIG_SWITCH_PAGE;
    typedef sigc::signal< void, int > SIG_TAB_CLICK;
    typedef sigc::signal< void, int > SIG_TAB_CLOSE;
    typedef sigc::signal< void, int > SIG_TAB_RELOAD;
    typedef sigc::signal< void, int, int , int > SIG_TAB_MENU;

    // D&D
    typedef sigc::signal< void, int > SIG_DRAG_BEGIN;
    typedef sigc::signal< void > SIG_DRAG_END;


    class DragableNoteBook : public Gtk::VBox
    {
        SIG_SWITCH_PAGE m_sig_switch_page;
        SIG_TAB_CLICK m_sig_tab_click;
        SIG_TAB_CLOSE m_sig_tab_close;
        SIG_TAB_RELOAD m_sig_tab_reload;
        SIG_TAB_MENU  m_sig_tab_menu;

        SIG_DRAG_BEGIN m_sig_drag_begin;
        SIG_DRAG_END m_sig_drag_end;

        TabNotebook m_notebook_tab;
        Gtk::Notebook m_notebook_toolbar;
        Gtk::Notebook m_notebook_view;

        bool m_show_tabs;

        int m_page;
        bool m_drag;
        bool m_dblclick;

        // 入力コントローラ
        CONTROL::Control m_control;

        Tooltip m_tooltip;

        bool m_dragable;

        SKELETON::IconPopup* m_down_arrow;

      public:

        SIG_SWITCH_PAGE signal_switch_page(){ return m_sig_switch_page; }
        SIG_TAB_CLICK sig_tab_click() { return m_sig_tab_click; }
        SIG_TAB_CLOSE sig_tab_close() { return m_sig_tab_close; }
        SIG_TAB_RELOAD sig_tab_reload(){ return m_sig_tab_reload; }
        SIG_TAB_MENU sig_tab_menu() { return m_sig_tab_menu; }

        SIG_DRAG_BEGIN sig_drag_begin() { return m_sig_drag_begin; }
        SIG_DRAG_END sig_drag_end() { return m_sig_drag_end; }

        DragableNoteBook();
        virtual ~DragableNoteBook();

        void clock_in();
        void focus_out();

        const bool get_show_tabs() const{ return m_show_tabs; }
        void set_show_tabs( bool show_tabs );
        void set_scrollable( bool scrollable );
        const int get_n_pages();
        Gtk::Widget* get_nth_page( int page_num );
        const int page_num( const Gtk::Widget& child );
        const int get_current_page();
        void set_current_page( int page_num );

        int append_page( const std::string& url, Gtk::Widget& child );
        int insert_page( const std::string& url, Gtk::Widget& child, int page );
        void remove_page( int page );

        // ツールバー関係
        void show_toolbar();
        void hide_toolbar();
        void append_toolbar( Gtk::Widget& toolbar );
        void set_current_toolbar( int page_num, SKELETON::View* view );
        const int get_current_toolbar();
        void focus_toolbar_search(); // ツールバー内の検索entryにフォーカスを移す
        void update_toolbar_url( std::string& url_old, std::string& url_new );
        void update_toolbar_label( SKELETON::View* view );
        void update_toolbar_close_button( SKELETON::View* view );
        void update_toolbar_button();

        // タブの文字列取得/セット
        const std::string get_tab_fulltext( int page );
        void set_tab_fulltext( const std::string& str, int page );

        // タブにアイコンをセットする
        void set_tabicon( const std::string& iconname, const int page, const int icon );

        // ドラッグ可/不可切り替え(デフォルト false );
        void set_dragable( bool dragable ){ m_dragable = dragable; }

        // タブの幅を固定するか
        void set_fixtab( bool fix );

        // タブ幅調整
        bool adjust_tabwidth();

      private:

        // ツールバー取得
        SKELETON::ToolBar* get_toolbar( int page );

        // タブ作成
        SKELETON::TabLabel* create_tablabel( const std::string& url );

        // notebook_tabのタブが切り替わったときに呼び出されるslot
        void slot_switch_page_tab( GtkNotebookPage*, guint page );

        // notebook_tab の上でボタンを押した/離した
        bool slot_button_press_event( GdkEventButton* event );
        bool slot_button_release_event( GdkEventButton* event );

      protected:

        // コントローラ
        CONTROL::Control& get_control(){ return m_control; }

        // タブからくるシグナルにコネクトする
        void slot_motion_event();
        void slot_leave_event();

        void slot_drag_begin();
        void slot_drag_motion( const int page, const int tab_x, const int tab_y, const int tab_width );
        void slot_drag_end();
    };
}

#endif
