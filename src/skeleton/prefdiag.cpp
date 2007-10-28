// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "prefdiag.h"

#include "command.h"
#include "session.h"
#include "global.h"

using namespace SKELETON;

PrefDiag::PrefDiag( Gtk::Window* parent, const std::string& url, bool add_cancel, bool add_apply )
    : Gtk::Dialog(), m_url( url ), m_bt_apply( Gtk::Stock::APPLY )
{
    if( add_apply ){
        m_bt_apply.signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_apply_clicked ) );
        get_action_area()->pack_start( m_bt_apply );

    }

    if( add_cancel ){
        add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
        ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_cancel_clicked ) );
    }

    add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK )
    ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );

    if( parent ) set_transient_for( *parent );
    else set_transient_for( *CORE::get_mainwindow() );
}


int PrefDiag::run(){

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &PrefDiag::slot_timeout), 0 );
    sigc::connection conn = Glib::signal_timeout().connect( slot_timeout, TIMER_TIMEOUT );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

    return ret;
}
