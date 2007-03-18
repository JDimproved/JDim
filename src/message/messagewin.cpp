// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "session.h"
#include "command.h"

using namespace MESSAGE;


MessageWin::MessageWin()
    : SKELETON::JDWindow()
{
    // サイズ設定
    int x = SESSION::mes_x();
    int y = SESSION::mes_y();
    int w = SESSION::mes_width();
    int h = SESSION::mes_height();

    resize( w, h );
    move( x, y );
    set_maximized( SESSION::mes_maximized() ); 

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << x << " " << y << " " << w << " " << h << std::endl;
#endif

    pack_remove_end( false, get_statbar(), Gtk::PACK_SHRINK );
    property_window_position().set_value( Gtk::WIN_POS_NONE );
    set_transient_for( *CORE::get_mainwindow() );

    show_all_children();
}


MessageWin::~MessageWin()
{
#ifdef _DEBUG
    std::cout << "MessageWin::~MessageWin\n";
#endif

    // ウィンドウサイズを保存
    int width, height;;
    int x = 0;
    int y = 0;
    get_size( width, height );
    if( get_window() ) get_window()->get_root_origin( x, y );

#ifdef _DEBUG
    std::cout << "window size : x = " << x << " y = " << y << " w = " << width << " h = " << height
              << " max = " << is_maximized() << std::endl;
#endif

    if( ! is_maximized() ){

        SESSION::set_mes_x( x );
        SESSION::set_mes_y( y );
        SESSION::set_mes_width( width );
        SESSION::set_mes_height( height );
    }
    SESSION::set_mes_maximized( is_maximized() );
}


void MessageWin::focus_in()
{
    present();
}


bool MessageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "MessageWin::on_delete_event\n";
#endif

    MESSAGE::get_admin()->set_command( "close_currentview" );

    return true;
}
