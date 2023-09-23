// ライセンス: GPL2

// プロキシ設定ダイアログ

#ifndef _PROXYPREF_H
#define _PROXYPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

namespace CORE
{
    constexpr const char kSendCookieTooltip[] = "JDimからプロキシへサイトのクッキーを送信します。\n"
                                                "プロキシのクッキー処理にあわせて設定してください。";
    constexpr const char* kFallbackProxyNotice =
        "スレッドの途中からプロキシ接続に代わるとdatファイルの整合性が取れなくなり"
        "\"416 Requested Range Not Satisfiable\"の通信エラーが出ることがあります。"
        "その場合はスレ情報を消さずに再取得を行ってください。"
        "このオプションは実験的なサポートのため変更または廃止の可能性があります。";

    class ProxyFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

        Gtk::Box m_hbox_port; ///< "ポート番号"のラベルと入力欄を一つにまとめる
        Gtk::Label m_label_port; ///< "ポート番号"のラベル

      public:

        Gtk::CheckButton ckbt;
        Gtk::CheckButton send_cookie_check;
        SKELETON::LabelEntry entry_host;
        Gtk::Entry entry_port; ///< "ポート番号"の入力欄

        ProxyFrame( const std::string& title, const Glib::ustring& ckbt_label, const Glib::ustring& send_label,
                    const Glib::ustring& host_label, const Glib::ustring& port_label )
            : m_hbox_port{ Gtk::ORIENTATION_HORIZONTAL, 0 }
            , m_label_port{ port_label, true }
            , ckbt( ckbt_label, true )
            , send_cookie_check( send_label, true )
            , entry_host( true, host_label)
        {
            send_cookie_check.set_tooltip_text( kSendCookieTooltip );
            // ポート番号の最大値が収まる幅に調整する
            entry_port.set_width_chars( 5 );
            entry_port.set_max_length( 5 );
            entry_port.set_hexpand( false );
            m_label_port.set_mnemonic_widget( entry_port );

            m_hbox_port.pack_start( m_label_port, 0, 0, false );
            m_hbox_port.pack_start( entry_port, 0, 0, false );

            m_hbox.set_spacing( 8 );
            m_hbox.pack_start( ckbt, Gtk::PACK_SHRINK );
            m_hbox.pack_start( send_cookie_check, Gtk::PACK_SHRINK );
            m_hbox.pack_start( entry_host );
            m_hbox.pack_start( m_hbox_port, Gtk::PACK_SHRINK );

            m_hbox.set_border_width( 8 );
            m_vbox.set_spacing( 8 );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class ProxyFrameFallbackOption : public Gtk::Frame
    {
        Gtk::Box m_vbox;
        Gtk::Box m_hbox;

        Gtk::Box m_hbox_port; ///< "ポート番号"のラベルと入力欄を一つにまとめる
        Gtk::Label m_label_port; ///< "ポート番号"のラベル

        Gtk::Box m_vbox_exp_option;
        Gtk::Box m_hbox_fallback_proxy;
        Glib::RefPtr<Glib::Binding> m_binding_notice;
        Gtk::ToggleButton m_toggle_notice;
        Gtk::Revealer m_revealer_notice;

      public:

        Gtk::CheckButton ckbt;
        Gtk::CheckButton send_cookie_check;

        Gtk::CheckButton fallback_proxy_check;
        Gtk::Label notice_label;

        SKELETON::LabelEntry entry_host;
        Gtk::Entry entry_port; ///< "ポート番号"の入力欄

        ProxyFrameFallbackOption( const std::string& title, const Glib::ustring& ckbt_label,
                                  const Glib::ustring& send_label, const Glib::ustring& fallback_label,
                                  const Glib::ustring& host_label, const Glib::ustring& port_label )
            : m_vbox( Gtk::ORIENTATION_VERTICAL, 0 )
            , m_hbox( Gtk::ORIENTATION_HORIZONTAL, 8 )
            , m_hbox_port{ Gtk::ORIENTATION_HORIZONTAL, 0 }
            , m_label_port{ port_label, true }
            , m_vbox_exp_option( Gtk::ORIENTATION_VERTICAL, 0 )
            , m_hbox_fallback_proxy( Gtk::ORIENTATION_HORIZONTAL, 0 )
            , m_toggle_notice( "注意事項" )
            , ckbt( ckbt_label, true )
            , send_cookie_check( send_label, true )
            , fallback_proxy_check( fallback_label, true )
            , notice_label( kFallbackProxyNotice, false )
            , entry_host( true, host_label)
        {
            send_cookie_check.set_tooltip_text( kSendCookieTooltip );
            // ポート番号の最大値が収まる幅に調整する
            entry_port.set_width_chars( 5 );
            entry_port.set_max_length( 5 );
            entry_port.set_hexpand( false );
            m_label_port.set_mnemonic_widget( entry_port );

            m_hbox_port.pack_start( m_label_port, 0, 0, false );
            m_hbox_port.pack_start( entry_port, 0, 0, false );

            fallback_proxy_check.set_halign( Gtk::ALIGN_START );
            m_toggle_notice.set_halign( Gtk::ALIGN_END );

            m_revealer_notice.add( notice_label );
            m_revealer_notice.set_reveal_child( false );
            notice_label.set_line_wrap( true );
            notice_label.set_margin_end( 8 );
            notice_label.set_margin_start( 8 );

            m_binding_notice = Glib::Binding::bind_property( m_toggle_notice.property_active(),
                                                             m_revealer_notice.property_reveal_child() );

            m_hbox.pack_start( ckbt, Gtk::PACK_SHRINK );
            m_hbox.pack_start( send_cookie_check, Gtk::PACK_SHRINK );
            m_hbox.pack_start( entry_host );
            m_hbox.pack_start( m_hbox_port, Gtk::PACK_SHRINK );

            m_hbox_fallback_proxy.set_hexpand( true );
            m_hbox_fallback_proxy.set_margin_start( 8 );
            m_hbox_fallback_proxy.set_margin_end( 8 );
            m_hbox_fallback_proxy.pack_start( fallback_proxy_check, Gtk::PACK_SHRINK );
            m_hbox_fallback_proxy.pack_end( m_toggle_notice, Gtk::PACK_SHRINK );
            m_vbox_exp_option.pack_start( m_hbox_fallback_proxy, Gtk::PACK_SHRINK );
            m_vbox_exp_option.pack_start( m_revealer_notice, Gtk::PACK_SHRINK );

            m_hbox.set_border_width( 8 );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_vbox_exp_option, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class ProxyPref : public SKELETON::PrefDiag
    {
        Gtk::Label m_label;

        // 2ch読み込み用
        ProxyFrameFallbackOption m_frame_2ch;

        // 2ch書き込み用
        ProxyFrame m_frame_2ch_w;

        // 一般用
        ProxyFrame m_frame_data;

        // OK押した
        void slot_ok_clicked() override
        {
            // 2ch
            CONFIG::set_use_proxy_for2ch( m_frame_2ch.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for2ch( m_frame_2ch.send_cookie_check.get_active() );
            CONFIG::set_proxy_for2ch( MISC::utf8_trim( m_frame_2ch.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for2ch( atoi( m_frame_2ch.entry_port.get_text().c_str() ) );
            CONFIG::set_use_fallback_proxy_for2ch( m_frame_2ch.fallback_proxy_check.get_active() );

            // 2ch書き込み用
            CONFIG::set_use_proxy_for2ch_w( m_frame_2ch_w.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for2ch_w( m_frame_2ch_w.send_cookie_check.get_active() );
            CONFIG::set_proxy_for2ch_w( MISC::utf8_trim( m_frame_2ch_w.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for2ch_w( atoi( m_frame_2ch_w.entry_port.get_text().c_str() ) );

            // 一般
            CONFIG::set_use_proxy_for_data( m_frame_data.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for_data( m_frame_data.send_cookie_check.get_active() );
            CONFIG::set_proxy_for_data( MISC::utf8_trim( m_frame_data.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for_data( atoi( m_frame_data.entry_port.get_text().c_str() ) );
        }

      public:

        ProxyPref( Gtk::Window* parent, const std::string& url )
            : SKELETON::PrefDiag( parent, url )
            , m_label( "認証を行う場合はホスト名を「ユーザID:パスワード@ホスト名」としてください。" )
            , m_frame_2ch(
                "2ch読み込み用", "使用する(_U)", "クッキーを送る(_I)",
                "プロキシを使わない接続で過去ログが見つからなかったときは2ch読み込み用プロキシを使う(_B)",
                "ホスト名(_H)： ", "ポート番号(_P)： " )
            , m_frame_2ch_w( "2ch書き込み用", "使用する(_S)", "クッキーを送る(_J)",
                             "ホスト名(_N)： ", "ポート番号(_R)： " )
            , m_frame_data( "その他のサーバ用(板一覧、外部板、画像など)", "使用する(_E)", "クッキーを送る(_K)",
                            "ホスト名(_A)： ", "ポート番号(_T)： " )
        {
            std::string host;

            // 2ch用
            m_frame_2ch.ckbt.set_active( CONFIG::get_use_proxy_for2ch() );
            m_frame_2ch.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for2ch() );
            if( CONFIG::get_proxy_basicauth_for2ch().empty() ) host = CONFIG::get_proxy_for2ch();
            else host = CONFIG::get_proxy_basicauth_for2ch() + "@" + CONFIG::get_proxy_for2ch();
            m_frame_2ch.entry_host.set_text( host );
            m_frame_2ch.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for2ch() ) );
            m_frame_2ch.fallback_proxy_check.set_active( CONFIG::get_use_fallback_proxy_for2ch() );

            set_activate_entry( m_frame_2ch.entry_host );
            set_activate_entry( m_frame_2ch.entry_port );

            // 2ch書き込み用
            m_frame_2ch_w.ckbt.set_active( CONFIG::get_use_proxy_for2ch_w() );
            m_frame_2ch_w.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for2ch_w() );
            if( CONFIG::get_proxy_basicauth_for2ch_w().empty() ) host = CONFIG::get_proxy_for2ch_w();
            else host = CONFIG::get_proxy_basicauth_for2ch_w() + "@" + CONFIG::get_proxy_for2ch_w();
            m_frame_2ch_w.entry_host.set_text( host );
            m_frame_2ch_w.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for2ch_w() ) );

            set_activate_entry( m_frame_2ch_w.entry_host );
            set_activate_entry( m_frame_2ch_w.entry_port );

            // 一般用
            m_frame_data.ckbt.set_active( CONFIG::get_use_proxy_for_data() );
            m_frame_data.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for_data() );
            if( CONFIG::get_proxy_basicauth_for_data().empty() ) host = CONFIG::get_proxy_for_data();
            else host = CONFIG::get_proxy_basicauth_for_data() + "@" + CONFIG::get_proxy_for_data();
            m_frame_data.entry_host.set_text( host );
            m_frame_data.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for_data() ) );

            set_activate_entry( m_frame_data.entry_host );
            set_activate_entry( m_frame_data.entry_port );

            get_content_area()->set_spacing( 4 );
            get_content_area()->pack_start( m_label );
            get_content_area()->pack_start( m_frame_2ch );
            get_content_area()->pack_start( m_frame_2ch_w );
            get_content_area()->pack_start( m_frame_data );

            set_title( "プロキシ設定" );
            // 2ch読み込み用設定にある label のテキストを wrap するためダイアログのサイズを設定しておく
            // 実際はウィジェットの最小サイズを考慮した大きさになる
            set_default_size( 100, 100 );
            show_all_children();
        }

        ~ProxyPref() noexcept = default;
    };

}

#endif
