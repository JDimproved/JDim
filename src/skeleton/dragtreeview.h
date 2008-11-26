// ライセンス: GPL2
//
// ドラッグ開始可能なtreeviewクラス ( ドロップは受け付けない )
//
// フォント変更、偶数、奇数行別に色分けも可能。コンストラクタの use_usr_fontcolor を trueにしてフォントや色を指定する
// 複数行選択、ツールチップ、ポップアップの表示も可能
//

#ifndef _DRAGTREEVIEW_H
#define _DRAGTREEVIEW_H

#include "treeviewbase.h"

#include "tooltip.h"

#include "control/control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class View;
    class PopupWin;

    class DragTreeView : public JDTreeViewBase
    {
        typedef sigc::signal< void > SIG_DRAG_BEGIN;
        typedef sigc::signal< void > SIG_DRAG_END;

        SIG_DRAG_BEGIN m_sig_drag_begin;
        SIG_DRAG_END m_sig_drag_end;

        bool m_dragging;  // ドラッグ中

        // 範囲選択に使用
        bool m_selection_canceled; // 範囲選択を解除したときにsig_button_release()を発行しないようにする
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
        
        // 入力コントローラ
        CONTROL::Control m_control;

      public:

        SIG_DRAG_BEGIN sig_drag_begin() { return m_sig_drag_begin; }
        SIG_DRAG_END sig_drag_end() { return m_sig_drag_end; }

        // use_usr_fontcolor が true の時はフォントや色を指定する
        DragTreeView( const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even );
        virtual ~DragTreeView();

        virtual void clock_in();

        // ドラッグ中
        const bool is_dragging() const { return m_dragging; }

        // 色初期化
        void init_color( const int colorid_text, const int colorid_bg, const int colorid_bg_even );

        // フォント初期化
        void init_font( const std::string& fontname );
        
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

        virtual bool on_motion_notify_event( GdkEventMotion* event );
        virtual bool on_scroll_event( GdkEventScroll* event );
        virtual bool on_leave_notify_event( GdkEventCrossing* event );

      private:

        void slot_selection_changed();
   };
}

#endif
