// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "aboutconfigdiag.h"

#include "jdlib/miscutil.h"

using namespace CONFIG;

AboutConfigDiagStr::AboutConfigDiagStr( Gtk::Window* parent, std::string* value, const std::string& defaultval )
    : SKELETON::PrefDiag( parent, "", true ), m_value( value ), m_defaultval( defaultval ),
      m_button_default( "デフォルト" )
{
    resize( 600, 1 );

    m_entry.set_text( *value );
    m_hbox.pack_start( m_entry );

    m_button_default.signal_clicked().connect( sigc::mem_fun( *this, &AboutConfigDiagStr::slot_default ) );
    m_hbox.pack_start( m_button_default, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox );

    set_activate_entry( m_entry );

    set_title( "文字列設定" );
    show_all_children();
}


void AboutConfigDiagStr::slot_ok_clicked()
{
    if( m_value ) *m_value = m_entry.get_text();
}


void AboutConfigDiagStr::slot_default()
{
    m_entry.set_text( m_defaultval );
}


///////////////////////////////////////////


AboutConfigDiagInt::AboutConfigDiagInt( Gtk::Window* parent, int* value, const int defaultval )
    : SKELETON::PrefDiag( parent, "", true ), m_value( value ), m_defaultval( defaultval ),
      m_button_default( "デフォルト" )
{
    resize( 200, 1 );

    m_entry.set_text( MISC::itostr( *value ) );
    m_hbox.pack_start( m_entry );

    m_button_default.signal_clicked().connect( sigc::mem_fun( *this, &AboutConfigDiagInt::slot_default ) );
    m_hbox.pack_start( m_button_default, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox );

    set_activate_entry( m_entry );

    set_title( "整数値設定" );
    show_all_children();
}


void AboutConfigDiagInt::slot_ok_clicked()
{
    if( m_value ) *m_value = atoi( m_entry.get_text().c_str() );
}


void AboutConfigDiagInt::slot_default()
{
    m_entry.set_text( MISC::itostr( m_defaultval ) );
}


///////////////////////////////////////////


AboutConfigDiagBool::AboutConfigDiagBool( Gtk::Window* parent, bool* value, const bool defaultval )
    : SKELETON::PrefDiag( parent, "", true ), m_value( value ), m_defaultval( defaultval ),
      m_radio_true( "はい" ), m_radio_false( "いいえ" ),  m_button_default( "デフォルト" )
{
    m_radio_true.set_group( m_radiogroup );
    m_radio_false.set_group( m_radiogroup );
    
    if( *value ) m_radio_true.set_active();
    else m_radio_false.set_active();

    m_hbox.pack_start( m_radio_true );
    m_hbox.pack_start( m_radio_false );

    m_button_default.signal_clicked().connect( sigc::mem_fun( *this, &AboutConfigDiagBool::slot_default ) );
    m_hbox.pack_start( m_button_default, Gtk::PACK_SHRINK );


    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox );

    set_title( "真偽値設定" );
    show_all_children();
}


void AboutConfigDiagBool::slot_ok_clicked()
{
    if( m_value ) *m_value = m_radio_true.get_active();
}


void AboutConfigDiagBool::slot_default()
{
    if( m_defaultval ) m_radio_true.set_active();
    else m_radio_false.set_active();
}
