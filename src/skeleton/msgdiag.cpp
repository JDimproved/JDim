// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgdiag.h"

#include "command.h"
#include "session.h"

#include <gtk/gtkmessagedialog.h>
#include <gtk/gtklabel.h>

using namespace SKELETON;


MsgDiag::MsgDiag( Gtk::Window& parent,
                  const Glib::ustring& message,
                  bool use_markup,
                  Gtk::MessageType type,
                  Gtk::ButtonsType buttons,
                  bool modal )
    : Gtk::MessageDialog( parent, message, use_markup, type, buttons, modal )
{}

// parent がポインタの時は  NULL かどうかで場合分け
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

    // tab でラベルにフォーカスが移らないようにする ( messagedialog.ccg をハックした )
    Gtk::Widget* wdt = Glib::wrap( gobj()->label );
    if( wdt ){
        Gtk::Label* label = dynamic_cast< Gtk::Label* >( wdt );
        if( label ) label->property_can_focus() = false;
    }
}


int MsgDiag::run()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

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



/////////////////////////////////////


MsgCheckDiag::MsgCheckDiag( Gtk::Window* parent,
                            const Glib::ustring& message,
                            const Glib::ustring& message_check,
                            Gtk::MessageType type, Gtk::ButtonsType buttons )
    : SKELETON::MsgDiag( parent, message, false, type, Gtk::BUTTONS_NONE, false )
    , m_chkbutton( message_check, true )
{
    const int mrg = 16;
                
    Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox );
    hbox->pack_start( m_chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
    get_vbox()->pack_start( *hbox, Gtk::PACK_SHRINK );

    Gtk::Button* okbutton = NULL;

    if( buttons == Gtk::BUTTONS_OK ){
        okbutton = Gtk::manage( new Gtk::Button( Gtk::Stock::OK ) );
        add_action_widget( *okbutton, Gtk::RESPONSE_OK );
    }
    else if( buttons == Gtk::BUTTONS_OK_CANCEL ){
        add_button( Gtk::Stock::NO, Gtk::RESPONSE_CANCEL );
        okbutton = Gtk::manage( new Gtk::Button( Gtk::Stock::OK ) );
        add_action_widget( *okbutton, Gtk::RESPONSE_OK );
    }
    else if( buttons == Gtk::BUTTONS_YES_NO ){
        add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );
        okbutton = Gtk::manage( new Gtk::Button( Gtk::Stock::YES ) );
        add_action_widget( *okbutton, Gtk::RESPONSE_YES );
    }

    if( okbutton ){
        okbutton->set_flags( Gtk::CAN_DEFAULT );
        okbutton->grab_default();
        okbutton->grab_focus();
    }

    show_all_children();
}
