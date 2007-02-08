// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imagewin.h"

#include "session.h"
#include "dndmanager.h"
#include "command.h"


using namespace IMAGE;

#define IMGWIN_FOLDSIZE 10


ImageWin::ImageWin()
    : m_boot( true ), m_folded( false )
{
    // サイズ設定
    int x = SESSION::get_img_x();
    int y = SESSION::get_img_y();
    int w = SESSION::get_img_width();
    m_height = SESSION::get_img_height();
    m_maximized = SESSION::mes_maximized();

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << x << " " << y << " " << w << " " << m_height << std::endl;
#endif

    move( x, y );
    resize( w, m_height );

    if( m_maximized ) maximize();

    m_scrwin.set_size_request( 0, 0 );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    add( m_vbox );
    show_all_children();

    property_window_position().set_value( Gtk::WIN_POS_NONE );
    if( CORE::get_toplevel() ) set_transient_for( *dynamic_cast< Gtk::Window* >( CORE::get_toplevel() ) );
}



ImageWin::~ImageWin()
{
    // ウィンドウサイズを保存
    int width, height;;
    int x = 0;
    int y = 0;

    get_size( width, height );
    if( m_folded ) height = m_height;

    if( get_window() ) get_window()->get_root_origin( x, y );

#ifdef _DEBUG
    std::cout << "ImageWin::~ImageWin window size : x = " << x << " y = " << y << " w = " << width << " h = " << height
              << " max = " << m_maximized << std::endl;
#endif

    if( ! m_maximized ){
        SESSION::set_img_x( x );
        SESSION::set_img_y( y );
        SESSION::set_img_width( width );
        SESSION::set_img_height( height );
    }
    SESSION::set_img_maximized( m_maximized );
}



void ImageWin::pack_remove( bool unpack, Gtk::Widget& tab, Gtk::Widget& view )
{
#ifdef _DEBUG
    std::cout << "ImageWin::pack_remove remove - " << unpack << std::endl;
#endif

    m_vbox.pack_remove_start( unpack, tab, Gtk::PACK_SHRINK );

    if( unpack ){

        // ScrolledWindow は Gtk::Viewport を作って widget を add するときがあるので注意
        Gtk::Viewport *vport = dynamic_cast< Gtk::Viewport* >( m_scrwin.get_child() );
        if( vport ) vport->remove();
        else m_scrwin.remove();
    }
    else m_scrwin.add( view );

    m_vbox.pack_remove_start( unpack, m_scrwin );

    if( ! unpack ) m_vbox.show_all_children();
}



bool ImageWin::on_expose_event( GdkEventExpose* event )
{
#ifdef _DEBUG
    if( m_boot ) std::cout << "ImageWin::on_expose_event\n";
#endif

    // 最初の expose イベントが来たら起動完了とする
    if( m_boot ){

        m_boot = false;

        // ImageAdmin 経由で Coreにブートが終わったことを知らせるため
        // ダミーコマンドを送る
        IMAGE::get_admin()->set_command( "imgwin_boot_fin" );
    }

    return Gtk::Window::on_expose_event( event );
}



void ImageWin::focus_in()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_in\n";
#endif

    // 展開
    if( m_folded ){

        resize( get_width(), m_height );
        m_folded = false;
    }

    present();
}


void ImageWin::focus_out()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_out\n";
#endif

    // ポップアップメニューを表示しているかD&D中はfocus_outしない
    if( SESSION::is_popupmenu_shown() ) return;
    if( CORE::get_dnd_manager()->now_dnd() ) return;

    // 折り畳み
    if( ! m_folded ){
        m_height = get_height();
        resize( get_width(), IMGWIN_FOLDSIZE );
        m_folded = true;
    }

}


bool ImageWin::on_focus_in_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_in_event in = " << event->in << std::endl;
#endif

    if( ! m_boot ) CORE::core_set_command( "switch_image" );
    m_focus = true;

    return Gtk::Window::on_focus_in_event( event );
}


bool ImageWin::on_focus_out_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_out_event in = " << event->in << std::endl;
#endif

    focus_out();
    m_focus = false;

    return Gtk::Window::on_focus_out_event( event );
}



bool ImageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_delete_event\n";
#endif

    hide();
    CORE::core_set_command( "switch_leftview" );    

    return true;
}



bool ImageWin::on_window_state_event( GdkEventWindowState* event )
{
    m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;

#ifdef _DEBUG
    std::cout << "ImageWin::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     

    return Gtk::Window::on_window_state_event( event );
}


bool ImageWin::on_configure_event( GdkEventConfigure* event )
{
    // リサイズ中
    if( ! m_focus ) m_height = get_height();

#ifdef _DEBUG
    std::cout << "ImageWin::on_configure_event height = " << m_height << std::endl;
#endif     

    return Gtk::Window::on_configure_event( event );
}
