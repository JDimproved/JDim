// ライセンス: GPL2

// パスワード設定ダイアログ

#ifndef _PASSWDPREF_H
#define _PASSWDPREF_H

#include "skeleton/prefdiag.h"

#include "login2ch.h"
#include "loginbe.h"
#include "loginacorn.h"

#include "jdlib/miscutil.h"

enum
{
    BOXSPACING = 8
};

namespace CORE
{
    constexpr const char* kViewConcealSymbolic = "view-conceal-symbolic";
    constexpr const char* kViewRevealSymbolic = "view-reveal-symbolic";

    // 2chログイン用
    class PasswdFrame2ch : public Gtk::Grid
    {
        Gtk::Label m_label_sid_2ch;
        Gtk::Label m_label_sid_2ch_value;

        Gtk::Label m_label_id;
        Gtk::Label m_label_passwd;

      public:

        Gtk::Entry entry_id;
        Gtk::Entry entry_passwd;

        PasswdFrame2ch()
            : m_label_sid_2ch{ "SID:" }
            , m_label_sid_2ch_value{ "2chのログインは現在サポート中止しています。" }
            , m_label_id{ "ユーザID(_I):", true }
            , m_label_passwd{ "パスワード(_P):", true }
        {
            property_margin() = 16;
            set_column_spacing( 10 );
            set_row_spacing( 16 );
            set_vexpand( true );

            m_label_id.set_halign( Gtk::ALIGN_START );
            m_label_id.set_mnemonic_widget( entry_id );
            entry_id.set_hexpand( true );

            m_label_passwd.set_halign( Gtk::ALIGN_START );
            m_label_passwd.set_mnemonic_widget( entry_passwd );
            entry_passwd.set_hexpand( true );
            entry_passwd.set_visibility( false );

            m_label_sid_2ch.set_halign( Gtk::ALIGN_START );
            m_label_sid_2ch.set_valign( Gtk::ALIGN_END );
            m_label_sid_2ch_value.set_ellipsize( Pango::ELLIPSIZE_END );
            m_label_sid_2ch_value.set_halign( Gtk::ALIGN_START );
            m_label_sid_2ch_value.set_valign( Gtk::ALIGN_END );
            m_label_sid_2ch_value.set_hexpand( true );
            m_label_sid_2ch_value.set_selectable( true );

            attach( m_label_id, 0, 0, 1, 1 );
            attach( entry_id, 1, 0, 1, 1 );
            attach( m_label_passwd, 0, 1, 1, 1 );
            attach( entry_passwd, 1, 1, 1, 1 );
            attach( m_label_sid_2ch, 0, 2, 1, 1 );
            attach( m_label_sid_2ch_value, 1, 2, 1, 1 );

            entry_passwd.set_icon_from_icon_name( kViewConcealSymbolic, Gtk::ENTRY_ICON_SECONDARY );
            entry_passwd.signal_icon_release().connect(
                sigc::bind( &PasswdFrame2ch::slot_icon_release, &entry_passwd ) );
        }

        /** @brief アイコンをクリックしてマウスボタンを離したときにパスワードとアイコンの状態を切り替える
         *
         * @details PasswdFrameBe でも使うため static メンバー関数にする。
         * アイコンは現在の状態を表す (非表示の状態では非表示のアイコン、表示の状態では表示のアイコン)。
         * @param[in,out] entry パスワード入力欄
         */
        static void slot_icon_release( Gtk::EntryIconPosition, const GdkEventButton*, Gtk::Entry* entry )
        {
            if( entry->get_visibility() ) {
                entry->set_icon_from_icon_name( kViewConcealSymbolic, Gtk::ENTRY_ICON_SECONDARY );
                entry->set_visibility( false );
            }
            else {
                entry->set_icon_from_icon_name( kViewRevealSymbolic, Gtk::ENTRY_ICON_SECONDARY );
                entry->set_visibility( true );
            }
        }
    };

    // BEログイン用
    class PasswdFrameBe : public Gtk::Grid
    {
        Gtk::Label m_label_dmdm;
        Gtk::Label m_label_dmdm_value;
        Gtk::Label m_label_mdmd;
        Gtk::Label m_label_mdmd_value;

        Gtk::Label m_label_id;
        Gtk::Label m_label_passwd;

      public:

        Gtk::Entry entry_id;
        Gtk::Entry entry_passwd;

        PasswdFrameBe()
            : m_label_dmdm{ "DMDM:" }
            , m_label_dmdm_value{ CORE::get_loginbe()->get_sessionid() }
            , m_label_mdmd{ "MDMD:" }
            , m_label_mdmd_value{ CORE::get_loginbe()->get_sessiondata() }
            , m_label_id{ "メールアドレス(_I):", true }
            , m_label_passwd{ "パスワード(_P):", true }
        {
            property_margin() = 16;
            set_column_spacing( 10 );
            set_row_spacing( 8 );

            entry_id.set_hexpand( true );
            m_label_id.set_mnemonic_widget( entry_id );

            entry_passwd.set_hexpand( true );
            entry_passwd.set_visibility( false );
            m_label_passwd.set_mnemonic_widget( entry_passwd );

            m_label_id.set_halign( Gtk::ALIGN_START );
            m_label_passwd.set_halign( Gtk::ALIGN_START );
            m_label_dmdm.set_halign( Gtk::ALIGN_START );
            m_label_dmdm_value.set_halign( Gtk::ALIGN_START );
            m_label_mdmd.set_halign( Gtk::ALIGN_START );
            m_label_mdmd_value.set_halign( Gtk::ALIGN_START );

            m_label_dmdm_value.set_ellipsize( Pango::ELLIPSIZE_END );
            m_label_mdmd_value.set_ellipsize( Pango::ELLIPSIZE_END );
            m_label_dmdm_value.set_selectable( true );
            m_label_mdmd_value.set_selectable( true );

            attach( m_label_id, 0, 0, 1, 1 );
            attach( entry_id, 1, 0, 1, 1 );
            attach( m_label_passwd, 0, 1, 1, 1 );
            attach( entry_passwd, 1, 1, 1, 1 );
            attach( m_label_dmdm, 0, 2, 1, 1 );
            attach( m_label_dmdm_value, 1, 2, 1, 1 );
            attach( m_label_mdmd, 0, 3, 1, 1 );
            attach( m_label_mdmd_value, 1, 3, 1, 1 );

            entry_passwd.set_icon_from_icon_name( kViewConcealSymbolic, Gtk::ENTRY_ICON_SECONDARY );
            entry_passwd.signal_icon_release().connect(
                sigc::bind( &PasswdFrame2ch::slot_icon_release, &entry_passwd ) );
        }
    };

    // どんぐりシステム メールアドレス登録警備員のログイン用
    class PasswdFrameAcorn : public Gtk::Grid
    {
        Gtk::Label m_label_account;
        Gtk::Label m_label_account_value;
        Gtk::Label m_label_cookie;
        Gtk::Label m_label_cookie_value;

        Gtk::Label m_label_id;
        Gtk::Label m_label_passwd;

      public:

        Gtk::Entry entry_id;
        Gtk::Entry entry_passwd;

        PasswdFrameAcorn()
            : m_label_account{ "アカウント:" }
            , m_label_account_value{ CORE::get_loginacorn()->get_sessionid() }
            , m_label_cookie{ "クッキー(acorn):" }
            , m_label_cookie_value{ CORE::get_loginacorn()->get_sessiondata() }
            , m_label_id{ "メールアドレス(_I):", true }
            , m_label_passwd{ "パスワード(_P):", true }
        {
            constexpr const char* mail_addr_tooltip = "警備員アカウントを登録したGmailアドレスを設定してください。";

            property_margin() = 16;
            set_column_spacing( 10 );
            set_row_spacing( 8 );

            entry_id.set_hexpand( true );
            entry_id.set_tooltip_text( mail_addr_tooltip );
            m_label_id.set_mnemonic_widget( entry_id );

            entry_passwd.set_hexpand( true );
            entry_passwd.set_visibility( false );
            m_label_passwd.set_mnemonic_widget( entry_passwd );

            m_label_id.set_halign( Gtk::ALIGN_START );
            m_label_id.set_tooltip_text( mail_addr_tooltip );
            m_label_passwd.set_halign( Gtk::ALIGN_START );
            m_label_account.set_halign( Gtk::ALIGN_START );
            m_label_account_value.set_halign( Gtk::ALIGN_START );
            m_label_cookie.set_halign( Gtk::ALIGN_START );
            m_label_cookie_value.set_halign( Gtk::ALIGN_START );

            m_label_account_value.set_ellipsize( Pango::ELLIPSIZE_END );
            m_label_cookie_value.set_ellipsize( Pango::ELLIPSIZE_END );
            m_label_account_value.set_selectable( true );
            m_label_cookie_value.set_selectable( true );

            attach( m_label_id, 0, 0, 1, 1 );
            attach( entry_id, 1, 0, 1, 1 );
            attach( m_label_passwd, 0, 1, 1, 1 );
            attach( entry_passwd, 1, 1, 1, 1 );
            attach( m_label_account, 0, 2, 1, 1 );
            attach( m_label_account_value, 1, 2, 1, 1 );
            attach( m_label_cookie, 0, 3, 1, 1 );
            attach( m_label_cookie_value, 1, 3, 1, 1 );

            entry_passwd.set_icon_from_icon_name( kViewConcealSymbolic, Gtk::ENTRY_ICON_SECONDARY );
            entry_passwd.signal_icon_release().connect(
                sigc::bind( &PasswdFrame2ch::slot_icon_release, &entry_passwd ) );
        }
    };


    class PasswdPref : public SKELETON::PrefDiag
    {

        Gtk::Notebook m_notebook;

        PasswdFrame2ch m_frame_2ch;
        PasswdFrameBe m_frame_be;
        PasswdFrameAcorn m_frame_acorn;

        // OK押した
        void slot_ok_clicked() override
        {
            // 2ch
            CORE::get_login2ch()->set_username( MISC::utf8_trim( m_frame_2ch.entry_id.get_text().raw() ) );
            CORE::get_login2ch()->set_passwd( MISC::utf8_trim( m_frame_2ch.entry_passwd.get_text().raw() ) );

            // BE
            CORE::get_loginbe()->set_username( MISC::utf8_trim( m_frame_be.entry_id.get_text().raw() ) );
            CORE::get_loginbe()->set_passwd( MISC::utf8_trim( m_frame_be.entry_passwd.get_text().raw() ) );

            // どんぐりシステム メールアドレス登録警備員
            CORE::get_loginacorn()->set_username( MISC::utf8_trim( m_frame_acorn.entry_id.get_text().raw() ) );
            CORE::get_loginacorn()->set_passwd( MISC::utf8_trim( m_frame_acorn.entry_passwd.get_text().raw() ) );
        }

      public:

        PasswdPref( Gtk::Window* parent, const std::string& url )
            : SKELETON::PrefDiag( parent, url )
        {
            // 2chログイン用
            m_frame_2ch.entry_id.set_text( CORE::get_login2ch()->get_username() );
            m_frame_2ch.entry_passwd.set_text( CORE::get_login2ch()->get_passwd() );

            set_activate_entry( m_frame_2ch.entry_id );
            set_activate_entry( m_frame_2ch.entry_passwd );

            // beログイン用
            m_frame_be.entry_id.set_text( CORE::get_loginbe()->get_username() );
            m_frame_be.entry_passwd.set_text( CORE::get_loginbe()->get_passwd() );

            set_activate_entry( m_frame_be.entry_id );
            set_activate_entry( m_frame_be.entry_passwd );

            // どんぐりシステム メールアドレス登録警備員
            m_frame_acorn.entry_id.set_text( CORE::get_loginacorn()->get_username() );
            m_frame_acorn.entry_passwd.set_text( CORE::get_loginacorn()->get_passwd() );

            set_activate_entry( m_frame_acorn.entry_id );
            set_activate_entry( m_frame_acorn.entry_passwd );

            m_notebook.append_page( m_frame_2ch, "2ch" );
            m_notebook.append_page( m_frame_be, "BE" );
            m_notebook.append_page( m_frame_acorn, "どんぐりシステム" );
            get_content_area()->pack_start( m_notebook );

            set_title( "パスワード設定" );
            show_all_children();
            set_default_size( 450, -1 );
        }

        ~PasswdPref() noexcept override = default;
    };

}

#endif
