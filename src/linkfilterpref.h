// ライセンス: GPL2

// リンククリック時のフィルタ設定ダイアログ

#ifndef _LINKFILTERPREF_H
#define _LINKFILTERPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/treeviewbase.h"

#include "control.h"

namespace CORE
{
    class LinkFilterDiag : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::Entry m_entry_url;
        Gtk::Entry m_entry_cmd;
        Gtk::Label m_label_url;
        Gtk::Label m_label_cmd;

      public:

        LinkFilterDiag( Gtk::Window* parent, const std::string& url, const std::string& cmd )
        : SKELETON::PrefDiag( parent, "" ),
        m_label_url( "アドレス" ),
        m_label_cmd( "実行するコマンド" )
        {
            resize( 640, 1 );

            m_entry_url.set_text( url );
            m_entry_cmd.set_text( cmd );

            m_vbox.pack_start( m_label_url, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_entry_url, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_label_cmd, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_entry_cmd, Gtk::PACK_SHRINK );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_vbox );

            set_title( "フィルタ設定" );
            show_all_children();
        }

        const Glib::ustring get_url() { return m_entry_url.get_text(); }
        const Glib::ustring get_cmd() { return m_entry_cmd.get_text(); }
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

        const Gtk::TreeModel::iterator get_selected_row();
        const Gtk::TreeModel::iterator get_top_row();
        const Gtk::TreeModel::iterator get_bottom_row();

        void select_row( const Gtk::TreeModel::iterator& row );

        // OK押した
        virtual void slot_ok_clicked();

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
