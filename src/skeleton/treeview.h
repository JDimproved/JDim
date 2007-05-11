// ライセンス: GPL2
//
// treeviewクラス
//

#ifndef _TREEVIEW_H
#define _TREEVIEW_H

#include "tooltip.h"

#include "control.h"

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class View;
    class PopupWin;

    class JDTreeView : public Gtk::TreeView
    {
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;
        typedef sigc::signal< bool, GdkEventScroll* > SIG_SCROLL_EVENT;
        typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;
        typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_RELEASE;
        typedef sigc::signal< bool, GdkEventMotion* > SIG_MOTION_NOTIFY;

        typedef sigc::signal< void > SIG_DRAG_BEGIN;
        typedef sigc::signal< void, Gtk::TreeModel::Path > SIG_DRAG_MOTION;
        typedef sigc::signal< void, Gtk::TreeModel::Path > SIG_DRAG_DROP;
        typedef sigc::signal< void > SIG_DRAG_END;


        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;
        SIG_SCROLL_EVENT m_sig_scroll_event;
        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_BUTTON_RELEASE m_sig_button_release;
        SIG_MOTION_NOTIFY m_sig_motion_notify;

        SIG_DRAG_BEGIN m_sig_drag_begin;
        SIG_DRAG_MOTION m_sig_drag_motion;
        SIG_DRAG_DROP m_sig_drag_drop;
        SIG_DRAG_END m_sig_drag_end;

        bool m_reorderable;
        bool m_drag;
        bool m_selection_canceled;
        Gtk::TreeModel::Path m_path_dragstart;
        Gtk::TreeModel::Path m_path_dragpre;

        Tooltip m_tooltip;

        // 色
        bool m_use_bg_even;
        Gdk::Color m_color_text;
        Gdk::Color m_color_bg;
        Gdk::Color m_color_bg_even;

        // ポップアップウィンドウ用
        PopupWin* m_popup_win;
        std::string m_pre_popup_url;

        // get_row_height() で高さを取得するためのcolumn番号
        int m_column_for_height;
        
        // 入力コントローラ
        CONTROL::Control m_control;

      public:

        SIG_KEY_PRESS sig_key_press() { return m_sig_key_press; }
        SIG_KEY_RELEASE sig_key_release() { return m_sig_key_release; }
        SIG_SCROLL_EVENT sig_scroll_event(){ return m_sig_scroll_event; }
        SIG_BUTTON_PRESS sig_button_press() { return m_sig_button_press; }
        SIG_BUTTON_RELEASE sig_button_release() { return m_sig_button_release; }
        SIG_MOTION_NOTIFY sig_motion_notify() { return m_sig_motion_notify; }

        SIG_DRAG_BEGIN sig_drag_begin() { return m_sig_drag_begin; }
        SIG_DRAG_MOTION sig_drag_motion() { return m_sig_drag_motion; }
        SIG_DRAG_DROP sig_drag_drop() { return m_sig_drag_drop; }
        SIG_DRAG_END sig_drag_end() { return m_sig_drag_end; }

        JDTreeView( const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even );
        ~JDTreeView();

       
        const bool reorderable() const { return m_reorderable; }
        void clock_in();

        // 色初期化
        void init_color( const int colorid_text, const int colorid_bg, const int colorid_bg_even );

        // フォント初期化
        void init_font( const std::string& fontname );
        
        // D&D可で並び替え可
        void set_reorderable_view( bool reorderable );

        // 現在フォーカスしてる行の最初のパスを取得
        Gtk::TreeModel::Path get_current_path();

        //x, y 座標の下のパスを取得
        Gtk::TreeModel::Path get_path_under_xy( int x, int y );

        // 現在のマウスポインタの下のパスを取得
        Gtk::TreeModel::Path get_path_under_mouse();

        // 現在のマウスポインタの下のセルの幅高さとセル内での座標を取得        
        void get_cell_xy_wh( int& cell_x, int& cell_y, int& cell_w, int& cell_h );

        // 選択中の Gtk::TreeModel::iterator のリストを取得
        std::list< Gtk::TreeModel::iterator > get_selected_iterators();

        // ツールチップ表示
        // set_tooltip_min_width()で指定した幅よりもツールチップが広い場合は表示
        void set_str_tooltip( const std::string& str );
        void set_tooltip_min_width( const int& min_width );
        void hide_tooltip();
        void show_tooltip();

        // ポップアップウィンドウ表示
        const std::string& pre_popup_url() const { return m_pre_popup_url; }
        void show_popup( const std::string& url, View* view );
        void delete_popup();

        // 選択行の移動
        void goto_top();
        void goto_bottom();
        void row_up();
        void row_down();
        void page_up();
        void page_down();

        // path の前後のpathを取得
        Gtk::TreePath prev_path( const Gtk::TreePath& path, bool check_expand = true );
        Gtk::TreePath next_path( const Gtk::TreePath& path, bool check_expand = true );

        // path -> row 変換
        Gtk::TreeModel::Row get_row( const Gtk::TreePath& path );

        // pathの親を再起的にexpandする
        void expand_parents( const Gtk::TreePath& path );

        // 行のセルの高さ
        int get_row_height();
        void set_column_for_height( int column ){ m_column_for_height = column; }

        // マウスホイールの処理
        void wheelscroll( GdkEventScroll* event );

        // 実際の描画の際に cellrenderer のプロパティをセットするスロット関数
        void slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );

      protected:

        // drag_source_set() でセットしたボタンでドラッグした時に呼び出される順番は
        //
        // (1) on_button_press_event
        // (2) on_drag_begin
        // (3) on_drag_motion
        // (4) on_button_release_event
        // (5) on_drag_drop
        // (6) on_drag_end
        //
        // drag_source_set() でセットしたボタン以外でドラッグしたときは on_drag_motion()
        // ではなくて普通に on_motion_notify_event() が呼ばれるのに注意
        //
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual void on_drag_begin( const Glib::RefPtr< Gdk::DragContext>& context );
        virtual bool on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context );

        virtual bool on_key_press_event( GdkEventKey* event );
        virtual bool on_key_release_event( GdkEventKey* event );

        virtual bool on_motion_notify_event( GdkEventMotion* event );
        virtual bool on_scroll_event( GdkEventScroll* event );
        virtual bool on_leave_notify_event( GdkEventCrossing* event );

      private:
        void slot_selection_changed();
   };
}

#endif
