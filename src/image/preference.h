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
        SKELETON::LabelEntry m_label_url_ref;

      public:
        Preferences( Gtk::Window* parent, const std::string& url );
    };

}

#endif
