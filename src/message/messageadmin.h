// ライセンス: GPL2

//
// 書き込みビューの管理クラス
//

#ifndef _MESSAGEADMIN_H
#define _MESSAGEADMIN_H

#include "skeleton/admin.h"

namespace MESSAGE
{
    class MessageWin;

    class MessageAdmin : public SKELETON::Admin
    {
        MessageWin* m_win;
        SKELETON::View* m_view;
        Gtk::EventBox m_eventbox; // notebook の代わりにeventboxを使用する

      public:

        MessageAdmin( const std::string& url );
        virtual ~MessageAdmin();

        virtual bool empty(){ return ( ! m_view ); }
        virtual Gtk::Widget* get_widget(){ return &m_eventbox; }
        virtual Gtk::Window* get_win();

      protected:

        virtual void command_local( const COMMAND_ARGS& command );

      private:

        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin();
        virtual void tab_left();
        virtual void tab_right();
        virtual void close_view( const std::string& url );
        virtual void close_current_view();
        virtual void set_title( const std::string& url, const std::string& title );
        virtual void set_url( const std::string& url, const std::string& url_show );
        virtual void set_status( const std::string& url, const std::string& stat );
        virtual void focus_view( int page );
        virtual void focus_current_view();
        virtual void relayout_all();
        virtual void open_window();
        virtual void close_window();

        virtual SKELETON::View* get_view( const std::string& url, bool use_find = false );
        virtual SKELETON::View* get_current_view();
    };
    
    MESSAGE::MessageAdmin* get_admin();
    void delete_admin();
}

#endif
