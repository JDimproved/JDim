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
        bool m_use_label;
        bool m_use_separator;

        Gtk::Table m_table;
        Gtk::Label m_label;

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
        Gtk::Button m_bt_separator;

        Gtk::TreeView m_tree_hidden;
        Gtk::TreeModelColumn< Glib::ustring > m_col_hidden;
        Gtk::TreeModel::ColumnRecord m_rec_hidden;
        Glib::RefPtr< Gtk::ListStore > m_store_hidden;

      public:

        SelectItemPref( Gtk::Window* parent, const std::string& url, bool use_apply, bool use_label, bool use_separator );
        virtual ~SelectItemPref(){}

      protected:

        void clear();
        void set_label( const std::string& label ){ m_label.set_text( label ); }

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

        // 区切りボタン
        virtual void slot_sepalator(){}

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
