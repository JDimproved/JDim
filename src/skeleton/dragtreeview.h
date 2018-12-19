// ライセンス: GPL2
//
// ドラッグ開始可能なtreeviewクラス
//
// set_enable_drop_uri_list() で他のアプリから text/uri-list のドロップを受け付ける
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

    typedef sigc::signal< void, const std::list< std::string >& > SIG_DROPPED_URI_LIST;

    class DragTreeView : public JDTreeViewBase
    {
        std::string m_url;
        std::string m_dndtarget;

        bool m_dragging;  // ドラッグ中

        // 範囲選択に使用
        bool m_selection_canceled; // 範囲選択を解除したときにsig_button_release()を発行しないようにする
        Gtk::TreeModel::Path m_path_dragstart;
        Gtk::TreeModel::Path m_path_dragpre;

        Tooltip m_tooltip;

#if GTKMM_CHECK_VERSION(3,0,0)
        static constexpr const char* s_css_classname = u8"jd-dragtreeview";
        Glib::RefPtr< Gtk::CssProvider > m_provider = Gtk::CssProvider::create();

        using Color = Gdk::RGBA;
#else
        using Color = Gdk::Color;
#endif

        // 色
        bool m_use_bg_even;
        Color m_color_text;
        Color m_color_bg;
        Color m_color_bg_even;

        // ポップアップウィンドウ用
        PopupWin* m_popup_win;
        std::string m_pre_popup_url;
        bool m_popup_shown;
        
        // 入力コントローラ
        CONTROL::Control m_control;

        // text/uri-list をドロップされた
        SIG_DROPPED_URI_LIST m_sig_dropped_url_list;

      public:

        // use_usr_fontcolor が true の時はフォントや色を指定する
        DragTreeView( const std::string& url, const std::string& dndtarget,
                      const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even );
        ~DragTreeView();

        virtual void clock_in();

        SIG_DROPPED_URI_LIST sig_dropped_uri_list(){ return m_sig_dropped_url_list; }
        const std::string& get_dndtarget() const { return m_dndtarget; }

        // 他のアプリからの text/url-list のドロップを有効にする
        // ドロップされるとSIG_DROPPED_URI_LIST を発行する
        void set_enable_drop_uri_list();

        void redraw_view();

        // 色初期化
        void init_color( const int colorid_text, const int colorid_bg, const int colorid_bg_even );

        // フォント初期化
        void init_font( const std::string& fontname );
        
        // ツールチップ表示
        // set_tooltip_min_width()で指定した幅よりもツールチップが広い場合は表示
        void set_str_tooltip( const std::string& str );
        void set_tooltip_min_width( const int min_width );
        void hide_tooltip();
        void show_tooltip();

        // ポップアップウィンドウ表示
        const std::string& pre_popup_url() const { return m_pre_popup_url; }
        void reset_pre_popupurl( const std::string& url ){ if( ! m_pre_popup_url.empty() && m_pre_popup_url != url ) m_pre_popup_url = std::string(); }
        void show_popup( const std::string& url, View* view );
        void hide_popup();

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
        bool on_button_press_event( GdkEventButton* event ) override;
        bool on_button_release_event( GdkEventButton* event ) override;
        void on_drag_begin( const Glib::RefPtr< Gdk::DragContext>& context ) override;
        void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context ) override;

        void on_drag_data_received( const Glib::RefPtr< Gdk::DragContext >& context, int x, int y,
                                    const Gtk::SelectionData& selection, guint info, guint time ) override;

        bool on_key_press_event( GdkEventKey* event ) override;

        bool on_motion_notify_event( GdkEventMotion* event ) override;
        bool on_scroll_event( GdkEventScroll* event ) override;
        bool on_leave_notify_event( GdkEventCrossing* event ) override;

      private:

        // ポップアップが表示されているか
        bool is_popup_shown() const { return ( m_popup_win && m_popup_shown ); }

        // ポップアップ削除
        void delete_popup();

        // ポップアップが表示されていてかつマウスがその上にあるか
        bool is_mouse_on_popup();

        bool slot_popup_leave_notify_event( GdkEventCrossing* event );
        void slot_selection_changed();
   };
}

#endif
