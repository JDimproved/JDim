// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "popupwinbase.h"

#if GTKMM_CHECK_VERSION(3,0,0)
#include "command.h"
#endif


using namespace SKELETON;


PopupWinBase::PopupWinBase( bool draw_frame )
    : Gtk::Window( Gtk::WINDOW_POPUP )
    , m_draw_frame( draw_frame )
{
#ifdef _DEBUG
    std::cout << "PopupWinBase::PopupWinBase\n";
#endif

    if( m_draw_frame ) set_border_width( 1 );
#if GTKMM_CHECK_VERSION(3,0,0)
    if ( auto main_window = CORE::get_mainwindow() ) {
        set_transient_for( *main_window );
    }
#endif
}


PopupWinBase::~PopupWinBase() noexcept = default;


void PopupWinBase::on_realize()
{
    Gtk::Window::on_realize();

#if !GTKMM_CHECK_VERSION(3,0,0)
    const Glib::RefPtr< Gdk::Window > window = get_window();
    m_gc = Gdk::GC::create( window );
#endif
}


#if GTKMM_CHECK_VERSION(3,0,0)
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
#else
bool PopupWinBase::on_expose_event( GdkEventExpose* event )
{
    const bool ret = Gtk::Window::on_expose_event( event );

    // 枠の描画
    if( m_draw_frame ) {
        m_gc->set_foreground( Gdk::Color( "black" ) );
        get_window()->draw_rectangle( m_gc, false, 0, 0, get_width()-1, get_height()-1 );
    }

    return ret;
}
#endif // GTKMM_CHECK_VERSION(3,0,0)


bool PopupWinBase::on_configure_event( GdkEventConfigure* event )
{
    const bool ret = Gtk::Window::on_configure_event( event );
    m_sig_configured.emit( get_width(), get_height() );

    return ret;
}
