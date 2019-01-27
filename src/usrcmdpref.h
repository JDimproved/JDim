// ライセンス: GPL2

// ユーザコマンド設定ダイアログ

#ifndef _USRCMDPREF_H
#define _USRCMDPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/edittreeview.h"
#include "skeleton/editcolumns.h"

#include "control/control.h"

namespace CORE
{
    class UsrCmdDiag : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::Entry m_entry_name;
        Gtk::Entry m_entry_cmd;
        Gtk::Label m_label_name;
        Gtk::HBox m_hbox_cmd;
        Gtk::Label m_label_cmd;
        Gtk::Button m_button_manual;

      public:

        UsrCmdDiag( Gtk::Window* parent, const Glib::ustring& name, const Glib::ustring& cmd );

        const Glib::ustring get_name() { return m_entry_name.get_text(); }
        const Glib::ustring get_cmd() { return m_entry_cmd.get_text(); }

      private:
        void slot_show_manual();
    };


    class UsrCmdPref : public SKELETON::PrefDiag
    {
        Gtk::Label m_label;

        SKELETON::EditTreeView m_treeview;
        SKELETON::EditColumns m_columns;
        Glib::RefPtr< Gtk::TreeStore > m_treestore;

        Gtk::ScrolledWindow m_scrollwin;

        Gtk::CheckButton m_ckbt_hide_usrcmd;

        Gtk::TreeModel::Path m_path_selected;

        // ポップアップメニュー
        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;

        CONTROL::Control m_control;

      public:

        UsrCmdPref( Gtk::Window* parent, const std::string& url );

      private:

        void timeout() override;

        // OK押した
        void slot_ok_clicked() override;

        bool slot_button_press( GdkEventButton* event );
        bool slot_button_release( GdkEventButton* event );
        bool slot_key_press( GdkEventKey* event );
        bool slot_key_release( GdkEventKey* event );

        void show_popupmenu();
        void delete_row();

        void slot_newcmd();
        void slot_newdir();
        void slot_newsepa();
        void slot_delete();
        void slot_rename();
        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
    };
}

#endif
