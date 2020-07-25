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

#include "control/controlutil.h"
#include "control/controlid.h"

#ifdef HAVE_MIGEMO_H
#include "jdlib/jdmigemo.h"
#include "config/globalconf.h"
#endif


JDWinMain::JDWinMain( const bool init, const bool skip_setupdiag,
                      const int init_w, const int init_h, const int init_x, const int init_y )
    : SKELETON::JDWindow( false ),
      m_core( nullptr ),
      m_cancel_state_event( false )
{
#ifdef _DEBUG
    std::cout << "JDWinMain::JDWinMain init = " << init << std::endl
              << "x y w h = " << JDWinMain::get_x_win() << " " << JDWinMain::get_y_win()
              << " " << JDWinMain::get_width_win() << " " << JDWinMain::get_height_win() << std::endl;
#endif

    setlocale( LC_ALL, "ja_JP.UTF-8" );

#ifndef _WIN32
    // アイコンをセット
    // WindowsはwindresのデフォルトICONが初期適用されるので不要
    std::vector< Glib::RefPtr< Gdk::Pixbuf > > list_icons;
    list_icons.push_back( ICON::get_icon( ICON::JD16 ) );
    list_icons.push_back( ICON::get_icon( ICON::JD32 ) );
    list_icons.push_back( ICON::get_icon( ICON::JD48 ) );
    list_icons.push_back( ICON::get_icon( ICON::JD96 ) );
    Gtk::Window::set_default_icon_list( list_icons );
#endif

    // セッション回復
    SESSION::init_session();
    bool cancel_maximize = false;
    if( init_w >= 0 ){
        cancel_maximize = true;
        JDWinMain::set_width_win( init_w );
    }
    if( init_h >= 0 ){
        cancel_maximize = true;
        JDWinMain::set_height_win( init_h );
    }
    if( init_x >= 0 ){
        cancel_maximize = true;
        JDWinMain::set_x_win( init_x );
    }
    if( init_y >= 0 ){
        cancel_maximize = true;
        JDWinMain::set_y_win( init_y );
    }
    if( cancel_maximize ){
        JDWinMain::set_maximized_win( false );
        JDWinMain::set_full_win( false );
    }

    // サイズ変更
    init_win();
    if( SESSION::is_full_win_main() ) fullscreen();

    set_spacing( 4 );

    set_modal( false );
    property_window_position().set_value( Gtk::WIN_POS_NONE );
    set_resizable( true );
    property_destroy_with_parent().set_value( false );

    add_events( Gdk::BUTTON_PRESS_MASK );
    add_events( Gdk::BUTTON_RELEASE_MASK );
    add_events( Gdk::POINTER_MOTION_MASK );

    // migemo 初期化
#ifdef HAVE_MIGEMO_H
    jdmigemo::init( CONFIG::get_migemodict_path() );
#endif

    // 後はcoreを作って任せる
    m_core = new class CORE::Core( *this );
    m_core->run( init, skip_setupdiag );
}


JDWinMain::~JDWinMain()
{
#ifdef _DEBUG
    std::cout << "JDWinMain::~JDWinMain window size : x = " << JDWinMain::get_x_win()
              << " y = " << JDWinMain::get_y_win() << " w = " << JDWinMain::get_width_win()
              << " h = " << JDWinMain::get_height_win() << " max = " << JDWinMain::is_maximized_win() << std::endl;
#endif

    if( m_core ){

        delete m_core;
        m_core = nullptr;

        JDLIB::check_loader_alive();
    }

    // migemo のクローズ
#ifdef HAVE_MIGEMO_H
    jdmigemo::close();
#endif

    // アイコンマネージャ削除
    ICON::delete_icon_manager();
}


// 通常のセッション保存
void JDWinMain::save_session()
{
#ifdef _DEBUG
    std::cout << "JDWinMain::save_session\n";
#endif

    if( m_core ) m_core->save_session();
}


void JDWinMain::hide()
{
#ifdef _DEBUG
    std::cout << "JDWinMain::hide\n";
#endif

    // GNOME環境の時に閉じるとウィンドウが最小化する時があるので
    // state_eventをキャンセルする
    m_cancel_state_event = true;

    Gtk::Widget::hide();
}


int JDWinMain::get_x_win()
{
    return SESSION::get_x_win_main();
}

int JDWinMain::get_y_win()
{
    return SESSION::get_y_win_main();
}

void JDWinMain::set_x_win( const int x )
{
    SESSION::set_x_win_main( x );
}

void JDWinMain::set_y_win( const int y )
{
    SESSION::set_y_win_main( y );
}

int JDWinMain::get_width_win()
{
    return SESSION::get_width_win_main();
}

int JDWinMain::get_height_win()
{
    return SESSION::get_height_win_main();
}

void JDWinMain::set_width_win( const int width )
{
    SESSION::set_width_win_main( width );
}

void JDWinMain::set_height_win( const int height )
{
    SESSION::set_height_win_main( height );
}

bool JDWinMain::is_focus_win()
{
    return SESSION::is_focus_win_main();
}

void JDWinMain::set_focus_win( const bool set )
{
    SESSION::set_focus_win_main( set );
}


bool JDWinMain::is_maximized_win()
{
    return SESSION::is_maximized_win_main();
}


void JDWinMain::set_maximized_win( const bool set )
{
    SESSION::set_maximized_win_main( set );
}


bool JDWinMain::is_iconified_win()
{
    return SESSION::is_iconified_win_main();
}

void JDWinMain::set_iconified_win( const bool set )
{
    SESSION::set_iconified_win_main( set );
}


bool JDWinMain::is_full_win()
{
    return SESSION::is_full_win_main();
}

void JDWinMain::set_full_win( const bool set )
{
    SESSION::set_full_win_main( set );
}


bool JDWinMain::is_shown_win()
{
    return SESSION::is_shown_win_main();
}


void JDWinMain::set_shown_win( const bool set )
{
    SESSION::set_shown_win_main( set );
}




bool JDWinMain::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "JDWinMain::on_delete_event\n";
#endif

    // GNOME環境の時に閉じるとウィンドウが最小化する時があるので
    // state_eventをキャンセルする
    m_cancel_state_event = true;
    return SKELETON::JDWindow::on_delete_event( event );
}


// 最大、最小化
bool JDWinMain::on_window_state_event( GdkEventWindowState* event )
{
    // キャンセル ( JDWinMain::on_delete_even() の説明を参照せよ )
    if( m_cancel_state_event ) {
#ifdef _DEBUG
        std::cout << "JDWinMain::on_window_state_event cancel" << std::endl;
#endif
        return Gtk::Window::on_window_state_event( event );
    }

    return SKELETON::JDWindow::on_window_state_event( event );
}


bool JDWinMain::on_configure_event( GdkEventConfigure* event )
{
    // キャンセル ( JDWinMain::on_delete_event() の説明を参照せよ )
    if( m_cancel_state_event ) {
#ifdef _DEBUG
        std::cout << "JDWinMain::on_configure_event cancel" << std::endl;
#endif
        return Gtk::Window::on_configure_event( event );
    }

    return SKELETON::JDWindow::on_configure_event( event );
}


bool JDWinMain::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDWinMain::on_button_press_event\n";
#endif

    // マウスジェスチャ
    m_control.MG_start( event );

    return SKELETON::JDWindow::on_button_press_event( event );
}


bool JDWinMain::on_button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDWinMain::on_button_release_event\n";
#endif

    const bool ret =  SKELETON::JDWindow::on_button_release_event( event );

    /// マウスジェスチャ
    const int mg = m_control.MG_end( event );

    // マウスジェスチャ
    if( mg != CONTROL::None ) operate_win( mg );

    return ret;
}


//
// マウスが動いた
//
bool JDWinMain::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
    std::cout << "JDWinMain::on_motion_notify_event\n";
#endif

    /// マウスジェスチャ
    m_control.MG_motion( event );

    return SKELETON::JDWindow::on_motion_notify_event( event );
}


bool JDWinMain::operate_win( const int control )
{
    return CONTROL::operate_common( control, std::string(), nullptr );
}
