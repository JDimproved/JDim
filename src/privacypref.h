// ライセンス: GPL2

// プライバシー設定ダイアログ

#ifndef _PRIVACYPREF_H
#define _PRIVACYPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/msgdiag.h"

#include "jdlib/gtk_compat.h"

#include "command.h"

namespace CORE
{
    class PrivacyPref : public SKELETON::PrefDiag
    {
        Gtk::Box m_vbox;
        Gtk::CheckButton m_bt_board;
        Gtk::CheckButton m_bt_thread;
        Gtk::CheckButton m_bt_close;
        Gtk::CheckButton m_bt_search;
        Gtk::CheckButton m_bt_name;
        Gtk::CheckButton m_bt_mail;

        Gtk::Box m_hbox_selectall;
        Gtk::Button m_bt_selectall;

        void slot_selectall()
        {
            m_bt_board.set_active( true );
            m_bt_thread.set_active( true );
            m_bt_close.set_active( true );
            m_bt_search.set_active( true );
            m_bt_name.set_active( true );
            m_bt_mail.set_active( true );
        }

        // OK押した
        void slot_ok_clicked() override
        {
            if( m_bt_board.get_active() ) CORE::core_set_command( "clear_board" );
            if( m_bt_thread.get_active() ) CORE::core_set_command( "clear_thread" );
            if( m_bt_close.get_active() ) CORE::core_set_command( "clear_closed_thread" );
            if( m_bt_search.get_active() ) CORE::core_set_command( "clear_search" );
            if( m_bt_name.get_active() ) CORE::core_set_command( "clear_name" );
            if( m_bt_mail.get_active() ) CORE::core_set_command( "clear_mail" );
        }

      public:

        PrivacyPref( Gtk::Window* parent, const std::string& url )
            : SKELETON::PrefDiag( parent, url )
            , m_vbox{ Gtk::ORIENTATION_VERTICAL, 0 }
            , m_bt_board( "板履歴(_B)", true )
            , m_bt_thread( "スレ履歴(_T)", true )
            , m_bt_close( "最近閉じたスレの履歴(_R)", true )
            , m_bt_search( "検索履歴(_F)", true )
            , m_bt_name( "書き込みビューの名前履歴(_N)", true )
            , m_bt_mail( "書き込みビューのメール履歴(_E)", true )
            , m_hbox_selectall{ Gtk::ORIENTATION_HORIZONTAL, 0 }
            , m_bt_selectall( "全て選択(_A)", true )
        {
            m_vbox.set_spacing( 8 );
            m_vbox.set_border_width( 8 );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_thread );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_board );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_close );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_search );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_name );
            JDLIB::compat::box_append_expand( m_vbox, m_bt_mail );

            m_bt_selectall.signal_clicked().connect( sigc::mem_fun( *this, &PrivacyPref::slot_selectall ) );
            JDLIB::compat::box_append_shrink( m_hbox_selectall, m_bt_selectall );
            JDLIB::compat::box_append_shrink( m_vbox, m_hbox_selectall );

            get_content_area()->set_spacing( 8 );
            JDLIB::compat::box_append_shrink( *get_content_area(), m_vbox );

            set_title( "プライバシー情報の消去" );
            show_all_children();
        }

        ~PrivacyPref() noexcept override = default;
    };

}

#endif
