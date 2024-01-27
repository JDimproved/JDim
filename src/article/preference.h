// ライセンス: GPL2

#ifndef _ARTICLE_PREFERENCES_H
#define _ARTICLE_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"

namespace ARTICLE
{
    class Preferences : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;

        // 情報
        Gtk::Grid m_grid_info;
        Gtk::Label m_label_name;
        Gtk::Label m_label_name_value;
        Gtk::Label m_label_url;
        Gtk::Label m_label_url_value;
        Gtk::Label m_label_url_dat;
        Gtk::Label m_label_url_dat_value;
        Gtk::Label m_label_cache;
        Gtk::Label m_label_cache_value;
        Gtk::Label m_label_size;
        Gtk::Label m_label_size_value;

        // 最大レス数
        Gtk::Label m_label_maxres;
        Gtk::SpinButton m_spin_maxres;

        // テキストエンコーディング
        Gtk::Label m_label_charset;
        Gtk::Label m_label_charset_value;
        Gtk::ComboBoxText m_combo_charset;

        // あぼーん
        Gtk::VBox m_vbox_abone;
        Gtk::Notebook m_notebook_abone;
        Gtk::VBox m_vbox_abone_id;
        Gtk::Label m_label_abone_id;
        SKELETON::EditView m_edit_id, m_edit_res, m_edit_name, m_edit_word, m_edit_regex;

        Gtk::Label m_label_abone;

        // 透明あぼーん
        Gtk::CheckButton m_check_transpabone;

        // 連鎖あぼーん
        Gtk::CheckButton m_check_chainabone;

        // ageあぼーん
        Gtk::CheckButton m_check_ageabone;

        // デフォルト名無しあぼーん
        Gtk::CheckButton m_check_defnameabone;

        // ID無しあぼーん
        Gtk::CheckButton m_check_noidabone;

        // 板レベルでのあぼーん
        Gtk::CheckButton m_check_boardabone;

        // 全体レベルでのあぼーん
        Gtk::CheckButton m_check_globalabone;

        Gtk::Label m_label_since;
        Gtk::Label m_label_since_value;

        // 最終更新日時
        Gtk::Label m_label_modified;
        Gtk::Label m_label_modified_value;
        Gtk::Button m_button_clearmodified;

        // 書き込み日時
        Gtk::Label m_label_write;
        Gtk::Label m_label_write_value;
        Gtk::Button m_bt_clear_post_history;

        // 名前とメール
        Gtk::Label m_label_write_name;
        Gtk::Label m_label_write_name_value;
        Gtk::Label m_label_write_mail;
        Gtk::Label m_label_write_mail_value;

      public:

        Preferences( Gtk::Window* parent, const std::string& url, const std::string& command );
        ~Preferences() noexcept override;

      private:
        void slot_ok_clicked() override;
        void slot_clear_modified();
        void slot_clear_post_history();
    };
}

#endif
