// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "addetcdialog.h"

#include "command.h"
#include "session.h"


using namespace BBSLIST;


AddEtcDialog::AddEtcDialog( const bool move, const std::string& _url, const std::string& _name, const std::string& _id, const std::string& _passwd )
    : Gtk::Dialog(),
      m_entry_name( true, "板名(_N)：", _name ),
      m_entry_url( true, "アドレス(_U)：", _url ),
      m_entry_id( true, "ID(_I)：", _id ),
      m_entry_pw( true, "パスワード(_P)：", _passwd )
{
    resize( 600, 1 );

    m_vbox.set_spacing( 8 );
    m_vbox.set_border_width( 8 );
    m_vbox.add( m_entry_id );
    m_vbox.add( m_entry_pw );
    m_frame.set_label( "BASIC認証" );
    m_frame.add( m_vbox );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_entry_name );
    get_vbox()->pack_start( m_entry_url );
    get_vbox()->pack_start( m_frame );

    add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );

    if( move ){
        add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
        set_title( "外部板編集" );
    }
    else{
        add_button( Gtk::Stock::ADD, Gtk::RESPONSE_OK );
        set_title( "外部板追加" );
    }

    show_all_children();
}


AddEtcDialog::~AddEtcDialog()
{}



int AddEtcDialog::run()
{
#ifdef _DEBUG
    std::cout << "AddEtcDialog::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "AddEtcDialog::run fin\n";
#endif

    return ret;
}
