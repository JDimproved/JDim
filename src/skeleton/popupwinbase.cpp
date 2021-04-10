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
    if( m_draw_frame ) {
        set_border_width( 1 );

        auto provider = Gtk::CssProvider::create();
        provider->load_from_data( "window { border: 1px solid black; }" );
        get_style_context()->add_provider( provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
    }

    if ( auto main_window = CORE::get_mainwindow() ) {
        set_transient_for( *main_window );
    }
}


bool PopupWinBase::on_configure_event( GdkEventConfigure* event )
{
    const bool ret = Gtk::Window::on_configure_event( event );
    m_sig_configured.emit( get_width(), get_height() );

    return ret;
}
