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
        Gtk::Label m_label_name;
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

        SKELETON::LabelEntry m_label_since;
        SKELETON::LabelEntry m_label_modified;
        SKELETON::LabelEntry m_label_write;

      public:

        Preferences( Gtk::Window* parent, const std::string& url );
        virtual ~Preferences();

      private:
        virtual void slot_ok_clicked();
    };
}

#endif
