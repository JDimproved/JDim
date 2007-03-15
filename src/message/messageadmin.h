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

        std::string m_title;

      public:

        MessageAdmin( const std::string& url );
        virtual ~MessageAdmin();

        virtual bool empty(){ return ( ! m_view ); }
        virtual Gtk::Widget* get_widget(){ return &m_eventbox; }
        virtual Gtk::Window* get_win();

        virtual void clock_in();

      protected:

        virtual void command_local( const COMMAND_ARGS& command );

      private:

        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin();
        virtual void tab_left();
        virtual void tab_right();
        virtual void redraw_view( const std::string& url );
        virtual void redraw_current_view();
        virtual void close_view( const std::string& url );
        virtual void close_current_view();
        virtual void set_status( const std::string& url, const std::string& stat );
        virtual void focus_current_view();
        virtual void relayout_all();
        virtual void open_window();
        virtual void close_window();
    };
    
    MESSAGE::MessageAdmin* get_admin();
    void delete_admin();
}

#endif
