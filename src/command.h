// ライセンス: GPL2

//
// コアのインターフェース
//


#ifndef _COMMAND_H
#define _COMMAND_H

#include <string>

namespace Gtk
{
    class Widget;
}

namespace CORE
{
    void core_set_command( const std::string& command,
                           const std::string& url = std::string(),
                           const std::string& arg1 = std::string(),
                           const std::string& arg2 = std::string(),
                           const std::string& arg3 = std::string(),
                           const std::string& arg4 = std::string(),
                           const std::string& arg5 = std::string(),
                           const std::string& arg6 = std::string()
        );

    // メインウィンドウ取得
    Gtk::Widget* get_toplevel();
}

#endif
