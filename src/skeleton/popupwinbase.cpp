// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "popupwinbase.h"

#include "command.h"


using namespace SKELETON;


PopupWinBase::PopupWinBase( bool draw_frame )
    : Gtk::Window( Gtk::WINDOW_POPUP )
    , m_draw_frame( draw_frame )
{
    if( m_draw_frame ) set_border_width( 1 );

    if ( auto main_window = CORE::get_mainwindow() ) {
        set_transient_for( *main_window );
    }
}


PopupWinBase::~PopupWinBase() noexcept = default;


bool PopupWinBase::on_draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
    const bool ret = Gtk::Window::on_draw( cr );
    if( m_draw_frame ) {
        Gdk::Cairo::set_source_rgba( cr, Gdk::RGBA( "black" ) );
        cr->set_line_width( 1.0 );
        cr->rectangle( 0.0, 0.0, get_width(), get_height() );
        cr->stroke();
    }
    return ret;
}


bool PopupWinBase::on_configure_event( GdkEventConfigure* event )
{
    const bool ret = Gtk::Window::on_configure_event( event );
    m_sig_configured.emit( get_width(), get_height() );

    return ret;
}
