// ライセンス: GPL2

#ifndef _BOARD_PREFERENCES_H
#define _BOARD_PREFERENCES_H

#include "gtkmmversion.h"

#include "skeleton/view.h"
#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"
#include "skeleton/spinbutton.h"

#if GTKMM_CHECK_VERSION(3,0,0)
using GtkNotebookPage = Gtk::Widget;
#endif

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
        Gtk::HBox m_hbox_write1;
        Gtk::HBox m_hbox_write2;
        SKELETON::LabelEntry m_entry_writename;
        SKELETON::LabelEntry m_entry_writemail;
        Gtk::CheckButton m_check_noname; // 名無し書き込みチェック
        Gtk::Button m_bt_clear_post_history;
        Gtk::Button m_bt_set_default_namemail;

        // クッキー と キーワード表示
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

        Gtk::HBox m_hbox_max;
        SKELETON::LabelEntry m_label_max_line;
        SKELETON::LabelEntry m_label_max_byte;

        // 最大レス数
        Gtk::Label m_label_maxres;
        SKELETON::SpinButton m_spin_maxres;

        SKELETON::LabelEntry m_label_last_access;

        Gtk::HBox m_hbox_modified;
        SKELETON::LabelEntry m_label_modified;
        Gtk::Button m_button_clearmodified;

        // samba24
        Gtk::HBox m_hbox_samba;
        SKELETON::LabelEntry m_label_samba;
        Gtk::Button m_button_clearsamba;

        // 過去ログ表示
        Gtk::CheckButton m_check_oldlog;

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

        Gtk::VBox m_vbox_abone_title;
        Gtk::Button m_button_remove_old_title;

        // ローカルルール
        SKELETON::View* m_localrule;

        // SETTING.TXT
        SKELETON::EditView m_edit_settingtxt;

      public:
        Preferences( Gtk::Window* parent, const std::string& url, const std::string command );
        ~Preferences();

      private:
        void slot_clear_modified();
        void slot_clear_samba();
        void slot_clear_post_history();
        void slot_set_default_namemail();
        void slot_delete_cookie();
        void slot_check_live();
        void slot_remove_old_title();
        void slot_switch_page( GtkNotebookPage*, guint page );
        void slot_ok_clicked() override;
        void timeout() override;
    };

}

#endif
