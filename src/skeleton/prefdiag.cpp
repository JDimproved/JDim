// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "prefdiag.h"

#include "command.h"
#include "session.h"
#include "dispatchmanager.h"
#include "global.h"

using namespace SKELETON;

PrefDiag::PrefDiag( Gtk::Window* parent, const std::string& url, const bool add_cancel, const bool add_apply )
    : Gtk::Dialog(), m_url( url ), m_bt_ok( NULL ), m_bt_apply( Gtk::Stock::APPLY )
{
    // ディスパッチ管理をダイアログ用に切り替える
    CORE::enable_dialog_dispmanager();

    if( add_apply ){
        m_bt_apply.signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_apply_clicked ) );
        get_action_area()->pack_start( m_bt_apply );

    }

    if( add_cancel ){
        add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
        ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_cancel_clicked ) );
    }

    m_bt_ok = add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
    m_bt_ok->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );

    if( parent ) set_transient_for( *parent );
    else set_transient_for( *CORE::get_mainwindow() );
}


PrefDiag::~PrefDiag()
{
    m_conn_timer.disconnect();

    // ディスパッチ管理をメインルーチン用に切り替える
    CORE::disable_dialog_dispmanager();
}


//
// okボタンをフォーカス
//
void PrefDiag::grab_ok()
{
    if( ! m_bt_ok ) return;

    m_bt_ok->set_flags( Gtk::CAN_DEFAULT );
    m_bt_ok->grab_default();
    m_bt_ok->grab_focus();
}


int PrefDiag::run(){

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &PrefDiag::slot_timeout), 0 );
    m_conn_timer = Glib::signal_timeout().connect( slot_timeout, TIMER_TIMEOUT );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

    return ret;
}
