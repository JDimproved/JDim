// ライセンス: GPL2
//
// ツールバーの基底クラス
//

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include "jdtoolbar.h"

#include <gtkmm.h>

namespace SKELETON
{
    class Admin;
    class View;
    class ImgToolButton;
    class ImgToggleToolButton;
    class MenuButton;
    class BackForwardButton;
    class SearchEntry;

    class ToolBar : public Gtk::VBox
    {
        SKELETON::Admin* m_admin;
        std::string m_url;

        bool m_enable_slot;

        // ツールバー表示状態
        bool m_toolbar_shown;

#if GTKMMVER < 2120
        Gtk::Tooltips m_tooltip;
#endif

        // メインツールバー
        SKELETON::JDToolbar m_buttonbar;

        // ラベル
        Gtk::ToolItem* m_tool_label;
        Gtk::EventBox* m_ebox_label;
        Gtk::Label* m_label;

        // 検索関係
        Gtk::Toolbar* m_searchbar; // 検索バー
        bool m_searchbar_shown;
        SKELETON::ImgToolButton *m_button_open_searchbar;
        SKELETON::ImgToolButton *m_button_close_searchbar;
        SKELETON::ImgToolButton *m_button_up_search;
        SKELETON::ImgToolButton *m_button_down_search;

        Gtk::ToolItem* m_tool_search;
        SKELETON::SearchEntry* m_entry_search;

        // 板を開く
        // Gtk::ToolButtonを使うとボタンの枠が表示されないことがあるので
        // Gtk::ToolItemにGtk::Buttonをaddする
        Gtk::ToolItem* m_tool_board;
        Gtk::Label* m_label_board;
        SKELETON::MenuButton* m_button_board;

        // その他ボタン
        SKELETON::ImgToolButton* m_button_write;
        SKELETON::ImgToolButton* m_button_reload;
        SKELETON::ImgToolButton* m_button_stop;
        SKELETON::ImgToolButton* m_button_close;
        SKELETON::ImgToolButton* m_button_delete;
        SKELETON::ImgToolButton* m_button_favorite;
        SKELETON::ImgToggleToolButton* m_button_lock;

        // 進む、戻るボタン
        Gtk::ToolItem* m_tool_back;
        SKELETON::BackForwardButton* m_button_back;

        Gtk::ToolItem* m_tool_forward;
        SKELETON::BackForwardButton* m_button_forward;

      public:

        ToolBar( Admin* admin );
        virtual ~ToolBar(){}

        void set_url( const std::string& url );
        const std::string& get_url() { return m_url; }

        // タブが切り替わった時にDragableNoteBookから呼び出される( Viewの情報を取得する )
        virtual void set_view( SKELETON::View * view );

        bool is_empty();

        // ツールバー表示
        void show_toolbar();

        // ツールバー非表示
        void hide_toolbar();

        // 検索バー表示/非表示
        void open_searchbar();
        void close_searchbar();

        // 検索entryをフォーカス
        void focus_entry_search();

        // ボタン表示更新
        void update_button();

      protected:

        // ボタンのパッキング
        virtual void pack_buttons() = 0;
        void unpack_buttons();

        // ボタンのrelief指定
        void set_relief();


        Gtk::Toolbar& get_buttonbar(){ return m_buttonbar; }

        // ラベル
        Gtk::ToolItem* get_label();

        // 検索関係
        Gtk::Toolbar* get_searchbar();
        Gtk::ToolButton* get_button_open_searchbar();
        Gtk::ToolButton* get_button_close_searchbar();

        Gtk::ToolItem* get_entry_search();
        void add_search_mode( const int mode );
        const std::string get_search_text();
        Gtk::ToolButton* get_button_up_search();
        Gtk::ToolButton* get_button_down_search();

        // 板を開く
        Gtk::ToolItem* get_button_board();

        // その他ボタン
        Gtk::ToolButton* get_button_write();
        Gtk::ToolButton* get_button_reload();
        Gtk::ToolButton* get_button_stop();
        Gtk::ToolButton* get_button_close();
        Gtk::ToolButton* get_button_delete();
        Gtk::ToolButton* get_button_favorite();
        Gtk::ToggleToolButton* get_button_lock();

        Gtk::ToolItem* get_button_back();
        Gtk::ToolItem* get_button_forward();

        void pack_separator();
        void pack_transparent_separator();
        void set_tooltip( Gtk::ToolItem& toolitem, const std::string& tip );

      private:

        // ラベル関係
        void set_label( const std::string& label );
        void set_broken(); // viewが壊れている
        void set_old(); // viewが古い

        // 検索関係
        void slot_toggle_searchbar();
        void slot_changed_search();
        void slot_active_search();
        void slot_operate_search( int controlid );
        void slot_clicked_up_search();
        void slot_clicked_down_search();

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
