// ライセンス: GPL2

// about:config

#ifndef _ABOUTCONFIG_H
#define _ABOUTCONFIG_H

#include "skeleton/prefdiag.h"

namespace CONFIG
{
    class TreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< Glib::ustring > m_col_name;
        Gtk::TreeModelColumn< Glib::ustring > m_col_value;

        Gtk::TreeModelColumn< int > m_col_type;
        Gtk::TreeModelColumn< bool > m_col_drawbg;

        Gtk::TreeModelColumn< std::string* > m_col_value_str;
        Gtk::TreeModelColumn< int* > m_col_value_int;
        Gtk::TreeModelColumn< bool* > m_col_value_bool;

        Gtk::TreeModelColumn< std::string > m_col_default_str;
        Gtk::TreeModelColumn< int > m_col_default_int;
        Gtk::TreeModelColumn< bool > m_col_default_bool;

        TreeColumn()
        {
            add( m_col_name );
            add( m_col_value );

            add( m_col_type );
            add( m_col_drawbg );

            add( m_col_value_str );
            add( m_col_value_int );
            add( m_col_value_bool );

            add( m_col_default_str );
            add( m_col_default_int );
            add( m_col_default_bool );
        }
    };


    ////////////////////////////////


    class AboutConfig : public SKELETON::PrefDiag
    {
        Gtk::Label m_label;
        Gtk::TreeView m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        TreeColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

      public:

        AboutConfig( Gtk::Window* parent );
        ~AboutConfig() noexcept {}

      private:

        void pack_widgets();

        void slot_ok_clicked() override;
        void slot_cancel_clicked() override;

        void slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );

        void append_rows();
        void append_row( const std::string& name, std::string& value, const std::string& defaultval );
        void append_row( const std::string& name, int& value, const int defaultval );
        void append_row( const std::string& name, bool& value, const bool defaultval );
        void append_row( const std::string& comment );

        void set_value( Gtk::TreeModel::Row& row, const std::string& value );
        void set_value( Gtk::TreeModel::Row& row, const int& value );
        void set_value( Gtk::TreeModel::Row& row, const bool& value );

        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
    };
}

#endif
