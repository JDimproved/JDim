// ライセンス: GPL2

//
// 書き込みウィンドウの管理クラス
//

#ifndef _MESSAGEADMIN_H
#define _MESSAGEADMIN_H

#include <gtkmm.h>
#include <string>

#include "command_args.h"

namespace SKELETON
{
    class View;
}

namespace MESSAGE
{
    class MessageWin;

    // SKELETON::Admin を継承していない(独自Adminクラス)
    class MessageAdmin
    {
        Glib::Dispatcher m_disp;
        std::list< COMMAND_ARGS > m_list_command;

        MessageWin* m_win;
        SKELETON::View* m_view;

      public:

        MessageAdmin();
        ~MessageAdmin();

        void clock_in();

        void set_command( const std::string& command, const std::string& url = std::string() , const std::string& arg1 = std::string() );

      private:

        void exec_command();
        void open_view( const std::string& url, const std::string& msg, bool new_thread );
        void redraw_view( const std::string& url );
        void close_view();
        void focus_view();
    };
    
    MESSAGE::MessageAdmin* get_admin();
    void delete_admin();
}

#endif
