// ライセンス: GPL2

#ifndef _FONTCOLORPREF_H
#define _FONTCOLORPREF_H

#include "skeleton/prefdiag.h"

#include <vector>

namespace CORE
{
    class ColorTreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< Glib::ustring > m_col_name;
        Gtk::TreeModelColumn< std::string > m_col_color;
        Gtk::TreeModelColumn< int > m_col_colorid;
        Gtk::TreeModelColumn< std::string > m_col_default;

        ColorTreeColumn()
        {
            add( m_col_name );
            add( m_col_color );
            add( m_col_colorid );
            add( m_col_default );
        }
    };


    ////////////////////////////////


    class FontColorPref : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;

        // フォントの設定
        std::vector< int > m_font_tbl;
        std::vector< std::string > m_tooltips_font;

        Gtk::Grid m_grid_font;
        Gtk::Box m_hbox_font;
        Gtk::EventBox m_event_font;
        Gtk::ComboBoxText m_combo_font;
        Gtk::FontButton m_fontbutton;

        Gtk::CheckButton m_checkbutton_font;

        Gtk::Label m_label_space;
        Gtk::SpinButton m_spin_space;
        Gtk::Label m_label_ubar;
        Gtk::SpinButton m_spin_ubar;

        Gtk::Label m_label_aafont;
        Gtk::Entry m_entry_aafont;

        Gtk::Button m_bt_reset_font;

        // 色の設定
        Gtk::Label m_label_warning_color;
        Gtk::VBox m_vbox_color;
        Gtk::ScrolledWindow m_scroll_color; ///< "色の設定"タブをスクロール可能にする

        Gtk::CheckButton m_chk_use_gtktheme_message;
        Gtk::CheckButton m_chk_use_gtkrc_tree;
        Gtk::CheckButton m_chk_use_gtkrc_selection;
        Gtk::CheckButton m_chk_use_html_color;

        Gtk::TreeView m_treeview_color;
        Glib::RefPtr< Gtk::ListStore > m_liststore_color;
        CORE::ColorTreeColumn m_columns_color;
        Gtk::ScrolledWindow m_scrollwin_color;
        Gtk::HBox m_hbox_change_color;
        Gtk::Label m_label_reset_color;
        Gtk::Button m_bt_change_color;
        Gtk::Button m_bt_reset_color;
        Gtk::Button m_bt_reset_color_dark;
        Gtk::Box m_hbox_reset_all_colors;
        Gtk::Label m_label_reset_all_colors;
        Gtk::Button m_bt_reset_all_colors;
        Gtk::Button m_bt_reset_all_colors_dark;

        // テーマの設定
        Gtk::Grid m_grid_theme;
        Gtk::Label m_label_gtk_theme;
        Gtk::ComboBoxText m_combo_theme;
        Gtk::CheckButton m_check_system_theme;
        Glib::RefPtr<Glib::Binding> m_binding_theme; ///< ComboBoxTextとSettings gtk_theme_nameをバインドする
        Glib::RefPtr<Glib::Binding> m_binding_system;
        Gtk::Label m_label_dark_theme;
        Gtk::CheckButton m_check_dark_theme;
        /// @brief ComboBoxTextとSettings gtk_application_prefer_dark_themeをバインドする
        Glib::RefPtr<Glib::Binding> m_binding_dark;

        Gtk::Label m_label_icon_theme;
        Gtk::ComboBoxText m_combo_icon;
        Gtk::CheckButton m_check_system_icon;
        Glib::RefPtr<Glib::Binding> m_binding_icon; ///< ComboBoxTextとSettings gtk_icon_theme_nameをバインドする
        Glib::RefPtr<Glib::Binding> m_binding_system_icon;
        Gtk::Label m_label_use_symbolic_icon;
        Gtk::CheckButton m_check_use_symbolic_icon;

        Gtk::ScrolledWindow m_scroll_note;
        Gtk::Label m_label_note_title;
        Gtk::Label m_label_note;

      public:

        FontColorPref( Gtk::Window* parent, const std::string& url );
        ~FontColorPref() noexcept override;

      private:

        // ウィジェットを追加
        void pack_widget();

        // フォントの設定
        void set_font_settings( const std::string& name, const int fontid, const std::string& tooltip );
        void slot_combo_font_changed();
        void slot_fontbutton_on_set();
        void slot_checkbutton_font_toggled();
        void slot_reset_font();

        // 色の設定
        void set_color_settings( const int colorid, const std::string& name, const std::string& defaultval );
        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        void slot_cell_data_name( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );
        void slot_cell_data_color( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );
        void slot_change_color();
        void slot_reset_color();
        void slot_reset_color_dark();
        void slot_reset_all_colors();
        void slot_reset_all_colors_dark();
        void slot_toggled_symbolic();

        // OK,cancel,apply が押された
        void slot_ok_clicked() override;
        void slot_apply_clicked() override;
        void slot_cancel_clicked() override;
    };
}

#endif
