// ライセンス: GPL2

// 実況設定ダイアログ

#ifndef _LIVEPREF_H
#define _LIVEPREF_H

#include "skeleton/prefdiag.h"


namespace CORE
{
    class LivePref : public SKELETON::PrefDiag
    {
        Gtk::Grid m_grid;

        Gtk::Label m_label_inst;

        Gtk::Frame m_frame_mode;
        Gtk::VBox m_vbox_mode;
        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_mode1;
        Gtk::RadioButton m_mode2;

        Gtk::SpinButton m_spin_speed;
        Gtk::Label m_label_speed;

        Gtk::SpinButton m_spin_th;
        Gtk::Label m_label_th;

        Gtk::Button m_bt_reset;

      public:

        LivePref( Gtk::Window* parent, const std::string& url );
        ~LivePref() noexcept override = default;

      private:

        // OK押した
        void slot_ok_clicked() override;

        void slot_reset();
    };

}

#endif
