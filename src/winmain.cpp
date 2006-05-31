// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "winmain.h"
#include "command.h"
#include "core.h"
#include "session.h"

#include "icons/icon_jd16.h"
#include "icons/icon_jd32.h"
#include "icons/icon_jd48.h"

#include "jdlib/loader.h"

WinMain::WinMain( bool init )
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL )
    , m_core( 0 ), m_maximized( 0 )
{
#ifdef _DEBUG
    std::cout << "WinMain::WinMain init = " << init << std::endl;
#endif    

    setlocale( LC_ALL, "ja_JP.UTF-8" );

    // GLIBのスレッドシステム初期化
    if( !Glib::thread_supported() ) Glib::thread_init();
    assert( Glib::thread_supported() );

    // アイコン
    std::list< Glib::RefPtr< Gdk::Pixbuf > > list_icons;
    list_icons.push_back( Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16_png ), icon_jd16_png ) );
    list_icons.push_back( Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32_png ), icon_jd32_png ) );
    list_icons.push_back( Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48_png ), icon_jd48_png ) );
    Gtk::Window::set_default_icon_list( list_icons );
    
    // セッション回復
    SESSION::init_session();

    // サイズ設定
    resize( SESSION::width(), SESSION::height() );
    move( SESSION::x(), SESSION::y() );
    if( SESSION::maximized() ) maximize();

    set_modal( false );
    property_window_position().set_value( Gtk::WIN_POS_NONE );
    set_resizable( true );
    property_destroy_with_parent().set_value( false );

    // 後はcoreを作って任せる
    m_core = new class CORE::Core( *this );
    m_core->run( init );
}


WinMain::~WinMain()
{
#ifdef _DEBUG
    std::cout << "WinMain::~WinMain\n";
#endif

    save_session();
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

        if( get_window() ){

            // ウィンドウのサイズや位置を保存
            int x, y, width, height;;
            get_window()->get_root_origin( x, y );
            get_size( width, height );
            x = MAX( 0, x );
            y = MAX( 0, y );

#ifdef _DEBUG
            std::cout << "window size : x = " << x << " y = " << y << " w = " << width << " h = " << height
                      << " max = " << m_maximized << std::endl;
#endif

            if( !m_maximized ){
                SESSION::set_x( x );
                SESSION::set_y( y );
                SESSION::set_width( width );
                SESSION::set_height( height );
            }
            SESSION::set_maximized( m_maximized );
        }

        SESSION::save_session();

        JDLIB::check_loader_alive();
    }
}


bool WinMain::on_window_state_event( GdkEventWindowState* event )
{
    m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
    
#ifdef _DEBUG
    std::cout << "WinMain::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     

    // タブ幅調整
    CORE::core_set_command( "adjust_tabwidth" );

    return Gtk::Window::on_window_state_event( event );
}
