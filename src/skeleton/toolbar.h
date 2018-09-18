// ライセンス: GPL2
//
// ツールバーの基底クラス
//

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include "jdtoolbar.h"

#include <gtkmm.h>
#include "gtkmmversion.h"

namespace SKELETON
{
    class Admin;
    class View;
    class ImgToolButton;
    class ImgToggleToolButton;
    class ToolMenuButton;
    class ToolBackForwardButton;
    class BackForwardButton;
    class CompletionEntry;

    class ToolBar : public Gtk::VBox
    {
        SKELETON::Admin* m_admin;
        std::string m_url;

        bool m_enable_slot;

#if !GTKMM_CHECK_VERSION(2,12,0)
        Gtk::Tooltips m_tooltip;
#endif

        // ボタンバー
        SKELETON::JDToolbar m_buttonbar;
        bool m_buttonbar_shown;
        bool m_buttonbar_packed;

        // ラベル
        Gtk::ToolItem* m_tool_label;
        Gtk::EventBox* m_ebox_label;
        Gtk::Label* m_label;

        // 検索関係
        Gtk::Toolbar* m_searchbar; // 検索バー
        bool m_searchbar_shown;
        bool m_searchbar_packed;
        SKELETON::ImgToolButton *m_button_open_searchbar;
        SKELETON::ImgToolButton *m_button_close_searchbar;
        SKELETON::ImgToolButton *m_button_up_search;
        SKELETON::ImgToolButton *m_button_down_search;
        SKELETON::ImgToolButton *m_button_clear_highlight;

        Gtk::ToolItem* m_tool_search;
        SKELETON::CompletionEntry* m_entry_search;

        // 板を開く
        Gtk::Label* m_label_board;
        SKELETON::ToolMenuButton* m_button_board;

        // その他ボタン
        SKELETON::ImgToolButton* m_button_write;
        SKELETON::ImgToolButton* m_button_reload;
        SKELETON::ImgToolButton* m_button_stop;
        SKELETON::ImgToolButton* m_button_close;
        SKELETON::ImgToolButton* m_button_delete;
        SKELETON::ImgToolButton* m_button_favorite;
        SKELETON::ImgToolButton* m_button_undo;
        SKELETON::ImgToolButton* m_button_redo;
        SKELETON::ImgToggleToolButton* m_button_lock;

        // 進む、戻るボタン
        SKELETON::ToolBackForwardButton* m_button_back;
        SKELETON::ToolBackForwardButton* m_button_forward;

      public:

        ToolBar( Admin* admin );
        ~ToolBar() noexcept {}

        void set_url( const std::string& url );
        const std::string& get_url() { return m_url; }

        bool is_empty();

        // タブが切り替わった時にDragableNoteBookから呼び出される( Viewの情報を取得する )
        virtual void set_view( SKELETON::View * view );

        // タブが切り替わった時にDragableNoteBookから呼び出される( ツールバーを表示する )
        void show_toolbar();

        // ボタンバー表示/非表示
        void open_buttonbar();
        void close_buttonbar();

        // 検索バー表示/非表示
        void open_searchbar();
        void close_searchbar();

        // 検索entryをフォーカス
        void focus_entry_search();

        // 書き込みボタンをフォーカス
        void focus_button_write();

        // ボタン表示更新
        void update_button();

      protected:

        SKELETON::Admin* get_admin(){ return m_admin; }

        // ボタンのパッキング
        virtual void pack_buttons() = 0;
        void unpack_buttons();
        void unpack_search_buttons();

        // ボタンのrelief指定
        void set_relief();


        Gtk::Toolbar& get_buttonbar(){ return m_buttonbar; }

        // ラベル
        Gtk::ToolItem* get_label();

        // 検索関係
        Gtk::Toolbar* get_searchbar();
        Gtk::ToolItem* get_button_open_searchbar();
        Gtk::ToolItem* get_button_close_searchbar();

        // mode は補完モード ( compmanager.h 参照 )
        Gtk::ToolItem* get_tool_search( const int mode );
        SKELETON::CompletionEntry* get_entry_search();

        // CompletionEntry の入力コントローラのモード設定
        void add_search_control_mode( const int mode );

        const std::string get_search_text();

        // 上検索
        Gtk::ToolButton* get_button_up_search();

        // 下検索
        Gtk::ToolButton* get_button_down_search();

        // ハイライト解除
        Gtk::ToolButton* get_button_clear_highlight();

        // 板を開く
        Gtk::ToolItem* get_button_board();

        // その他ボタン
        Gtk::ToolItem* get_button_write();
        Gtk::ToolItem* get_button_reload();
        Gtk::ToolItem* get_button_stop();
        Gtk::ToolButton* get_button_close();
        Gtk::ToolItem* get_button_delete();
        Gtk::ToolItem* get_button_favorite();
        Gtk::ToolButton* get_button_undo();
        Gtk::ToolButton* get_button_redo();
        Gtk::ToolItem* get_button_lock();

        Gtk::ToolItem* get_button_back();
        Gtk::ToolItem* get_button_forward();

        void pack_separator();
        void pack_transparent_separator();
        void set_tooltip( Gtk::ToolItem& toolitem, const std::string& tip );

      private:

        // ラベル関係
        void set_label( const std::string& label );
        void set_color( const std::string& color );

        // 書き込みボタン関係
        void drawframe_button_write( const bool draw );
        bool slot_focusout_write_button( GdkEventFocus* event );

#if GTKMM_CHECK_VERSION(3,0,0)
        // 閉じるボタン関係
        static constexpr const char* s_css_leave = u8"jd-leave";
        void setup_manual_styling( Gtk::ToolButton& toolbutton );
#endif


        // 検索関係
        void slot_toggle_searchbar();
        void slot_changed_search();
        void slot_active_search();
        void slot_operate_search( const int controlid );
        void slot_clicked_up_search();
        void slot_clicked_down_search();
        void slot_clear_highlight();

        // 板を開く
        void slot_open_board();
        void slot_menu_board( int i );

        // その他ボタン
        void slot_clicked_write();
        void slot_clicked_reload();
        void slot_clicked_stop();
        void slot_clicked_close();
        void slot_clicked_delete();
        void slot_clicked_favorite();
        void slot_lock_clicked();

        void slot_clicked_back();
        void slot_selected_back( const int i );

        void slot_clicked_forward();
        void slot_selected_forward( const int i );
    };
}


#endif
