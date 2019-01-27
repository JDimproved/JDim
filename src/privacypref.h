// ライセンス: GPL2

// プライバシー設定ダイアログ

#ifndef _PRIVACYPREF_H
#define _PRIVACYPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/msgdiag.h"

#include "command.h"

namespace CORE
{
    class PrivacyPref : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::CheckButton m_bt_board;
        Gtk::CheckButton m_bt_thread;
        Gtk::CheckButton m_bt_close;
        Gtk::CheckButton m_bt_search;
        Gtk::CheckButton m_bt_name;
        Gtk::CheckButton m_bt_mail;

        Gtk::HBox m_hbox_selectall;
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
        : SKELETON::PrefDiag( parent, url ),
        m_bt_board( "板履歴(_B)", true ),
        m_bt_thread( "スレ履歴(_T)", true ),
        m_bt_close( "最近閉じたスレの履歴(_R)", true ),
        m_bt_search( "検索履歴(_F)", true ),
        m_bt_name( "書き込みビューの名前履歴(_N)", true ),
        m_bt_mail( "書き込みビューのメール履歴(_E)", true ),
        m_bt_selectall( "全て選択(_A)", true )
        {
            m_vbox.set_spacing( 8 );
            m_vbox.set_border_width( 8 );
            m_vbox.pack_start( m_bt_thread );
            m_vbox.pack_start( m_bt_board );
            m_vbox.pack_start( m_bt_close );
            m_vbox.pack_start( m_bt_search );
            m_vbox.pack_start( m_bt_name );
            m_vbox.pack_start( m_bt_mail );

            m_bt_selectall.signal_clicked().connect( sigc::mem_fun( *this, &PrivacyPref::slot_selectall ) );
            m_hbox_selectall.pack_start( m_bt_selectall, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox_selectall, Gtk::PACK_SHRINK );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_vbox, Gtk::PACK_SHRINK );

            set_title( "プライバシー情報の消去" );
            show_all_children();
        }

        ~PrivacyPref() noexcept {}
    };

}

#endif
