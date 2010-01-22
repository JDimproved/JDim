// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "openurldiag.h"
#include "command.h"

using namespace CORE;

OpenURLDialog::OpenURLDialog( const std::string& url )
    : SKELETON::PrefDiag( NULL, url, true, false, true ),
      m_label_url( true, "URL ：" )
{
    m_label_url.set_text( url );
    set_activate_entry( m_label_url );

    get_vbox()->pack_start( m_label_url, Gtk::PACK_SHRINK );

    set_title( "URLを開く" );
    resize( 600, 1 );

    set_default_response( Gtk::RESPONSE_OK );
    show_all_children();

    m_label_url.grab_focus();
}


void OpenURLDialog::slot_ok_clicked()
{
    CORE::core_set_command( "open_url", m_label_url.get_text() );
}
