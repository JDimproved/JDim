// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "session.h"
#include "command.h"

using namespace MESSAGE;


MessageWin::MessageWin()
{
    // サイズ設定
    int x = SESSION::mes_x();
    int y = SESSION::mes_y();
    int w = SESSION::mes_width();
    int h = SESSION::mes_height();
    m_maximized = SESSION::mes_maximized();

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << x << " " << y << " " << w << " " << h << std::endl;
#endif

    resize( w, h );
    move( x, y );
    if( m_maximized ) maximize();

    property_window_position().set_value( Gtk::WIN_POS_NONE );
    if( CORE::get_toplevel() ) set_transient_for( *dynamic_cast< Gtk::Window* >( CORE::get_toplevel() ) );
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
              << " max = " << m_maximized << std::endl;
#endif

    if( !m_maximized ){
        SESSION::set_mes_x( x );
        SESSION::set_mes_y( y );
        SESSION::set_mes_width( width );
        SESSION::set_mes_height( height );
    }
    SESSION::set_mes_maximized( m_maximized );
}




bool MessageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "MessageWin::on_delete_event\n";
#endif

    MESSAGE::get_admin()->set_command( "close_currentview" );

    return true;
}



bool MessageWin::on_window_state_event( GdkEventWindowState* event )
{
    m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;

#ifdef _DEBUG
    std::cout << "MessageWin::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     

    return Gtk::Window::on_window_state_event( event );
}
