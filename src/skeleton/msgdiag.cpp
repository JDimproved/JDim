// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgdiag.h"

#include "command.h"
#include "session.h"
#include "dispatchmanager.h"
#include "global.h"

#include <glib/gi18n.h>


using namespace SKELETON;


MsgDiag::MsgDiag( Gtk::Window& parent,
                  const Glib::ustring& message,
                  bool use_markup,
                  Gtk::MessageType type,
                  Gtk::ButtonsType buttons,
                  bool modal )
    : Gtk::MessageDialog( parent, message, use_markup, type, buttons, modal )
{}

// parent がポインタの時は  nullptr かどうかで場合分け
MsgDiag::MsgDiag( Gtk::Window* parent,
                  const Glib::ustring& message,
                  bool use_markup,
                  Gtk::MessageType type,
                  Gtk::ButtonsType buttons,
                  bool modal )
    : Gtk::MessageDialog( message, use_markup, type, buttons, modal )
{
    if( parent ) set_transient_for( *parent );
    else set_transient_for( *CORE::get_mainwindow() );

    // tab でラベルにフォーカスが移らないようにする
    const std::vector< Gtk::Widget* > area = get_message_area()->get_children();
    Gtk::Label* const primary_label = dynamic_cast< Gtk::Label* >( area.front() );
    if( primary_label ) {
        primary_label->set_can_focus( false );
    }
}


MsgDiag::~MsgDiag() noexcept = default;


void MsgDiag::add_default_button( const Gtk::StockID& stock_id, const int id )
{
    Gtk::Button* button = Gtk::manage( new Gtk::Button( stock_id ) );
    add_default_button( button, id );
}


void MsgDiag::add_default_button( const Glib::ustring& label, const int id )
{
    Gtk::Button* button = Gtk::manage( new Gtk::Button( label, true ) );
    add_default_button( button, id );
}


void MsgDiag::add_default_button( Gtk::Widget* button, const int id )
{
    if( ! button ) return;

    add_action_widget( *button, id );
    button->show();

    button->set_can_default( true );

    button->grab_default();
    button->grab_focus();
}


int MsgDiag::run()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &MsgDiag::slot_timeout), 0 );
    m_conn_timer = JDLIB::Timeout::connect( slot_timeout, TIMER_TIMEOUT );

    int ret = Gtk::MessageDialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "MsgDiag::run fin\n";
#endif

    return ret;
}


void MsgDiag::show()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::show\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    Gtk::MessageDialog::show();
}


void MsgDiag::hide()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::hide\n";
#endif

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

    Gtk::MessageDialog::hide();
}


// タイマーのslot関数
bool MsgDiag::slot_timeout( int timer_number )
{
    // メインループが停止していて dispatcher が働かないため
    // タイマーから強制的に実行させる
    CORE::get_dispmanager()->slot_dispatch();

    return true;
}



/////////////////////////////////////


MsgCheckDiag::MsgCheckDiag( Gtk::Window* parent,
                            const Glib::ustring& message,
                            const Glib::ustring& message_check,
                            Gtk::MessageType type, Gtk::ButtonsType buttons, const int default_response )
    : SKELETON::MsgDiag( parent, message, false, type, Gtk::BUTTONS_NONE, false )
    , m_chkbutton( message_check, true )
{
    const int mrg = 16;
                
    Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox );
    hbox->pack_start( m_chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
    get_content_area()->pack_start( *hbox, Gtk::PACK_SHRINK );

    Gtk::Button* button = nullptr;

    if( buttons == Gtk::BUTTONS_OK ){

        button = Gtk::manage( new Gtk::Button( Gtk::Stock::OK ) );
        add_default_button( button, Gtk::RESPONSE_OK );
    }
    else if( buttons == Gtk::BUTTONS_OK_CANCEL ){
        add_button( Gtk::Stock::NO, Gtk::RESPONSE_CANCEL );

        button = Gtk::manage( new Gtk::Button( Gtk::Stock::OK ) );
        add_default_button( button, Gtk::RESPONSE_OK );
    }
    else if( buttons == Gtk::BUTTONS_YES_NO ){

        if( default_response == Gtk::RESPONSE_NO ){

            button = Gtk::manage( new Gtk::Button( Gtk::Stock::NO ) );
            add_default_button( button, Gtk::RESPONSE_NO );

            add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );
        }
        else{

            add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );

            add_default_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );
        }
    }

    show_all_children();
}


MsgCheckDiag::~MsgCheckDiag() noexcept = default;


/////////////////////////////////////



MsgOverwriteDiag::MsgOverwriteDiag( Gtk::Window* parent )
    : SKELETON::MsgDiag( parent, "ファイルが存在します。ファイル名を変更して保存しますか？", 
                         false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE )
{
    add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );
    add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );
    add_button( "上書き", OVERWRITE_YES );
    add_button( "すべていいえ", OVERWRITE_NO_ALL );
    add_button( "すべて上書き", OVERWRITE_YES_ALL );
}


MsgOverwriteDiag::~MsgOverwriteDiag() noexcept = default;
