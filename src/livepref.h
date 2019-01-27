// ライセンス: GPL2

// 実況設定ダイアログ

#ifndef _LIVEPREF_H
#define _LIVEPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/spinbutton.h"

namespace CORE
{
    class LivePref : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;

        Gtk::Label m_label_inst;

        Gtk::Frame m_frame_mode;
        Gtk::VBox m_vbox_mode;
        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_mode1;
        Gtk::RadioButton m_mode2;

        Gtk::HBox m_hbox_speed;
        SKELETON::SpinButton m_spin_speed;
        Gtk::Label m_label_speed;

        Gtk::HBox m_hbox_th;
        SKELETON::SpinButton m_spin_th;
        Gtk::Label m_label_th;

        Gtk::Button m_bt_reset;

      public:

        LivePref( Gtk::Window* parent, const std::string& url );
        ~LivePref() noexcept {}

      private:

        // OK押した
        void slot_ok_clicked() override;

        void slot_reset();
    };

}

#endif
