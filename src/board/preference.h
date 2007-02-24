// ライセンス: GPL2

#ifndef _BOARD_PREFERENCES_H
#define _BOARD_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"

namespace BOARD
{
    class ProxyFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

      public:

        Gtk::RadioButton rd_global, rd_disable, rd_local;
        SKELETON::LabelEntry entry_host;
        SKELETON::LabelEntry entry_port;

        ProxyFrame( const std::string& title )
        : rd_global( "全体設定を使用する" ), rd_disable( "全体設定を無効にする" ), rd_local( "ローカル設定を使用する" ),
        entry_host( true, "ホスト：" ), entry_port( true, "ポート：" )
        {
            Gtk::RadioButton::Group grp = rd_global.get_group();
            rd_disable.set_group( grp );
            rd_local.set_group( grp );

            m_hbox.set_spacing( 8 );
            m_hbox.set_border_width( 8 );
            m_hbox.pack_start( entry_host );
            m_hbox.pack_start( entry_port, Gtk::PACK_SHRINK );

            m_vbox.set_spacing( 8 );
            m_vbox.set_border_width( 8 );
            m_vbox.pack_start( rd_global, Gtk::PACK_SHRINK );
            m_vbox.pack_start( rd_disable, Gtk::PACK_SHRINK );
            m_vbox.pack_start( rd_local, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class Preferences : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        Gtk::VBox m_vbox;

        // 書き込み時のデフォルト名とメール
        Gtk::Frame m_frame_write;
        Gtk::VBox m_vbox_write;
        Gtk::HBox m_hbox_write;
        SKELETON::LabelEntry m_entry_writename;
        SKELETON::LabelEntry m_entry_writemail;
        Gtk::CheckButton m_check_noname; // 名無し書き込みチェック

        // クッキー & hana
        Gtk::Frame m_frame_cookie;
        Gtk::HBox m_hbox_cookie;
        SKELETON::EditView m_edit_cookies;
        Gtk::VBox m_vbox_cookie;
        Gtk::Button m_button_cookie;

        // プロキシ
        Gtk::VBox m_vbox_proxy;
        Gtk::Label m_label_proxy;
        ProxyFrame m_proxy_frame;
        ProxyFrame m_proxy_frame_w;

        // 情報
        Gtk::VBox m_vbox_info;
        Gtk::Label m_label_name;
        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_cache;

        SKELETON::LabelEntry m_label_noname;
        SKELETON::LabelEntry m_label_line;
        SKELETON::LabelEntry m_label_byte;

        // あぼーん
        SKELETON::EditView m_edit_thread, m_edit_word, m_edit_regex;

        // SETTING.TXT
        SKELETON::EditView m_edit_settingtxt;

      public:
        Preferences( Gtk::Window* parent, const std::string& url );

      private:
        void slot_delete_cookie();
        virtual void slot_ok_clicked();
    };

}

#endif
