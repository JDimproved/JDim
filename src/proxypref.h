// ライセンス: GPL2

// プロキシ設定ダイアログ

#ifndef _PROXYPREF_H
#define _PROXYPREF_H

#include "skeleton/prefdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

namespace CORE
{
    class ProxyFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;
        Gtk::Label m_label_host;
        Gtk::Label m_label_port;

      public:

        Gtk::CheckButton ckbt;
        Gtk::Entry entry_host;
        Gtk::Entry entry_port;

        ProxyFrame( const std::string& title )
        : m_label_host( "host" ), m_label_port( "port" ), ckbt( "使用する" )
        {
            m_hbox.set_spacing( 8 );
            m_hbox.pack_start( ckbt, Gtk::PACK_SHRINK );

            m_hbox.pack_start( m_label_host, Gtk::PACK_SHRINK );
            m_hbox.pack_start( entry_host );

            m_hbox.pack_start( m_label_port, Gtk::PACK_SHRINK );
            m_hbox.pack_start( entry_port, Gtk::PACK_SHRINK );

            m_hbox.set_border_width( 8 );
            m_vbox.set_spacing( 8 );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class ProxyPref : public SKELETON::PrefDiag
    {
        // 2ch読み込み用
        ProxyFrame m_frame_2ch;

        // 2ch書き込み用
        ProxyFrame m_frame_2ch_w;

        // 一般用
        ProxyFrame m_frame_data;

        // OK押した
        virtual void slot_ok_clicked(){

            // 2ch
            if( m_frame_2ch.ckbt.get_active() ) CONFIG::set_use_proxy_for2ch( true );
            else CONFIG::set_use_proxy_for2ch( false );
            CONFIG::set_proxy_for2ch( MISC::remove_space( m_frame_2ch.entry_host.get_text() ) );
            CONFIG::set_proxy_port_for2ch( atoi( m_frame_2ch.entry_port.get_text().c_str() ) );

            // 2ch書き込み用
            if( m_frame_2ch_w.ckbt.get_active() ) CONFIG::set_use_proxy_for2ch_w( true );
            else CONFIG::set_use_proxy_for2ch_w( false );
            CONFIG::set_proxy_for2ch_w( MISC::remove_space( m_frame_2ch_w.entry_host.get_text() ) );
            CONFIG::set_proxy_port_for2ch_w( atoi( m_frame_2ch_w.entry_port.get_text().c_str() ) );

            // 一般
            if( m_frame_data.ckbt.get_active() ) CONFIG::set_use_proxy_for_data( true );
            else CONFIG::set_use_proxy_for_data( false );
            CONFIG::set_proxy_for_data( MISC::remove_space( m_frame_data.entry_host.get_text() ) );
            CONFIG::set_proxy_port_for_data( atoi( m_frame_data.entry_port.get_text().c_str() ) );
        }

      public:

        ProxyPref( const std::string& url )
        : SKELETON::PrefDiag( url )
        , m_frame_2ch( "2ch読み込み用" ), m_frame_2ch_w( "2ch書き込み用" ), m_frame_data( "その他のサーバ用" )
        {
            // 2ch用
            if( CONFIG::get_use_proxy_for2ch() ) m_frame_2ch.ckbt.set_active( true );
            else m_frame_2ch.ckbt.set_active( false );
            m_frame_2ch.entry_host.set_text( CONFIG::get_proxy_for2ch() );
            m_frame_2ch.entry_port.set_text( MISC::itostr( CONFIG::get_proxy_port_for2ch() ) );

            // 2ch書き込み用
            if( CONFIG::get_use_proxy_for2ch_w() ) m_frame_2ch_w.ckbt.set_active( true );
            else m_frame_2ch_w.ckbt.set_active( false );
            m_frame_2ch_w.entry_host.set_text( CONFIG::get_proxy_for2ch_w() );
            m_frame_2ch_w.entry_port.set_text( MISC::itostr( CONFIG::get_proxy_port_for2ch_w() ) );

            // 一般用
            if( CONFIG::get_use_proxy_for_data() ) m_frame_data.ckbt.set_active( true );
            else m_frame_data.ckbt.set_active( false );
            m_frame_data.entry_host.set_text( CONFIG::get_proxy_for_data() );
            m_frame_data.entry_port.set_text( MISC::itostr( CONFIG::get_proxy_port_for_data() ) );

            get_vbox()->set_spacing( 4 );
            get_vbox()->pack_start( m_frame_2ch );
            get_vbox()->pack_start( m_frame_2ch_w );
            get_vbox()->pack_start( m_frame_data );

            set_title( "プロキシ設定" );
            show_all_children();
        }

        virtual ~ProxyPref(){}
    };

}

#endif
