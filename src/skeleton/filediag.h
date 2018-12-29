// ライセンス: GPL2

// ファイル選択ダイアログの基底クラス

#ifndef _FILEDIAG_H
#define _FILEDIAG_H

#include <gtkmm.h>

#include "command.h"
#include "session.h"

namespace SKELETON
{
    class FileDiag : public Gtk::FileChooserDialog
    {
        Gtk::FileChooserAction m_action;

      public:

        // ボタン追加 + saveボタンをデフォルトボタンにセット
        void add_buttons(){

            add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );

            if( m_action == Gtk::FILE_CHOOSER_ACTION_OPEN ) add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT );
            else add_button( Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT );

            set_default_response( Gtk::RESPONSE_ACCEPT );
        }

        FileDiag( Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN )
        : Gtk::FileChooserDialog( parent, title, action ), m_action( action )
        {
            add_buttons();
        }

        // parent がポインタの時は  NULL かどうかで場合分け
        FileDiag( Gtk::Window* parent, const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN )
        : Gtk::FileChooserDialog( title, action ), m_action( action )
        {
            add_buttons();

            if( parent ) set_transient_for( *parent );
            else set_transient_for( *CORE::get_mainwindow() );
        }

        ~FileDiag() noexcept {}

        virtual int run(){

            SESSION::set_dialog_shown( true );
            CORE::core_set_command( "dialog_shown" );

            int ret = Gtk::FileChooserDialog::run();

            SESSION::set_dialog_shown( false );
            CORE::core_set_command( "dialog_hidden" );

            return ret;
        }

    };
}

#endif
