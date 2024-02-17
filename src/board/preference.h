// ライセンス: GPL2

#ifndef _BOARD_PREFERENCES_H
#define _BOARD_PREFERENCES_H

#include "gtkmmversion.h"

#include "skeleton/view.h"
#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"

#include <memory>


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

        explicit ProxyFrame( const std::string& title )
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
        Gtk::Grid m_grid_general;
        Gtk::ScrolledWindow m_scroll_vbox; ///< "一般"タブをスクロール可能にする
        Gtk::ScrolledWindow m_scroll_network; ///< "ネットワーク設定"タブをスクロール可能にする

        // 書き込み時のデフォルト名とメール
        Gtk::Frame m_frame_write;
        Gtk::Grid m_grid_write;
        Gtk::Box m_hbox_write;
        Gtk::Label m_label_writename;
        Gtk::Entry m_entry_writename;
        Gtk::Label m_label_writemail;
        Gtk::Entry m_entry_writemail;
        Gtk::CheckButton m_check_utf8_post; ///< UTF-8で書き込む
        Gtk::CheckButton m_check_noname; // 名無し書き込みチェック
        Gtk::Button m_bt_clear_post_history;
        Gtk::Button m_bt_set_default_namemail;

        // クッキー と キーワード表示
        Gtk::Frame m_frame_cookie;
        Gtk::Grid m_grid_cookie;
        SKELETON::EditView m_edit_cookies;
        Gtk::Button m_button_cookie;

        // 実況の更新間隔
        Gtk::Box m_hbox_live;
        Gtk::Label m_label_live;
        Gtk::CheckButton m_check_live;
        Gtk::SpinButton m_spin_live;

        // テキストエンコーディング
        Gtk::Box m_hbox_charset;
        Gtk::Label m_label_charset;
        Gtk::Label m_label_charset_value;
        Gtk::ComboBoxText m_combo_charset;

        Glib::RefPtr<Glib::Binding> m_binding_encoding; ///< ToggleButtonとRevealerをバインドする
        Gtk::ToggleButton m_toggle_encoding;
        Gtk::Revealer m_revealer_encoding;

        Gtk::Box m_vbox_encoding_analysis_method; ///< テキストエンコーディングを判定する方法をパックする
        Gtk::Label m_label_encoding_analysis_method; ///< "エンコーディングの判定"のラベル
        Gtk::RadioButtonGroup m_group_encoding;
        Gtk::RadioButton m_radio_encoding_default;
        Gtk::RadioButton m_radio_encoding_http_header;
        Gtk::RadioButton m_radio_encoding_guess;

        // ネットワーク設定
        Gtk::Box m_vbox_network;

        // ユーザーエージェント
        Gtk::Label m_comment_agent;
        Gtk::Box m_hbox_agent;
        Gtk::Label m_label_agent;
        Gtk::Entry m_entry_agent;

        // プロキシ
        Gtk::Label m_comment_proxy;
        ProxyFrame m_proxy_frame;
        ProxyFrame m_proxy_frame_w;

        // 情報
        Gtk::Label m_label_name;
        Gtk::Label m_label_name_value;
        Gtk::Label m_label_url;
        Gtk::Label m_label_url_value;
        Gtk::Label m_label_cache;
        Gtk::Label m_label_cache_value;

        Gtk::Label m_label_noname;
        Gtk::Label m_label_noname_value;

        Gtk::Label m_label_max_line;
        Gtk::Label m_label_max_line_value;
        Gtk::Label m_label_max_byte;
        Gtk::Label m_label_max_byte_value;

        // 最大レス数
        Gtk::Label m_label_maxres;
        Gtk::SpinButton m_spin_maxres;

        Gtk::Label m_label_last_access;
        Gtk::Label m_label_last_access_value;

        Gtk::Box m_hbox_modified;
        Gtk::Label m_label_modified;
        Gtk::Label m_label_modified_value;
        Gtk::Button m_button_clearmodified;

        // samba24
        Gtk::Box m_hbox_samba;
        Gtk::Label m_label_samba;
        Gtk::Label m_label_samba_value;
        Gtk::Button m_button_clearsamba;

        // 過去ログ表示
        Gtk::CheckButton m_check_oldlog;

        // あぼーん
        Gtk::Notebook m_notebook_abone;
        Gtk::Box m_vbox_abone_general; ///< "一般"タブの要素をパックする
        Gtk::Label m_label_warning_subject; ///< "注意"ラベル
        Gtk::Label m_label_warning; ///< あぼーん設定の注意書き
        Gtk::Separator m_sep_general; ///< 注意書きと設定欄の境界線
        SKELETON::EditView m_edit_id, m_edit_name, m_edit_word, m_edit_regex;

        // 連続投稿したIDをスレのNG IDに追加
        Gtk::Box m_hbox_abone_consecutive; ///< ラベルと設定欄をパックする
        Gtk::Label m_label_abone_consecutive; ///< 設定の項目ラベル
        Gtk::Label m_label_abone_consecutive_over_n_times; ///< 数値の単位を明示するラベル
        Gtk::SpinButton m_spin_abone_consecutive; ///< 回数の入力欄

        // スレッドあぼーん
        Gtk::Notebook m_notebook_abone_thread;
        SKELETON::EditView m_edit_thread, m_edit_word_thread, m_edit_regex_thread;

        Gtk::VBox m_vbox_abone_thread;
        Gtk::Label m_label_abone_thread;

        Gtk::Box m_hbox_low_number;
        Gtk::Label m_label_low_number;
        Gtk::SpinButton m_spin_low_number;

        Gtk::Box m_hbox_high_number;
        Gtk::Label m_label_high_number;
        Gtk::SpinButton m_spin_high_number;

        Gtk::HBox m_hbox_hour;
        Gtk::Label m_label_hour;
        Gtk::SpinButton m_spin_hour;

        Gtk::VBox m_vbox_abone_title;
        Gtk::Button m_button_remove_old_title;

        // ローカルルール
        std::unique_ptr<SKELETON::View> m_localrule;

        // SETTING.TXT
        SKELETON::EditView m_edit_settingtxt;

      public:
        Preferences( Gtk::Window* parent, const std::string& url, const std::string& command );
        ~Preferences() noexcept override = default;

      private:
        void slot_clear_modified();
        void slot_clear_samba();
        void slot_clear_post_history();
        void slot_set_default_namemail();
        void slot_delete_cookie();
        void slot_check_live();
        void slot_remove_old_title();
        void slot_switch_page( Gtk::Widget*, guint page );
        void slot_ok_clicked() override;
        void timeout() override;
    };

}

#endif
