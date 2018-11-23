// ライセンス: GPL2

//
// 板の管理クラス
//
#ifndef _IMAGEADMIN_H
#define _IMAGEADMIN_H

#include "skeleton/admin.h"

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

        // Gtk::manageで作ってるので view は deleteしなくても良い
        std::list< SKELETON::View* > m_list_view;

        int m_scroll;
        int m_counter_scroll;

      public:

        ImageAdmin( const std::string& url );
        ~ImageAdmin();

        virtual void save_session();

        Gtk::HBox& tab() { return m_tab; }
        virtual Gtk::Widget* get_widget() { return &m_view; }

        virtual bool empty();
        virtual void clock_in();

        // タブの数
        virtual int get_tab_nums();

        // 含まれているページのURLのリスト取得
        virtual const std::list< std::string > get_URLs();

        // 現在表示してるページ番号
        virtual int get_current_page();

      protected:

        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore( const bool only_locked );
        virtual COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock );

        virtual void switch_admin();
        virtual void open_view( const COMMAND_ARGS& command );
        virtual void tab_left( const bool updated );
        virtual void tab_right( const bool updatd );
        virtual void tab_head();
        virtual void tab_tail();
        virtual void redraw_view( const std::string& url );
        virtual void redraw_current_view();
        virtual void close_view( const std::string& url );
        virtual void close_other_views( const std::string& url );
        virtual void restore_lasttab();
        virtual void focus_view( int page );
        virtual void focus_current_view();
        virtual void open_window();
        virtual void close_window();

        virtual SKELETON::View* get_view( const std::string& url );
        virtual SKELETON::View* get_current_view();

        // ページがロックされているかリストで取得
        virtual std::list< bool > get_locked();

        // タブのロック/アンロック
        virtual const bool is_lockable( const int page );
        virtual const bool is_locked( const int page );
        virtual void lock( const int page );
        virtual void unlock( const int page );

        // タブの D&D 処理は SKELETON::Admin とは違うロジックでおこなう
        virtual void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ){}

      private:

        void close_left_views( const std::string& url );
        void close_right_views( const std::string& url );
        void close_error_views( const std::string mode );
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
        void slot_press_left();
        void slot_press_right();
        void slot_release_left();
        void slot_release_right();

        bool copy_file( const std::string& url, const std::string& path_from, const std::string& path_to );
        void save_all();
    };

    IMAGE::ImageAdmin* get_admin();
    void delete_admin();
}


#endif
