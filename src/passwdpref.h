// ライセンス: GPL2

// パスワード設定ダイアログ

#ifndef _PASSWDPREF_H
#define _PASSWDPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include "login2ch.h"
#include "loginbe.h"

#include "jdlib/miscutil.h"

enum
{
    BOXSPACING = 8
};

namespace CORE
{
    // 2chログイン用
    class PasswdFrame2ch : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        SKELETON::LabelEntry m_label_sid_2ch;

      public:

        SKELETON::LabelEntry entry_id;
        SKELETON::LabelEntry entry_passwd;

      PasswdFrame2ch()
      : m_label_sid_2ch( false, "SID： ", LOGIN::get_login2ch()->get_sessionid() ),
        entry_id( true, "ID(_I)： " ), entry_passwd( true, "パスワード(_P)： " )
        {
            m_vbox.set_border_width( BOXSPACING );

            entry_id.set_border_width( BOXSPACING );
            m_vbox.pack_start( entry_id );

            entry_passwd.set_border_width( BOXSPACING );
            entry_passwd.set_visibility( false );
            m_vbox.pack_start( entry_passwd, Gtk::PACK_SHRINK );

            m_label_sid_2ch.set_border_width( BOXSPACING );
            m_vbox.pack_start( m_label_sid_2ch );

            set_label( "2chログイン用" );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    // BEログイン用
    class PasswdFrameBe : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::Label m_label;

      public:

        SKELETON::LabelEntry entry_id;
        SKELETON::LabelEntry entry_passwd;

      PasswdFrameBe()
        : m_label( "パスワードではなく認証コードを入れて下さい" ),
        entry_id( true, "メールアドレス(_E)： " ), entry_passwd( true, "認証コード(_A)： " )
        {
            m_vbox.set_border_width( BOXSPACING );

            entry_id.set_border_width( BOXSPACING );
            m_vbox.pack_start( entry_id );

            entry_passwd.set_border_width( BOXSPACING );
            entry_passwd.set_visibility( false );
            m_vbox.pack_start( entry_passwd, Gtk::PACK_SHRINK );

            m_vbox.pack_start( m_label );

            set_label( "BE ログイン用" );
            set_border_width( BOXSPACING );
            add( m_vbox );
        }
    };

    class PasswdPref : public SKELETON::PrefDiag
    {
        PasswdFrame2ch m_frame_2ch;
        PasswdFrameBe m_frame_be;

        // OK押した
        virtual void slot_ok_clicked(){

            // 2ch
            LOGIN::get_login2ch()->set_username( MISC::remove_space( m_frame_2ch.entry_id.get_text() ) );
            LOGIN::get_login2ch()->set_passwd( MISC::remove_space( m_frame_2ch.entry_passwd.get_text() ) );

            // BE
            LOGIN::get_loginbe()->set_username( MISC::remove_space( m_frame_be.entry_id.get_text() ) );
            LOGIN::get_loginbe()->set_passwd( MISC::remove_space( m_frame_be.entry_passwd.get_text() ) );
        }

      public:

        PasswdPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url )
        , m_frame_2ch(), m_frame_be()
        {
            // 2chログイン用
            m_frame_2ch.entry_id.set_text( LOGIN::get_login2ch()->get_username() );
            m_frame_2ch.entry_passwd.set_text( LOGIN::get_login2ch()->get_passwd() );

            // beログイン用
            m_frame_be.entry_id.set_text( LOGIN::get_loginbe()->get_username() );
            m_frame_be.entry_passwd.set_text( LOGIN::get_loginbe()->get_passwd() );

            get_vbox()->set_spacing( BOXSPACING );
            get_vbox()->pack_start( m_frame_2ch );
            get_vbox()->pack_start( m_frame_be );

            set_title( "パスワード設定" );
            show_all_children();
        }

        virtual ~PasswdPref(){}
    };

}

#endif
