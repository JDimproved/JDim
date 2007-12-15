// ライセンス: GPL2

//
// ラベル + エントリー
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
        Gtk::Entry m_entry;

        Gdk::Color m_color_bg;
        Gdk::Color m_color_bg_org;

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
        void slot_realize();
        void slot_style_changed( Glib::RefPtr< Gtk::Style > style );
    };
}

#endif
