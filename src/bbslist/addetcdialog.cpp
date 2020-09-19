// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "addetcdialog.h"

#include "command.h"
#include "session.h"


using namespace BBSLIST;


AddEtcDialog::AddEtcDialog( const bool move, const std::string& url, const std::string& name, const std::string& id, const std::string& passwd )
    : SKELETON::PrefDiag( nullptr, url, true ),
      m_entry_name( true, "板名(_N)：", name ),
      m_entry_url( true, "アドレス(_U)：", url ),
      m_entry_id( true, "ID(_I)：", id ),
      m_entry_pw( true, "パスワード(_P)：", passwd )
{
    resize( 600, 1 );

    m_vbox.set_spacing( 8 );
    m_vbox.set_border_width( 8 );
    m_vbox.add( m_entry_id );
    m_vbox.add( m_entry_pw );

    set_activate_entry( m_entry_id );
    set_activate_entry( m_entry_pw );

    m_frame.set_label( "BASIC認証" );
    m_frame.add( m_vbox );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_entry_name );
    get_content_area()->pack_start( m_entry_url );
    get_content_area()->pack_start( m_frame );

    set_activate_entry( m_entry_name );
    set_activate_entry( m_entry_url );

    if( move ){
        set_title( "外部板編集" );
    }
    else{
        set_title( "外部板追加" );
    }

    show_all_children();
}


AddEtcDialog::~AddEtcDialog() noexcept = default;
