// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "command.h"
#include "global.h"
#include "core.h"

void CORE::core_set_command( const std::string& command, const std::string& url,
                             const std::string& arg1, const std::string& arg2,
                             const std::string& arg3, const std::string& arg4,
                             const std::string& arg5, const std::string& arg6
    ) 
{
    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;
    command_arg.arg2 = arg2;
    command_arg.arg3 = arg3;
    command_arg.arg4 = arg4;
    command_arg.arg5 = arg5;
    command_arg.arg6 = arg6;
    
    CORE::get_instance()->set_command( command_arg );
}


Gtk::Widget* CORE::get_toplevel(){ return CORE::get_instance()->get_toplevel(); }
