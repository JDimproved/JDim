// ライセンス: GPL2

// リンククリック時のフィルタ設定ダイアログ

#ifndef _LINKFILTERPREF_H
#define _LINKFILTERPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/treeviewbase.h"

#include "control/control.h"

namespace CORE
{
    class LinkFilterDiag : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::Entry m_entry_url;
        Gtk::Entry m_entry_cmd;
        Gtk::Label m_label_url;
        Gtk::HBox m_hbox_cmd;
        Gtk::Label m_label_cmd;
        Gtk::Button m_button_manual;

      public:

        LinkFilterDiag( Gtk::Window* parent, const std::string& url, const std::string& cmd );

        Glib::ustring get_url() { return m_entry_url.get_text(); }
        Glib::ustring get_cmd() { return m_entry_cmd.get_text(); }

      private:
        void slot_show_manual();
    };


    class TreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< std::string >  m_col_url;
        Gtk::TreeModelColumn< std::string >  m_col_cmd;

        TreeColumn()
        {
            add( m_col_url );
            add( m_col_cmd );
        }
    };


    class LinkFilterPref : public SKELETON::PrefDiag
    {
        SKELETON::JDTreeViewBase m_treeview;
        CONTROL::Control m_control;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        TreeColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::Label m_label;

        Gtk::Button m_button_top;
        Gtk::Button m_button_up;
        Gtk::Button m_button_down;
        Gtk::Button m_button_bottom;
        Gtk::Button m_button_delete;
        Gtk::Button m_button_add;
        Gtk::VButtonBox m_vbuttonbox;

        Gtk::HBox m_hbox;

      public:

        LinkFilterPref( Gtk::Window* parent, const std::string& url );

      private:

        void append_rows();
        void append_row( const std::string& url, const std::string& cmd );

        Gtk::TreeModel::iterator get_selected_row();
        Gtk::TreeModel::iterator get_top_row();
        Gtk::TreeModel::iterator get_bottom_row();

        void select_row( const Gtk::TreeModel::iterator& row );

        // OK押した
        void slot_ok_clicked() override;

        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        bool slot_key_release( GdkEventKey* event );

        void slot_top();
        void slot_up();
        void slot_down();
        void slot_bottom();
        void slot_delete();
        void slot_add();
    };
}

#endif
