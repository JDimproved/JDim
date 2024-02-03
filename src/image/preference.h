// ライセンス: GPL2

#ifndef _IMAGE_PREFERENCES_H
#define _IMAGE_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"

namespace IMAGE
{
    class Preferences : public SKELETON::PrefDiag
    {
        // 情報
        Gtk::Grid m_grid_info;

        Gtk::Label m_label_url;
        Gtk::Label m_label_url_value;
        Gtk::Label m_label_cache;
        Gtk::Label m_label_cache_value;
        Gtk::Label m_label_ref;
        Gtk::Label m_label_ref_value;

        Gtk::Label m_label_url_ref;
        Gtk::Label m_label_url_ref_value;
        Gtk::Button m_open_ref;

        Gtk::Label m_label_wh;
        Gtk::Label m_label_wh_value;
        Gtk::Label m_label_size;
        Gtk::Label m_label_size_value;
        Gtk::Label m_label_type;
        Gtk::Label m_label_type_value;

        Gtk::CheckButton m_check_protect;

      public:
        Preferences( Gtk::Window* parent, const std::string& url );

      private:
        void slot_ok_clicked() override;
        void slot_open_ref();
    };

}

#endif
