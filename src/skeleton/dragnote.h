// ライセンス: GPL2
//
// タブをドラッグしてページを入れ替え可能なNoteBook
//
// DragableNoteBook は 下の3つの notebook から構成されている
//
// タブ : TabNotebook
// ツールバー : ToolBarNotebook
// ビュー : ViewNotebook

#ifndef _DRAGNOTE_H
#define _DRAGNOTE_H

#include "tabnote.h"
#include "toolbarnote.h"
#include "viewnote.h"
#include "tabswitchbutton.h"

#include "tooltip.h"

#include "control/control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class View;
    class ToolBar;
    class TabLabel;
    class IconPopup;

    typedef sigc::signal< void, GtkNotebookPage*, int > SIG_SWITCH_PAGE;
    typedef sigc::signal< void, int > SIG_TAB_CLICKED;
    typedef sigc::signal< void, int > SIG_TAB_CLOSE;
    typedef sigc::signal< void, int > SIG_TAB_RELOAD;
    typedef sigc::signal< void, int, int , int > SIG_TAB_MENU;
    typedef sigc::signal< void, GdkEventScroll* > SIG_TAB_SCROLLED;

    typedef sigc::signal< void, Gtk::SelectionData&, const int > SIG_DRAG_DATA_GET;

    // DragableNoteBook を構成している各Notebookの高さ
    // 及びタブの高さや位置の情報
    struct Alloc_NoteBook
    {
        int y_tab;
        int x_tab;
        int height_tab;
        int width_tab;

        int y_tabbar;
        int height_tabbar;

        int y_toolbar;
        int height_toolbar;

        int x_box;
        int y_box;
        int width_box;
        int height_box;
    };

    class DragableNoteBook : public Gtk::VBox
    {
        SIG_SWITCH_PAGE m_sig_switch_page;
        SIG_TAB_CLICKED m_sig_tab_clicked;
        SIG_TAB_CLOSE m_sig_tab_close;
        SIG_TAB_RELOAD m_sig_tab_reload;
        SIG_TAB_MENU  m_sig_tab_menu;
        SIG_TAB_SCROLLED m_sig_tab_scrolled;

        SIG_DRAG_DATA_GET m_sig_drag_data_get;

        // DragableNoteBook は 下の3つのノートブックから出来ている
        TabNotebook m_notebook_tab;  // タブ
        ToolBarNotebook m_notebook_toolbar; // ツールバー
        ViewNotebook m_notebook_view; // ビュー

        Gtk::HBox m_hbox_tab;
        TabSwitchButton m_bt_tabswitch; // タブの切り替えボタン
#if !GTKMM_CHECK_VERSION(2,12,0)
        Gtk::Tooltips m_tooltip_tabswitch;
#endif

        bool m_show_tabs;
        bool m_show_toolbar;

        int m_page;

        // タブをドラッグ中
        bool m_dragging_tab;

        bool m_dblclick;

        // 入力コントローラ
        CONTROL::Control m_control;

        Tooltip m_tooltip;

        bool m_dragable;

        SKELETON::IconPopup* m_down_arrow;

        Alloc_NoteBook m_alloc_old;

      public:

        SIG_SWITCH_PAGE signal_switch_page(){ return m_sig_switch_page; }
        SIG_TAB_CLICKED sig_tab_clicked() { return m_sig_tab_clicked; }
        SIG_TAB_CLOSE sig_tab_close() { return m_sig_tab_close; }
        SIG_TAB_RELOAD sig_tab_reload(){ return m_sig_tab_reload; }
        SIG_TAB_MENU sig_tab_menu() { return m_sig_tab_menu; }
        SIG_TAB_SCROLLED sig_tab_scrolled(){ return m_sig_tab_scrolled; }

        SIG_DRAG_DATA_GET sig_drag_data_get() { return m_sig_drag_data_get; }

        DragableNoteBook();
        ~DragableNoteBook();

        void clock_in();
        void focus_out();

#if !GTKMM_CHECK_VERSION(3,0,0)
        // 枠描画
        void draw_box( Gtk::Widget* widget, GdkEventExpose* event );
#endif

        bool get_show_tabs() const { return m_show_tabs; }
        void set_show_tabs( bool show_tabs );
        void set_scrollable( bool scrollable );
        int get_n_pages();
        Gtk::Widget* get_nth_page( int page_num );
        int page_num( const Gtk::Widget& child );
        int get_current_page();
        void set_current_page( int page_num );

        int append_page( const std::string& url, Gtk::Widget& child );
        int insert_page( const std::string& url, Gtk::Widget& child, int page );
        void remove_page( const int page, const bool adust_tab );

        // ツールバー関係
        // 各Adminクラスの virtual void show_toolbar()でツールバーを作成してappend_toolbar()で登録する
        void show_toolbar();
        void hide_toolbar();
        void append_toolbar( Gtk::Widget& toolbar );
        void set_current_toolbar( const int id_toolbar, SKELETON::View* view );
        int get_current_toolbar();
        void focus_toolbar_search(); // ツールバー内の検索entryにフォーカスを移す
        void update_toolbar_url( std::string& url_old, std::string& url_new );
        void update_toolbar_button();

        // タブの文字列取得/セット
        const std::string& get_tab_fulltext( const int page );
        void set_tab_fulltext( const std::string& str, const int page );

        // タブのアイコン取得/セット
        int get_tabicon( const int page );
        void set_tabicon( const std::string& iconname, const int page, const int icon );

        // ドラッグ可/不可切り替え(デフォルト false );
        void set_dragable( bool dragable ){ m_dragable = dragable; }

        // タブの幅を固定するか
        void set_fixtab( bool fix );

        // タブ幅調整
        bool adjust_tabwidth();

        // タブ切り替えボタン
        Gtk::Button& get_tabswitch_button(){ return m_bt_tabswitch.get_button(); }

      private:

#if !GTKMM_CHECK_VERSION(3,0,0)
        bool on_expose_event( GdkEventExpose* event ) override;
#endif

#if !GTKMM_CHECK_VERSION(3,0,0)
        // DragableNoteBook を構成している各Notebookの高さ
        // 及びタブの高さと位置を取得 ( 枠の描画用 )
        Alloc_NoteBook get_alloc_notebook();
#endif

        // ツールバー取得
        SKELETON::ToolBar* get_toolbar( int page );

        // タブ作成
        SKELETON::TabLabel* create_tablabel( const std::string& url );

        // notebook_tabのタブが切り替わったときに呼び出されるslot
        void slot_switch_page_tab( GtkNotebookPage*, guint page );

        // notebook_tab の上でボタンを押した/離した
        bool slot_button_press_event( GdkEventButton* event );
        bool slot_button_release_event( GdkEventButton* event );

        // notebook_tab の上でホイールを回した
        bool slot_scroll_event( GdkEventScroll* event );

      protected:

        // コントローラ
        CONTROL::Control& get_control(){ return m_control; }

        // タブからくるシグナルにコネクトする
        void slot_motion_event();
        void slot_leave_event();

        void slot_drag_begin();
        void slot_drag_motion( const int page, const int tab_x, const int tab_y, const int tab_width );
        void slot_drag_data_get( Gtk::SelectionData& selection_data );
        void slot_drag_end();
    };
}

#endif
