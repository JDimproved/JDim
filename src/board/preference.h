// ライセンス: GPL2

#ifndef _BOARD_PREFERENCES_H
#define _BOARD_PREFERENCES_H

#include "skeleton/view.h"
#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"
#include "skeleton/spinbutton.h"

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

        // 実況の更新間隔
        Gtk::HBox m_hbox_live;
        Gtk::Label m_label_live;
        Gtk::CheckButton m_check_live;
        SKELETON::SpinButton m_spin_live;

        // プロキシ
        Gtk::VBox m_vbox_proxy;
        Gtk::Label m_label_proxy;
        ProxyFrame m_proxy_frame;
        ProxyFrame m_proxy_frame_w;

        // 情報
        SKELETON::LabelEntry m_label_name;
        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_cache;

        SKELETON::LabelEntry m_label_noname;
        SKELETON::LabelEntry m_label_line;
        SKELETON::LabelEntry m_label_byte;

        SKELETON::LabelEntry m_entry_max_res;

        SKELETON::LabelEntry m_label_last_access;

        Gtk::HBox m_hbox_modified;
        SKELETON::LabelEntry m_label_modified;
        Gtk::Button m_button_clearmodified;

        Gtk::HBox m_hbox_samba;
        SKELETON::LabelEntry m_label_samba;
        Gtk::Button m_button_clearsamba;

        // あぼーん
        Gtk::Notebook m_notebook_abone;
        Gtk::Label m_label_warning;
        SKELETON::EditView m_edit_id, m_edit_name, m_edit_word, m_edit_regex;

        // スレッドあぼーん
        Gtk::Notebook m_notebook_abone_thread;
        SKELETON::EditView m_edit_thread, m_edit_word_thread, m_edit_regex_thread;

        Gtk::VBox m_vbox_abone_thread;
        Gtk::Label m_label_abone_thread;

        Gtk::HBox m_hbox_number;
        Gtk::Label m_label_number;
        SKELETON::SpinButton m_spin_number;

        Gtk::HBox m_hbox_hour;
        Gtk::Label m_label_hour;
        SKELETON::SpinButton m_spin_hour;


        // ローカルルール
        SKELETON::View* m_localrule;

        // SETTING.TXT
        SKELETON::EditView m_edit_settingtxt;

      public:
        Preferences( Gtk::Window* parent, const std::string& url );
        virtual ~Preferences();

      private:
        void slot_clear_modified();
        void slot_clear_samba();
        void slot_delete_cookie();
        void slot_check_live();
        void slot_switch_page( GtkNotebookPage*, guint page );
        virtual void slot_ok_clicked();
        virtual bool slot_timeout( int timer_number );
    };

}

#endif
