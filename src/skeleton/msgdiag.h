// ライセンス: GPL2

// メッセージダイアログの基底クラス

#ifndef _MSGDIAG_H
#define _MSGDIAG_H

#include <gtkmm.h>

#include "command.h"
#include "session.h"

namespace SKELETON
{
    class MsgDiag : public Gtk::MessageDialog
    {
      public:

        MsgDiag( Gtk::Window& parent,
                 const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false)
        : Gtk::MessageDialog( parent, message, use_markup, type, buttons, modal )
        {}

        // parent がポインタの時は  NULL かどうかで場合分け
        MsgDiag( Gtk::Window* parent,
                 const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false)
        : Gtk::MessageDialog( message, use_markup, type, buttons, modal )
        {
            if( parent ) set_transient_for( *parent );
            else set_transient_for( *CORE::get_mainwindow() );
        }

        virtual ~MsgDiag(){}

        virtual int run(){

            SESSION::set_dialog_shown( true );
            CORE::core_set_command( "dialog_shown" );

            int ret = Gtk::MessageDialog::run();

            SESSION::set_dialog_shown( false );
            CORE::core_set_command( "dialog_hidden" );

            return ret;
        }

    };
}

#endif
