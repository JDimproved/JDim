// ライセンス: GPL2
//
// treeviewクラスの基底クラス
//

#ifndef _TREEVIEWBASE_H
#define _TREEVIEWBASE_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class JDTreeViewBase : public Gtk::TreeView
    {
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;

        typedef sigc::signal< bool, GdkEventScroll* > SIG_SCROLL_EVENT;
        typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;
        typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_RELEASE;
        typedef sigc::signal< bool, GdkEventMotion* > SIG_MOTION_NOTIFY;

        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;

        SIG_SCROLL_EVENT m_sig_scroll_event;
        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_BUTTON_RELEASE m_sig_button_release;
        SIG_MOTION_NOTIFY m_sig_motion_notify;

        // get_row_height() で高さを取得するためのcolumn番号
        int m_column_for_height;

      public:

        SIG_KEY_PRESS& sig_key_press() { return m_sig_key_press; }
        SIG_KEY_RELEASE& sig_key_release() { return m_sig_key_release; }

        SIG_SCROLL_EVENT& sig_scroll_event(){ return m_sig_scroll_event; }
        SIG_BUTTON_PRESS& sig_button_press() { return m_sig_button_press; }
        SIG_BUTTON_RELEASE& sig_button_release() { return m_sig_button_release; }
        SIG_MOTION_NOTIFY& sig_motion_notify() { return m_sig_motion_notify; }

        JDTreeViewBase();
        ~JDTreeViewBase() noexcept;

        // 行数
        const int get_row_size();

        // カーソル解除
        void unset_cursor(){ get_selection()->unselect_all(); }

        // 現在フォーカスしてる行の最初のパスを取得
        Gtk::TreeModel::Path get_current_path();

        // 現在フォーカスしてる行の最初のrowを取得
        Gtk::TreeModel::Row get_current_row();

        //x, y 座標の下のパスを取得
        Gtk::TreeModel::Path get_path_under_xy( int x, int y );

        // 現在のマウスポインタの下のパスを取得
        Gtk::TreeModel::Path get_path_under_mouse();

        // 現在のマウスポインタの下のセルの幅高さとセル内での座標を取得        
        void get_cell_xy_wh( int& cell_x, int& cell_y, int& cell_w, int& cell_h );

        // 選択中の Gtk::TreeModel::iterator のリストを取得
        std::list< Gtk::TreeModel::iterator > get_selected_iterators();

        // 選択行の削除
        virtual void delete_selected_rows( const bool force );

        // 選択行の移動
        void goto_top();
        void goto_bottom();
        const bool row_up();
        const bool row_down();
        void page_up();
        void page_down();

        // path の前後のpathを取得
        Gtk::TreePath prev_path( const Gtk::TreePath& path, bool check_expand = true );
        Gtk::TreePath next_path( const Gtk::TreePath& path, bool check_expand = true );

        // path -> row 変換
        Gtk::TreeModel::Row get_row( const Gtk::TreePath& path );

        // pathの親を再起的にexpandする
        void expand_parents( const Gtk::TreePath& path );

        // pathが開かれているか
        const bool is_expand( const Gtk::TreePath& path );

        // 行のセルの高さ
        int get_row_height();
        void set_column_for_height( int column ){ m_column_for_height = column; }

      protected:

        bool on_key_press_event( GdkEventKey* event ) override;
        bool on_key_release_event( GdkEventKey* event ) override;

        bool on_scroll_event( GdkEventScroll* event ) override;
        bool on_button_press_event( GdkEventButton* event ) override;
        bool on_button_release_event( GdkEventButton* event ) override;
        bool on_motion_notify_event( GdkEventMotion* event ) override;
   };
}

#endif
