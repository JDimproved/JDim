// ライセンス: GPL2

// 表示項目設定

#ifndef _SELECTITEMPREF_H
#define _SELECTITEMPREF_H

#include "prefdiag.h"

#include <string>

namespace SKELETON
{
    class SelectItemPref : public SKELETON::PrefDiag
    {
        Gtk::Table m_table;

        Gtk::ScrolledWindow m_scrwin_shown;
        Gtk::TreeView m_tree_shown;
        Gtk::TreeModelColumn< Glib::ustring > m_col_shown;
        Gtk::TreeModel::ColumnRecord m_rec_shown;
        Glib::RefPtr< Gtk::ListStore > m_store_shown;

        Gtk::VBox m_vbox_buttons;
        Gtk::Button m_bt_up;
        Gtk::Button m_bt_down;
        Gtk::Button m_bt_del;
        Gtk::Button m_bt_add;
        Gtk::Button m_bt_def;

        Gtk::ScrolledWindow m_scrwin_hidden;
        Gtk::TreeView m_tree_hidden;
        Gtk::TreeModelColumn< Glib::ustring > m_col_hidden;
        Gtk::TreeModel::ColumnRecord m_rec_hidden;
        Glib::RefPtr< Gtk::ListStore > m_store_hidden;

      public:

        SelectItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~SelectItemPref(){}

      protected:

        void clear();

        // 項目の現在値取得
        std::string get_items();

        // 表示項目に指定した項目を追加
        void append_shown( const std::string& name );

        // 非表示項目に指定した項目を追加
        void append_hidden( const std::string& name );

        // 非表示項目から指定した項目を削除
        void erase_hidden( const std::string& name );

      protected:

        // 上へ
        void slot_up();

        // 下へ
        void slot_down();

        // 削除ボタン
        void slot_del();

        // 追加ボタン
        void slot_add();

        // 適用ボタン
        virtual void slot_apply_clicked(){ slot_ok_clicked(); }

        // デフォルトボタン
        virtual void slot_def() = 0;

      private:

        // widgetのパック
        void pack_widgets();
    };
}

#endif
