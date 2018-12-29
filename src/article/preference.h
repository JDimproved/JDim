// ライセンス: GPL2

#ifndef _ARTICLE_PREFERENCES_H
#define _ARTICLE_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"

namespace ARTICLE
{
    class Preferences : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;

        // 情報
        Gtk::VBox m_vbox_info;
        SKELETON::LabelEntry m_label_name;
        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_url_dat;
        SKELETON::LabelEntry m_label_cache;
        SKELETON::LabelEntry m_label_size;

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

        // 板レベルでのあぼーん
        Gtk::CheckButton m_check_boardabone;

        // 全体レベルでのあぼーん
        Gtk::CheckButton m_check_globalabone;

        SKELETON::LabelEntry m_label_since;

        // 最終更新日時
        Gtk::HBox m_hbox_modified;
        SKELETON::LabelEntry m_label_modified;
        Gtk::Button m_button_clearmodified;

        // 書き込み日時
        Gtk::HBox m_hbox_write;
        SKELETON::LabelEntry m_label_write;
        Gtk::Button m_bt_clear_post_history;

        // 名前とメール
        SKELETON::LabelEntry m_label_write_name;
        SKELETON::LabelEntry m_label_write_mail;

      public:

        Preferences( Gtk::Window* parent, const std::string& url, const std::string command );
        ~Preferences() noexcept;

      private:
        void slot_ok_clicked() override;
        void slot_clear_modified();
        void slot_clear_post_history();
    };
}

#endif
