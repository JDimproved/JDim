// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "window.h"

#include "global.h"

using namespace SKELETON;

JDWindow::JDWindow()
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
      m_maximized( false )
{
    // ステータスバー
#if GTKMMVER <= 240
    m_statbar.pack_start( m_mginfo );
#else
    m_statbar.pack_start( m_label_stat, Gtk::PACK_SHRINK );
    m_statbar.pack_end( m_mginfo, Gtk::PACK_SHRINK );
    m_mginfo.set_width_chars( MAX_MG_LNG * 2 + 16 );
    m_mginfo.set_justify( Gtk::JUSTIFY_LEFT );
#endif
    m_statbar.show_all_children();

    m_stat_scrbar.add( m_statbar );
    m_stat_scrbar.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_stat_scrbar.set_size_request( 8 );

    add( m_vbox );
}


JDWindow::~JDWindow()
{}


void JDWindow::set_spacing( int space )
{
    m_vbox.set_spacing( space );
}


void JDWindow::set_maximized( bool set )
{
    m_maximized = set;
    if( m_maximized ) maximize();
}


void JDWindow::pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    m_vbox.pack_remove_start( unpack, child, options, padding );
    if( ! unpack ) m_vbox.show_all_children();
}


void JDWindow::pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    m_vbox.pack_remove_end( unpack, child, options, padding );
    if( ! unpack ) m_vbox.show_all_children();
}


// ステータスバー表示
void JDWindow::set_status( const std::string& stat )
{
#if GTKMMVER <= 240
    m_statbar.push( stat );
#else
    m_label_stat.set_text( stat );
#endif
}


// マウスジェスチャ表示
void JDWindow::set_mginfo( const std::string& mginfo )
{
    m_mginfo.set_text( mginfo );
}


bool JDWindow::on_window_state_event( GdkEventWindowState* event )
{
    m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;

#ifdef _DEBUG
    std::cout << "JDWindow::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     

    return Gtk::Window::on_window_state_event( event );
}
