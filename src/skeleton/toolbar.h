// ライセンス: GPL2
//
// ツールバーの基底クラス
//

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include <gtkmm.h>


namespace SKELETON
{
    class Admin;
    class View;
    class ImgButton;
    class ImgToggleButton;
    class BackForwardButton;
    class SearchEntry;

    class ToolBar : public Gtk::VBox
    {
        SKELETON::Admin* m_admin;
        std::string m_url;

        bool m_enable_slot;

        // ツールバー表示状態
        bool m_toolbar_shown;

        Gtk::Tooltips m_tooltip;
        Gtk::HBox m_buttonbar;

        // ラベル
        Gtk::EventBox* m_ebox_label;
        Gtk::Label* m_label;

        // 検索関係
        Gtk::HBox* m_searchbar; // 検索バー
        bool m_searchbar_shown;
        SKELETON::ImgButton *m_button_open_searchbar;
        SKELETON::ImgButton *m_button_close_searchbar;

        SKELETON::SearchEntry* m_entry_search;
        SKELETON::ImgButton *m_button_up_search;
        SKELETON::ImgButton *m_button_down_search;

        // その他ボタン
        SKELETON::ImgButton* m_button_write;
        SKELETON::ImgButton* m_button_reload;
        SKELETON::ImgButton* m_button_stop;
        SKELETON::ImgButton* m_button_close;
        SKELETON::ImgButton* m_button_delete;
        SKELETON::ImgButton* m_button_favorite;
        SKELETON::ImgToggleButton* m_button_lock;

        SKELETON::BackForwardButton* m_button_back;
        SKELETON::BackForwardButton* m_button_forward;

      public:

        ToolBar( Admin* admin );
        virtual ~ToolBar(){}

        void set_url( const std::string& url );
        const std::string& get_url() { return m_url; }

        // タブが切り替わった時に呼び出される( Viewの情報を取得する )
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

        // ラベル表示更新
        void update_label( SKELETON::View* view );

        // 閉じるボタンの表示更新
        void update_close_button( SKELETON::View* view );

        // ボタン表示更新
        void update_button();

      protected:

        // ボタンのパッキング
        virtual void pack_buttons() = 0;
        void unpack_buttons();

        // ボタンのrelief指定
        void set_relief();


        Gtk::HBox& get_buttonbar(){ return m_buttonbar; }

        // ラベル
        Gtk::EventBox* get_label();

        // 検索関係
        Gtk::HBox* get_searchbar();
        SKELETON::ImgButton* get_button_open_searchbar();
        SKELETON::ImgButton* get_button_close_searchbar();

        SKELETON::SearchEntry* get_entry_search();
        Gtk::Button* get_button_up_search();
        Gtk::Button* get_button_down_search();

        // その他ボタン
        Gtk::Button* get_button_write();
        Gtk::Button* get_button_reload();
        Gtk::Button* get_button_stop();
        Gtk::Button* get_button_close();
        Gtk::Button* get_button_delete();
        Gtk::Button* get_button_favorite();
        Gtk::ToggleButton* get_button_lock();

        Gtk::Button* get_button_back();
        Gtk::Button* get_button_forward();

        void pack_separator();
        void set_tooltip( Gtk::Widget& widget, const std::string& tip ){ m_tooltip.set_tip( widget, tip ); }

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
