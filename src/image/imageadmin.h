// ライセンス: 最新のGPL

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

        Gtk::HBox& tab() { return m_tab; }
        Gtk::EventBox& view() { return m_view; }

        virtual bool empty();
        virtual std::list< std::string > get_URLs();
        virtual void clock_in();

        virtual int get_current_page();

      protected:
        virtual void command_local( const COMMAND_ARGS& command );

      private:
        
        virtual void restore();
        virtual void open_view( const COMMAND_ARGS& command );
        virtual void tab_left();
        virtual void tab_right();
        virtual void redraw_view( const std::string& url );
        virtual void redraw_current_view();
        virtual void close_view( const std::string& url );
        virtual void focus_current_view();
        void close_other_views( const std::string& url );
        void reorder( const std::string& url_from, const std::string& url_to );
        void focus_out_all();
        void switch_img( const std::string& url );

        SKELETON::View* get_icon( const std::string& url, int& pos );
        SKELETON::View* get_icon( const std::string& url );
        SKELETON::View* get_nth_icon( unsigned int n );
        SKELETON::View* get_current_icon();
        SKELETON::View* get_view( const std::string& url );
        SKELETON::View* get_current_view();

        // スクロール
        void scroll_tab( int scroll );
        void slot_press_left();
        void slot_press_right();
        void slot_release_left();
        void slot_release_right();

        void save_all();
    };

    IMAGE::ImageAdmin* get_admin();
    void delete_admin();
};


#endif
