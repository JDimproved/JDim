// ライセンス: GPL2
//
// DragableNoteBookを構成するタブ表示用の Notebook
//

#ifndef _TABNOTE_H
#define _TABNOTE_H

#include "gtkmmversion.h"

#include <gtkmm.h>

#if GTKMM_CHECK_VERSION(3,0,0)
using GtkNotebookPage = Gtk::Widget;
#endif

namespace SKELETON
{
    class DragableNoteBook;
    class TabLabel;

    struct Alloc_NoteBook;

    // タブのクリック
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_RELEASE;

    // マウス移動
    typedef sigc::signal< void > SIG_TAB_MOTION_EVENT;
    typedef sigc::signal< void > SIG_TAB_LEAVE_EVENT;

    // ホイール回転
    typedef sigc::signal< bool, GdkEventScroll* > SIG_SCROLL_EVENT;

    // D&D
    typedef sigc::signal< void, const int, const int, const int, const int > SIG_TAB_DRAG_MOTION;

    // タブ用の Notebook
    class TabNotebook : public Gtk::Notebook
    {
        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_BUTTON_RELEASE m_sig_button_release;

        SIG_TAB_MOTION_EVENT m_sig_tab_motion_event;
        SIG_TAB_LEAVE_EVENT m_sig_tab_leave_event;

        SIG_TAB_DRAG_MOTION m_sig_tab_drag_motion;

        SIG_SCROLL_EVENT m_sig_scroll_event;

        DragableNoteBook* m_parent;

        // タブ幅計算用layout
        Glib::RefPtr< Pango::Layout > m_layout_tab;

        int m_pre_width;
        bool m_fixtab;
        int m_tab_mrg;

        const std::string m_str_empty;

    public:

        SIG_BUTTON_PRESS sig_button_press(){ return m_sig_button_press; }
        SIG_BUTTON_RELEASE sig_button_release(){ return m_sig_button_release; }

        SIG_TAB_MOTION_EVENT sig_tab_motion_event(){ return  m_sig_tab_motion_event; }
        SIG_TAB_LEAVE_EVENT sig_tab_leave_event(){ return m_sig_tab_leave_event; }

        SIG_TAB_DRAG_MOTION sig_tab_drag_motion(){ return m_sig_tab_drag_motion; }

        SIG_SCROLL_EVENT sig_scroll_event(){ return m_sig_scroll_event; }

        TabNotebook( DragableNoteBook* parent );

        void clock_in();

        // タブの幅を固定するか(デフォルト false );
        void set_fixtab( bool fix ){ m_fixtab = fix; }

        int append_tab( Widget& tab );
        int insert_tab( Widget& tab, int page );
        void remove_tab( const int page, const bool adjust_tab );
        void reorder_child( int page1, int page2 );

        // タブ取得
        SKELETON::TabLabel* get_tablabel( int page );

        // タブの文字列取得/セット
        const std::string& get_tab_fulltext( int page );
        void set_tab_fulltext( const std::string& str, int page );

        // タブ幅調整
        bool adjust_tabwidth();

        // マウスの下にあるタブの番号を取得
        // タブ上では無いときは-1を返す
        // マウスがタブの右側にある場合はページ数の値を返す
        int get_page_under_mouse();

        // タブの高さ、幅、位置を取得 ( 描画用 )
        void get_alloc_tab( Alloc_NoteBook& alloc );

      private:

#if !GTKMM_CHECK_VERSION(3,0,0)
        // gtknotebook.c ( Revision 19311, 2008-01-06 ) を参考にして作成した描画関係の関数
        bool paint( GdkEventExpose* event );

        void draw_tab( const GtkNotebook *notebook,
                       const GtkNotebookPage *page,
                       GdkRectangle *area,
                       const Gdk::Rectangle& rect,
                       const Glib::RefPtr< Gdk::Window >& win
            );

        void draw_arrow( GtkWidget *widget,
                         const GtkNotebook *notebook,
                         const Gdk::Rectangle& rect,
                         const Glib::RefPtr< Gdk::Window >& win,
                         const int nbarrow );

        void get_arrow_rect( GtkWidget *widget, const GtkNotebook *notebook, GdkRectangle *rectangle, const gboolean before );
        gboolean get_event_window_position( const GtkWidget *widget, const GtkNotebook *notebook, GdkRectangle *rectangle );
#endif // !GTKMM_CHECK_VERSION(3,0,0)


        // 各タブのサイズと座標を取得
        void calc_tabsize();

      protected:

#if !GTKMM_CHECK_VERSION(3,0,0)
        bool on_expose_event( GdkEventExpose* event ) override;
#endif
        void on_size_allocate( Gtk::Allocation& allocation ) override;

        // signal_button_press_event と signal_button_release_event は emit されない
        // ときがあるので自前でemitする
        bool on_button_press_event( GdkEventButton* event ) override;
        bool on_button_release_event( GdkEventButton* event ) override;

        bool on_motion_notify_event( GdkEventMotion* event ) override;
        bool on_leave_notify_event( GdkEventCrossing* event ) override;
        bool on_scroll_event( GdkEventScroll* event ) override;

        bool on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) override;
    };
}

#endif
