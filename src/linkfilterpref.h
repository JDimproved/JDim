// ライセンス: GPL2

// リンククリック時のフィルタ設定ダイアログ

#ifndef _LINKFILTERPREF_H
#define _LINKFILTERPREF_H

#include "skeleton/prefdiag.h"

namespace CORE
{
    class LinkFilterDiag : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::Entry m_entry_link;
        Gtk::Entry m_entry_cmd;
        Gtk::Label m_label_link;
        Gtk::Label m_label_cmd;

      public:

        LinkFilterDiag( Gtk::Window* parent, const Glib::ustring& link, const Glib::ustring& cmd )
        : SKELETON::PrefDiag( parent, "" ),
        m_label_link( "アドレス" ),
        m_label_cmd( "実行するコマンド" )
        {
            resize( 640, 1 );

            m_entry_link.set_text( link );
            m_entry_cmd.set_text( cmd );

            m_vbox.pack_start( m_label_link, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_entry_link, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_label_cmd, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_entry_cmd, Gtk::PACK_SHRINK );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_vbox );

            set_title( "フィルタ設定" );
            show_all_children();
        }

        const Glib::ustring get_link() { return m_entry_link.get_text(); }
        const Glib::ustring get_cmd() { return m_entry_cmd.get_text(); }
    };


    class TreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< Glib::ustring >  m_col_link;
        Gtk::TreeModelColumn< Glib::ustring >  m_col_cmd;

        TreeColumn()
        {
            add( m_col_link );
            add( m_col_cmd );
        }
    };


    class LinkFilterPref : public SKELETON::PrefDiag
    {
        Gtk::TreeView m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        TreeColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

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
        void append_row( const std::string& link, const std::string& cmd );

        // OK押した
        virtual void slot_ok_clicked();

        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
    };
}

#endif
