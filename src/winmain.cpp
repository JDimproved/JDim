// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
#include "jddebug.h"

#include "winmain.h"
#include "command.h"
#include "core.h"
#include "session.h"

#include "icons/iconmanager.h"

#include "jdlib/loader.h"

#ifdef HAVE_MIGEMO_H
#include "jdlib/jdmigemo.h"
#include "config/globalconf.h"
#endif


WinMain::WinMain( const bool init, const bool skip_setupdiag )
    : SKELETON::JDWindow( false ),
      m_core( NULL ),
      m_cancel_state_event( false )
{
#ifdef _DEBUG
    std::cout << "WinMain::WinMain init = " << init << std::endl
              << "x y w h = " << get_x_win() << " " << get_y_win()
              << " " << get_width_win() << " " << get_height_win() << std::endl;
#endif

    setlocale( LC_ALL, "ja_JP.UTF-8" );

    // (注意) LC_TIMEが"C"でないと環境によってはstrptime()が失敗する
    setlocale(LC_TIME, "C"); 

    // GLIBのスレッドシステム初期化
    if( !Glib::thread_supported() ) Glib::thread_init();
    assert( Glib::thread_supported() );

    // アイコンをセット
    std::list< Glib::RefPtr< Gdk::Pixbuf > > list_icons;
    list_icons.push_back( ICON::get_icon( ICON::JD16 ) );
    list_icons.push_back( ICON::get_icon( ICON::JD32 ) );
    list_icons.push_back( ICON::get_icon( ICON::JD48 ) );
    Gtk::Window::set_default_icon_list( list_icons );
    
    // セッション回復
    SESSION::init_session();

    // サイズ変更
    init_win();

    set_spacing( 4 );

    set_modal( false );
    property_window_position().set_value( Gtk::WIN_POS_NONE );
    set_resizable( true );
    property_destroy_with_parent().set_value( false );

    // migemo 初期化
#ifdef HAVE_MIGEMO_H
    jd_migemo_init( CONFIG::get_migemodict_path().c_str() );
#endif

    // 後はcoreを作って任せる
    m_core = new class CORE::Core( *this );
    m_core->run( init, skip_setupdiag );
}


WinMain::~WinMain()
{
#ifdef _DEBUG
    std::cout << "WinMain::~WinMain window size : x = " << get_x_win() << " y = " << get_y_win()
              << " w = " << get_width_win() << " h = " << get_height_win() << " max = " << is_maximized_win() << std::endl;
#endif

    save_session();

    // migemo のクローズ
#ifdef HAVE_MIGEMO_H
    jd_migemo_close();
#endif

    // アイコンマネージャ削除
    ICON::delete_icon_manager();
}


// 緊急シャットダウン
void WinMain::shutdown()
{
#ifdef _DEBUG
    std::cout << "WinMain::shutdown\n";
#endif

    if( m_core ) m_core->shutdown();
}


// 通常のセッション保存
void WinMain::save_session()
{
#ifdef _DEBUG
    std::cout << "WinMain::save_session\n";
#endif

    if( m_core ){

        delete m_core;
        m_core = NULL;

        SESSION::save_session();

        JDLIB::check_loader_alive();
    }
}


void WinMain::hide()
{
#ifdef _DEBUG
    std::cout << "WinMain::hide\n";
#endif

    // GNOME環境の時に閉じるとウィンドウが最小化する時があるので
    // state_eventをキャンセルする
    m_cancel_state_event = true;

    Gtk::Widget::hide();
}


const int WinMain::get_x_win()
{
    return SESSION::get_x_win_main();
}

const int WinMain::get_y_win()
{
    return SESSION::get_y_win_main();
}

void WinMain::set_x_win( int x )
{
    SESSION::set_x_win_main( x );
}

void WinMain::set_y_win( int y )
{
    SESSION::set_y_win_main( y );
}

const int WinMain::get_width_win()
{
    return SESSION::get_width_win_main();
}

const int WinMain::get_height_win()
{
    return SESSION::get_height_win_main();
}

void WinMain::set_width_win( int width )
{
    SESSION::set_width_win_main( width );
}

void WinMain::set_height_win( int height )
{
    SESSION::set_height_win_main( height );
}

const bool WinMain::is_focus_win()
{
    return SESSION::is_focus_win_main();
}

void WinMain::set_focus_win( bool set )
{
    SESSION::set_focus_win_main( set );
}


const bool WinMain::is_maximized_win()
{
    return SESSION::is_maximized_win_main();
}


void WinMain::set_maximized_win( bool set )
{
    SESSION::set_maximized_win_main( set );
}


const bool WinMain::is_iconified_win()
{
    return SESSION::is_iconified_win_main();
}

void WinMain::set_iconified_win( bool set )
{
    SESSION::set_iconified_win_main( set );
}

const bool WinMain::is_shown_win()
{
    return SESSION::is_shown_win_main();
}


void WinMain::set_shown_win( bool set )
{
    SESSION::set_shown_win_main( set );
}




bool WinMain::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "WinMain::on_delete_event\n";
#endif

    // GNOME環境の時に閉じるとウィンドウが最小化する時があるので
    // state_eventをキャンセルする
    m_cancel_state_event = true;
    return SKELETON::JDWindow::on_delete_event( event );
}


// 最大、最小化
bool WinMain::on_window_state_event( GdkEventWindowState* event )
{
#ifdef _DEBUG
    std::cout << "WinMain::on_window_state_event\n";
    if( m_cancel_state_event ) std::cout << "cancel\n";
#endif     

    // キャンセル ( WinMain::on_delete_even() の説明を参照せよ )
    if( m_cancel_state_event ) return Gtk::Window::on_window_state_event( event );

    return SKELETON::JDWindow::on_window_state_event( event );
}


bool WinMain::on_configure_event( GdkEventConfigure* event )
{
#ifdef _DEBUG
    std::cout << "WinMain::on_configure_event\n";
    if( m_cancel_state_event ) std::cout << "cancel\n";
#endif     

    // キャンセル ( WinMain::on_delete_event() の説明を参照せよ )
    if( m_cancel_state_event ) return Gtk::Window::on_configure_event( event );

    return SKELETON::JDWindow::on_configure_event( event );
}
