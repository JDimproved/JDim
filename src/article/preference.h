// ライセンス: 最新のGPL

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
        SKELETON::EditView m_edit_id, m_edit_name;

        // 情報
        Gtk::VBox m_vbox_info;
        Gtk::Label m_label_name;
        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_url_dat;
        SKELETON::LabelEntry m_label_cache;
        SKELETON::LabelEntry m_label_size;

        SKELETON::LabelEntry m_label_since;
        SKELETON::LabelEntry m_label_modified;
        SKELETON::LabelEntry m_label_write;

      public:

        Preferences( const std::string& url );
        virtual ~Preferences();

      private:
        virtual void slot_ok_clicked();
    };
}

#endif
