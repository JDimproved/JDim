// ライセンス: GPL2

// キーボード設定ダイアログ

// KeyPref が本体で、KeyPrefの各行をダブルクリックすると KeyDiag が開いて個別に操作の設定が出来る
// KeyDiag の各行をダブルクリックすると KeyInputDiag が開いてキー入力が出来る

#ifndef _KEYPREFPREF_H
#define _KEYPREFPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/treeviewbase.h"

namespace CONTROL
{

    //
    // キーボード入力をラベルに表示するダイアログ
    //
    class KeyInputDiag : public SKELETON::PrefDiag
    {
        Gtk::Label m_label;
        std::string m_str_motion;
        guint m_key;
        bool m_ctrl;
        bool m_shift;
        bool m_alt;

      public:

        KeyInputDiag( Gtk::Window* parent, const std::string& url );

        const std::string get_str_motion() const { return m_str_motion; }
        const guint get_key() const{ return  m_key; }
        const bool get_ctrl() const{ return m_ctrl; }
        const bool get_shift() const{ return m_shift; }
        const bool get_alt() const{ return m_alt; }

      private:

        virtual bool on_key_press_event (GdkEventKey* event);
    };

    ///////////////////////////////////////

    class KeyDiagColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< std::string >  m_col_motion;

        KeyDiagColumn()
        {
            add( m_col_motion );
        }
    };


    //
    // 個別のショートカットキー設定ダイアログ
    //
    class KeyDiag : public SKELETON::PrefDiag
    {
        int m_id;

        SKELETON::JDTreeViewBase m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        KeyDiagColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::Label m_label;

        Gtk::Button m_button_delete;
        Gtk::Button m_button_add;
        Gtk::Button m_button_reset;
        Gtk::VButtonBox m_vbuttonbox;

        Gtk::HBox m_hbox;

      public:

        KeyDiag( Gtk::Window* parent, const std::string& url,
                 const int id, const std::string& label, const std::string& str_motions );

        const std::string get_str_motions();

      private:

        void append_row( const std::string& motion );

        // キーボード入力ダイアログを表示
        const std::string show_inputdiag();

        // 行をダブルクリック
        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );

        // 行削除
        void slot_delete();

        // 行追加
        void slot_add();

        // デフォルトに戻す
        void slot_reset();
    };

    ///////////////////////////////

    class KeyTreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< std::string >  m_col_label;
        Gtk::TreeModelColumn< std::string >  m_col_motions;
        Gtk::TreeModelColumn< int > m_col_id;
        Gtk::TreeModelColumn< bool > m_col_drawbg;

        KeyTreeColumn()
        {
            add( m_col_label );
            add( m_col_motions );
            add( m_col_id );
            add( m_col_drawbg );
        }
    };

    //
    // キーボード設定ダイアログ
    //
    class KeyPref : public SKELETON::PrefDiag
    {
        SKELETON::JDTreeViewBase m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        KeyTreeColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::HBox m_hbox;
        Gtk::Button m_button_reset;

        Gtk::Label m_label;

      public:

        KeyPref( Gtk::Window* parent, const std::string& url );

      private:

        void append_rows();
        void append_row( const int id );
        void append_comment_row( const std::string& comment );

        virtual void slot_cancel_clicked();

        void slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );

        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        void slot_reset();
    };
}

#endif
