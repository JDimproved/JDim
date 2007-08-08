// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgdiag.h"

#include "command.h"
#include "session.h"

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
