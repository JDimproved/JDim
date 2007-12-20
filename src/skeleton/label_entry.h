// ライセンス: GPL2

//
// ラベル + ラベル / エントリー
//
// プロパティの表示用
//

#ifndef _LABEL_ENTRY_H
#define _LABEL_ENTRY_H

#include <gtkmm.h>

namespace SKELETON
{
    class LabelEntry : public Gtk::HBox
    {
        bool m_editable;
        Gtk::Label m_label;
        Gtk::Label m_info;
        Gtk::Entry m_entry;

      public:

        LabelEntry( const bool editable, const std::string& label, const std::string& text = std::string() );

        void set_editable( const bool editable );
        void set_visibility( const bool visibility );

        void set_text( const std::string& text );
        const Glib::ustring get_text();

        void grab_focus();
        const bool has_grab();

      private:

        void setup();
    };
}

#endif
