// ライセンス: GPL2

//
// 板の管理クラス
//
#ifndef _IMAGEADMIN_H
#define _IMAGEADMIN_H

#include "skeleton/admin.h"

#include <memory>
#include <queue>
#include <string>


namespace IMAGE
{
    // スクロール方向
    enum{
        SCROLL_NO,
        SCROLL_LEFT,
        SCROLL_RIGHT
    };

    class ImageAdmin : public SKELETON::Admin
    {
        Gtk::HBox m_tab;
        Gtk::HBox m_iconbox;
        Gtk::ScrolledWindow m_scrwin;
        Gtk::Button m_left, m_right;
        Gtk::EventBox m_view;

        std::list<std::unique_ptr<SKELETON::View>> m_list_view;
        /// @brief 画像を閉じるURLを入れるキュー（待ち行列）
        std::queue<std::string> m_que_close_url;
        /// @brief 割り込みハンドラの接続状況を管理する
        sigc::connection m_conn_close_cmd;

        int m_scroll;
        int m_counter_scroll{};

        double m_smooth_dy{}; // GDK_SCROLL_SMOOTH のスクロール変化量

        Glib::RefPtr<Gtk::GestureMultiPress> m_gesture_press_left;
        Glib::RefPtr<Gtk::GestureMultiPress> m_gesture_press_right;

      public:

        explicit ImageAdmin( const std::string& url );
        ~ImageAdmin() override;

        void save_session() override;

        Gtk::HBox& tab() { return m_tab; }
        Gtk::Widget* get_widget() override { return &m_view; }

        bool empty() const override;
        void clock_in() override;

        // タブの数
        int get_tab_nums() override;

        // 含まれているページのURLのリスト取得
        std::vector<std::string> get_URLs() override;

        // 現在表示してるページ番号
        int get_current_page() override;

      protected:

        void command_local( const COMMAND_ARGS& command ) override;

        void restore( const bool only_locked ) override;
        COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) override;

        void switch_admin() override;
        void open_view( const COMMAND_ARGS& command ) override;
        void tab_left( const TabMove mode ) override;
        void tab_right( const TabMove mode ) override;
        void tab_head() override;
        void tab_tail() override;
        void redraw_view( const std::string& url ) override;
        void redraw_current_view() override;
        void close_view( const std::string& url ) override;
        void close_other_views( const std::string& url ) override;
        void restore_lasttab() override;
        void focus_view( int page ) override;
        void focus_current_view() override;
        void open_window() override;
        void close_window() override;

        SKELETON::View* get_view( const std::string& url ) override;
        SKELETON::View* get_current_view() override;

        // ページがロックされているかリストで取得
        std::vector<char> get_locked() override;

        // タブのロック/アンロック
        bool is_lockable( const int page ) override;
        bool is_locked( const int page ) override;
        void lock( const int page ) override;
        void unlock( const int page ) override;

        // タブの D&D 処理は SKELETON::Admin とは違うロジックでおこなう
        void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) override {}

      private:

        void close_left_views( const std::string& url );
        void close_right_views( const std::string& url );
        void close_error_views( const std::string& mode );
        void close_noerror_views();
        void reorder( const std::string& url_from, const std::string& url_to );
        void update_status_of_all_views();
        void focus_out_all();
        void switch_img( const std::string& url );

        SKELETON::View* get_icon( const std::string& url, int& pos );
        SKELETON::View* get_icon( const std::string& url );
        SKELETON::View* get_nth_icon( const unsigned int n );
        SKELETON::View* get_current_icon();

        // スクロール
        void scroll_tab( int scroll );

        // スクロールボタン
        void slot_press_left( int, double, double );
        void slot_press_right( int, double, double );
        void slot_release_left( int, double, double );
        void slot_release_right( int, double, double );
        // マウスホイールによるタブ切り替え
        bool slot_scroll_event( GdkEventScroll* event );

        bool copy_file( const std::string& url, const std::string& path_from, const std::string& path_to );
        void save_all();

        bool slot_close_command();
    };

    IMAGE::ImageAdmin* get_admin();
    void delete_admin();
}


#endif
