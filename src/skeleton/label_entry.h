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
        typedef sigc::signal< void > SIG_ACTIVATE;

        SIG_ACTIVATE m_sig_activate;

        bool m_editable;
        Gtk::Label m_label;
        Gtk::Label m_info;
        Gtk::Entry m_entry;

      public:

        LabelEntry( const bool editable, const std::string& label, const std::string& text = std::string() );

        SIG_ACTIVATE signal_activate(){ return m_sig_activate; }

        void set_editable( const bool editable );
        void set_visibility( const bool visibility );

        void set_label( const std::string& label );

        void set_text( const std::string& text );
        const Glib::ustring get_text();

        void grab_focus();
        bool has_grab();

      private:

        void setup();

        // entry からsignal_activateを受け取った
        void slot_entry_acivate();
    };
}

#endif
