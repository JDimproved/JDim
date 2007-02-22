// ライセンス: GPL2

#ifndef _FONTCOLORPREF_H
#define _FONTCOLORPREF_H

#include "skeleton/prefdiag.h"

#include <vector>

namespace CORE
{
    class FontColorPref : public SKELETON::PrefDiag
    {
        // フォントの設定
        std::vector< int > m_font_tbl;

        Gtk::ComboBoxText m_combo_font;
        Gtk::FontButton m_fontbutton;
        Gtk::CheckButton m_checkbutton_font;
        Gtk::HBox m_hbox_font;
        Gtk::VBox m_vbox_font;
        Gtk::Frame m_frame_font;

        // 色の設定
        std::vector< int > m_color_tbl;

        Gtk::ComboBoxText m_combo_color;
        Gtk::ColorButton m_colorbutton;
        Gtk::HBox m_hbox_color;
        Gtk::VBox m_vbox_color;
        Gtk::Frame m_frame_color;

      public:

        FontColorPref( const std::string& url );
        ~FontColorPref();

      private:

        // ウィジェットを追加
        void pack_widget();

        // フォントの設定
        void set_font_settings( const std::string& name, const int fontid );
        void slot_combo_font_changed();
        void slot_fontbutton_on_set();
        void slot_checkbutton_font_toggled();

        // 色の設定
        void set_color_settings( const std::string& name, const int colorid );
        void slot_combo_color_changed();
        void slot_colorbutton_on_set();

        // OK,cancel が押された
        virtual void slot_ok_clicked();
        virtual void slot_cancel_clicked();
    };
}

#endif
