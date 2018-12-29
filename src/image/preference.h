// ライセンス: GPL2

#ifndef _IMAGE_PREFERENCES_H
#define _IMAGE_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"

namespace IMAGE
{
    class Preferences : public SKELETON::PrefDiag
    {
        // 情報
        Gtk::VBox m_vbox_info;

        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_cache;
        SKELETON::LabelEntry m_label_ref;

        Gtk::HBox m_hbox_ref;
        SKELETON::LabelEntry m_label_url_ref;
        Gtk::Button m_open_ref;

        SKELETON::LabelEntry m_label_wh;
        SKELETON::LabelEntry m_label_size;
        SKELETON::LabelEntry m_label_type;

        Gtk::CheckButton m_check_protect;

      public:
        Preferences( Gtk::Window* parent, const std::string& url );

      private:
        void slot_ok_clicked() override;
        void slot_open_ref();
    };

}

#endif
