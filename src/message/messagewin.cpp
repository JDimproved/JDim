// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "jdlib/miscgtk.h"

#include "config/globalconf.h"

#include "session.h"
#include "command.h"

using namespace MESSAGE;

// メッセージウィンドウにはステータスバー内のマウスジェスチャ表示欄が
// 不要なので SKELETON::JDWindow() の第2引数を flase にする
MessageWin::MessageWin()
    : SKELETON::JDWindow( CONFIG::get_fold_message(), false )
{
#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = "
              << get_x_win() << " " << get_y_win()
              << " " << get_width_win() << " " << get_height_win() << std::endl;
#endif

    get_vbox().pack_remove_end( false, get_statbar(), Gtk::PACK_SHRINK );
    init_win();

    if( ! CONFIG::get_fold_message() ) set_transient_for( *CORE::get_mainwindow() );

    show_all_children();
}


MessageWin::~MessageWin()
{
#ifdef _DEBUG
    std::cout << "MessageWin::~MessageWin window size : x = " << get_x_win() << " y = " << get_y_win()
              << " w = " << get_width_win() << " h = " << get_height_win() << " max = " << is_maximized_win() << std::endl;
#endif

    set_shown_win( false );
    CORE::core_set_command( "restore_focus" );
}


bool MessageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "MessageWin::on_delete_event\n";
#endif

    MESSAGE::get_admin()->set_command( "close_currentview" );

    return true;
}


int MessageWin::get_x_win()
{
    return SESSION::get_x_win_mes();
}

int MessageWin::get_y_win()
{
    return SESSION::get_y_win_mes();
}

void MessageWin::set_x_win( const int x )
{
    SESSION::set_x_win_mes( x );
}

void MessageWin::set_y_win( const int y )
{
    SESSION::set_y_win_mes( y );
}

int MessageWin::get_width_win()
{
    return SESSION::get_width_win_mes();
}

int MessageWin::get_height_win()
{
    return SESSION::get_height_win_mes();
}

void MessageWin::set_width_win( const int width )
{
    SESSION::set_width_win_mes( width );
}

void MessageWin::set_height_win( const int height )
{
    SESSION::set_height_win_mes( height );
}

bool MessageWin::is_focus_win()
{
    return SESSION::is_focus_win_mes();
}

void MessageWin::set_focus_win( const bool set )
{
    SESSION::set_focus_win_mes( set );
}


bool MessageWin::is_maximized_win()
{
    return SESSION::is_maximized_win_mes();
}

void MessageWin::set_maximized_win( const bool set )
{
    SESSION::set_maximized_win_mes( set );
}


bool MessageWin::is_iconified_win()
{
    return SESSION::is_iconified_win_mes();
}

void MessageWin::set_iconified_win( const bool set )
{
    SESSION::set_iconified_win_mes( set );
}


bool MessageWin::is_shown_win()
{
    return SESSION::is_shown_win_mes();
}


void MessageWin::set_shown_win( const bool set )
{
    SESSION::set_shown_win_mes( set );
}


void MessageWin::switch_admin()
{
    CORE::core_set_command( "switch_message" );    
}
