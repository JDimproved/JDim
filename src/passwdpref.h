// ライセンス: GPL2

// パスワード設定ダイアログ

#ifndef _PASSWDPREF_H
#define _PASSWDPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include "login2ch.h"
#include "jdlib/miscutil.h"

namespace CORE
{
    class PasswdFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

      public:

        SKELETON::LabelEntry entry_id;
        SKELETON::LabelEntry entry_passwd;

        PasswdFrame( const std::string& title )
        : entry_id( true, "ID：" ), entry_passwd( true, "パスワード：" )
        {
            m_hbox.set_spacing( 8 );

            m_hbox.pack_start( entry_id );

            entry_passwd.set_visibility( false );
            m_hbox.pack_start( entry_passwd, Gtk::PACK_SHRINK );

            m_hbox.set_border_width( 8 );
            m_vbox.set_spacing( 8 );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class PasswdPref : public SKELETON::PrefDiag
    {
        // 2chログイン用
        PasswdFrame m_frame_2ch;
        SKELETON::LabelEntry m_label_sid_2ch;

        // OK押した
        virtual void slot_ok_clicked(){

            // 2ch
            LOGIN::get_login2ch()->set_username( MISC::remove_space( m_frame_2ch.entry_id.get_text() ) );
            LOGIN::get_login2ch()->set_passwd( MISC::remove_space( m_frame_2ch.entry_passwd.get_text() ) );
        }

      public:

        PasswdPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url )
        , m_frame_2ch( "2chログイン用 ID" )
        , m_label_sid_2ch( false, "SID：", LOGIN::get_login2ch()->get_sessionid() )
        {
            // 2ch用
            m_frame_2ch.entry_id.set_text( LOGIN::get_login2ch()->get_username() );
            m_frame_2ch.entry_passwd.set_text( LOGIN::get_login2ch()->get_passwd() );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_frame_2ch );
            get_vbox()->pack_start( m_label_sid_2ch );

            set_title( "2chログイン設定" );
            show_all_children();
        }

        virtual ~PasswdPref(){}
    };

}

#endif
