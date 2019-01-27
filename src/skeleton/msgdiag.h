// ライセンス: GPL2

// メッセージダイアログの基底クラス

#ifndef _MSGDIAG_H
#define _MSGDIAG_H

#include <gtkmm.h>

#include "jdlib/timeout.h"

namespace SKELETON
{
    class MsgDiag : public Gtk::MessageDialog
    {

        JDLIB::Timeout* m_conn_timer;

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

        ~MsgDiag() noexcept {}

        void add_default_button( const Gtk::StockID& stock_id, const int id );
        void add_default_button( const Glib::ustring& label, const int id );
        void add_default_button( Gtk::Widget* button, const int id );

        virtual int run();

        void show();
        void hide();

      private:

        // タイマーのslot関数
        bool slot_timeout( int timer_number );
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
                      Gtk::ButtonsType buttons = Gtk::BUTTONS_OK,
                      const int default_response = -1
            );

        Gtk::CheckButton& get_chkbutton(){ return m_chkbutton; }
    };


    /////////////////////////////////////

    // 上書きチェックダイアログ

    enum
    {
        OVERWRITE_YES = Gtk::RESPONSE_YES + 100,
        OVERWRITE_YES_ALL = Gtk::RESPONSE_YES + 200,
        OVERWRITE_NO_ALL = Gtk::RESPONSE_NO + 200
    };

    class MsgOverwriteDiag : public SKELETON::MsgDiag
    {
      public:

        MsgOverwriteDiag( Gtk::Window* parent );
    };
}

#endif
