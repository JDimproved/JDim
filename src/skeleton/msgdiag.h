// ライセンス: GPL2

// メッセージダイアログの基底クラス

#ifndef _MSGDIAG_H
#define _MSGDIAG_H

#include <gtkmm.h>

#include "command.h"

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

        MsgDiag( const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false)
        : Gtk::MessageDialog( *CORE::get_mainwindow(), message, use_markup, type, buttons, modal )
        {}

        virtual ~MsgDiag(){}
    };
}

#endif
