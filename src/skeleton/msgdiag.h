// ライセンス: GPL2

// メッセージダイアログの基底クラス

#ifndef _MSGDIAG_H
#define _MSGDIAG_H

#include <gtkmm.h>

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
                 bool modal = false);

        // parent がポインタの時は  NULL かどうかで場合分け
        MsgDiag( Gtk::Window* parent,
                 const Glib::ustring& message,
                 bool use_markup = false,
                 Gtk::MessageType type = Gtk::MESSAGE_INFO,
                 Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                 bool modal = false);

        virtual ~MsgDiag(){}

        virtual int run();

        void show();
        void hide();
    };


    /////////////////////////////////////

    // チェックボタン付き
    class MsgCheckDiag : public SKELETON::MsgDiag
    {
        Gtk::CheckButton m_chkbutton;

    public:

        MsgCheckDiag( Gtk::Window* parent,
                      const Glib::ustring& message,
                      const Glib::ustring& message_check,
                      Gtk::MessageType type = Gtk::MESSAGE_INFO,
                      Gtk::ButtonsType buttons = Gtk::BUTTONS_OK );

        Gtk::CheckButton& get_chkbutton(){ return m_chkbutton; }
    };
}

#endif
