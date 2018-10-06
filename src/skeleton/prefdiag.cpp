// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "prefdiag.h"
#include "label_entry.h"

#include "command.h"
#include "session.h"
#include "dispatchmanager.h"
#include "global.h"

using namespace SKELETON;

PrefDiag::PrefDiag( Gtk::Window* parent, const std::string& url, const bool add_cancel, const bool add_apply, const bool add_open )
    : Gtk::Dialog(), m_url( url ), m_bt_ok( NULL ), m_bt_apply( Gtk::Stock::APPLY )
{
    if( add_apply ){
        m_bt_apply.signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_apply_clicked ) );
        get_action_area()->pack_start( m_bt_apply );

    }

    if( add_cancel ){
        add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
        ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_cancel_clicked ) );
    }

    if( add_open ) m_bt_ok = add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK );
    else m_bt_ok = add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );

    m_bt_ok->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );

    if( parent ) set_transient_for( *parent );
    else set_transient_for( *CORE::get_mainwindow() );
}


PrefDiag::~PrefDiag()
{
    if( m_conn_timer != NULL )
    {
        delete m_conn_timer;
    }
}


//
// okボタンをフォーカス
//
void PrefDiag::grab_ok()
{
    if( ! m_bt_ok ) return;

#if GTKMM_CHECK_VERSION(2,18,0)
    m_bt_ok->set_can_default( true );
#else
    m_bt_ok->set_flags( Gtk::CAN_DEFAULT );
#endif
    m_bt_ok->grab_default();
    m_bt_ok->grab_focus();
}


//
// Entry、LabelEntryがactiveになったときにOKでダイアログを終了させる
//
void PrefDiag::set_activate_entry( Gtk::Entry& entry )
{
    entry.signal_activate().connect( sigc::mem_fun( *this, &PrefDiag::slot_activate_entry ) );
}

void PrefDiag::set_activate_entry( LabelEntry& entry )
{
    entry.signal_activate().connect( sigc::mem_fun( *this, &PrefDiag::slot_activate_entry ) );
}


void PrefDiag::slot_activate_entry()
{
#ifdef _DEBUG
    std::cout << "PrefDiag::slot_activate_entry\n";
#endif

    slot_ok_clicked();
    response( Gtk::RESPONSE_OK );
}


int PrefDiag::run(){

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &PrefDiag::slot_timeout), 0 );
    m_conn_timer = JDLIB::Timeout::connect( slot_timeout, TIMER_TIMEOUT );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

    return ret;
}


// タイマーのslot関数
bool PrefDiag::slot_timeout( int timer_number )
{
    // メインループが停止していて dispatcher が働かないため
    // タイマーから強制的に実行させる
    CORE::get_dispmanager()->slot_dispatch();

    timeout();

    return true;
}
