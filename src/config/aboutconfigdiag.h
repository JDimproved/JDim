// ライセンス: GPL2

// about:config から開く設定ダイアログ

#ifndef _ABOUTCONFIGDIAG_H
#define _ABOUTCONFIGDIAG_H

#include "skeleton/prefdiag.h"

namespace CONFIG
{
    class AboutConfigDiagStr : public SKELETON::PrefDiag
    {
        std::string* m_value;
        const std::string m_defaultval;

        Gtk::HBox m_hbox;
        Gtk::Entry m_entry;
        Gtk::Button m_button_default;

      public:

        AboutConfigDiagStr( Gtk::Window* parent, std::string* value, const std::string& defaultval );

      protected:
        void slot_ok_clicked() override;

        void slot_default();
    };


    /////////////////////////////////////////////


    class AboutConfigDiagInt : public SKELETON::PrefDiag
    {
        int* m_value;
        const int m_defaultval;

        Gtk::HBox m_hbox;
        Gtk::Entry m_entry;
        Gtk::Button m_button_default;

      public:

        AboutConfigDiagInt( Gtk::Window* parent, int* value, const int defaultval );

      protected:
        void slot_ok_clicked() override;

        void slot_default();
    };


    /////////////////////////////////////////////


    class AboutConfigDiagBool : public SKELETON::PrefDiag
    {
        bool* m_value;
        const bool m_defaultval;

        Gtk::HBox m_hbox;
        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_radio_true;
        Gtk::RadioButton m_radio_false;
        Gtk::Button m_button_default;

      public:

        AboutConfigDiagBool( Gtk::Window* parent, bool* value, const bool defaultval );

      protected:
        void slot_ok_clicked() override;

        void slot_default();
    };

}

#endif
