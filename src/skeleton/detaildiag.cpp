// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "detaildiag.h"
#include "view.h"

#include "viewfactory.h"

using namespace SKELETON;

DetailDiag::DetailDiag( Gtk::Window* parent, const std::string& url,
                        const bool add_cancel,
                        const std::string& message, const std::string& tab_message,
                        const std::string& detail_html, const std::string& tab_detail )
    : SKELETON::PrefDiag( parent, url, add_cancel ),
      m_message( message ),
      m_detail( nullptr )
{
    m_message.set_width_chars( 60 );
    m_message.set_line_wrap( true );
    m_message.property_margin() = 8;
    m_message.set_selectable( true );
    m_message.property_can_focus() = false;

    m_detail = CORE::ViewFactory( CORE::VIEW_ARTICLEINFO, get_url() );
    if( ! detail_html.empty() ) m_detail->set_command( "append_html", detail_html );

    m_notebook.append_page( m_message, tab_message );
    m_notebook.append_page( *m_detail, tab_detail );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &DetailDiag::slot_switch_page ) );

    get_content_area()->pack_start( m_notebook );

    show_all_children();
    grab_ok();
}


DetailDiag::~DetailDiag()
{
    if( m_detail ) delete m_detail;
}


void DetailDiag::timeout()
{
    if( m_detail ) m_detail->clock_in();
}


void DetailDiag::slot_switch_page( Gtk::Widget*, guint page )
{
    if( get_notebook().get_nth_page( page ) == m_detail ) m_detail->redraw_view();
}

