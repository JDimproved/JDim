// ライセンス: GPL2

// メッセージダイアログの基底クラス

// 画像ウィンドウを表示している時、ダイアログを消すと画像ウィンドウに
// フォーカスが移ってしまうので、あらかじめ画像ウィンドウをhideして
// おいてからデストラクタでshowする


#ifndef _MSGDIAG_H
#define _MSGDIAG_H

#include <gtkmm.h>

#include "command.h"

namespace SKELETON
{
    class MsgDiag : public Gtk::MessageDialog
    {
        void hide_imagewindow(){ CORE::core_set_command( "hide_imagewindow" ); }
        void show_imagewindow(){ CORE::core_set_command( "show_imagewindow" ); }

      public:

        MsgDiag( Gtk::Window& parent,
                 const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false)
        : Gtk::MessageDialog( parent, message, use_markup, type, buttons, modal )
        {
            hide_imagewindow();
        }

        MsgDiag( const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false)
        : Gtk::MessageDialog( message, use_markup, type, buttons, modal )
        {
            hide_imagewindow();
        }

        virtual ~MsgDiag(){
            show_imagewindow();
        }
    };
}

#endif
