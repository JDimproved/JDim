// ライセンス: GPL2

// 表示項目設定

#ifndef _SELECTITEMPREF_H
#define _SELECTITEMPREF_H

#include "prefdiag.h"
#include "imgbutton.h"

#include <string>
#include <list>

namespace SKELETON
{
    typedef struct
    {
        std::string name;
        Glib::RefPtr< Gdk::Pixbuf > icon;
    } DEFAULT_DATA;

    class SelectItemPref : public SKELETON::PrefDiag
    {
        // デフォルトの値を格納( 項目名, アイコンID, 有効/無効 )
        std::list< DEFAULT_DATA > m_list_default_data;

        // ColumnRecord
        class TreeModelColumns : public Gtk::TreeModel::ColumnRecord
        {
          public:

            TreeModelColumns()
            {
                add( m_column_icon );
                add( m_column_text );
            }

            Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > >  m_column_icon;
            Gtk::TreeModelColumn< Glib::ustring > m_column_text;
        };

        // 表示項目
        Gtk::TreeView m_tree_shown;
        Glib::RefPtr< Gtk::ListStore > m_store_shown;
        TreeModelColumns m_columns_shown;
        Gtk::ScrolledWindow m_scroll_shown;

        // ボタン(縦移動)
        Gtk::Button m_button_top;
        Gtk::Button m_button_up;
        Gtk::Button m_button_down;
        Gtk::Button m_button_bottom;
        Gtk::VButtonBox m_vbuttonbox_v;
        // ボタン(横移動)
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_add;
        Gtk::VButtonBox m_vbuttonbox_h;
        // ボタン(アクション)
        Gtk::Button m_button_default;
        Gtk::VButtonBox m_vbuttonbox_action;

        // まとめ( m_vbuttonbox_* )
        Gtk::VBox m_vbox;

        // 非表示項目
        Gtk::TreeView m_tree_hidden;
        Glib::RefPtr< Gtk::ListStore > m_store_hidden;
        TreeModelColumns m_columns_hidden;
        Gtk::ScrolledWindow m_scroll_hidden;

        // まとめ( m_tree.shown, m_vbox, m_tree_hidden )
        Gtk::HBox m_hbox;

        // キーフック用
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
        typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;
        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;

      public:

        SelectItemPref( Gtk::Window* parent, const std::string& url );
        ~SelectItemPref() noexcept {}

      private:

        // widgetのパック
        void pack_widgets();

        // フォーカスが外れたTreeViewから項目の選択をなくす
        bool slot_focus_in_shown( GdkEventFocus* event );
        bool slot_focus_in_hidden( GdkEventFocus* event );

        // 項目名でデフォルトデータからアイコンを取得
        Glib::RefPtr< Gdk::Pixbuf > get_icon( const Glib::ustring& name );

      protected:

        // 表示項目のクリア
        void clear();

        // デフォルトデータを追加
        void append_default_pair( const Glib::ustring& name,
                                  const Glib::RefPtr< Gdk::Pixbuf > icon = Glib::RefPtr< Gdk::Pixbuf >() );

        // 文字列を元に行を作成
        void append_rows( const std::string& str );

        // 全ての有効な項目を文字列で取得
        std::string get_items();

        // 表示項目に指定した項目を追加
        Gtk::TreeRow append_shown( const std::string& name, const bool set_cursor );

        // 非表示項目に指定した項目を追加
        Gtk::TreeRow append_hidden( const std::string& name, const bool set_cursor );

        // 非表示項目から指定した項目を削除
        void erase_hidden( const std::string& name );

        // KeyPressのフック
        bool on_key_press_event( GdkEventKey* event ) override;

        // KeyReleaseのフック
        bool on_key_release_event( GdkEventKey* event ) override;

        // 最上位へ移動
        void slot_top();

        // 上へ移動
        void slot_up();

        // 下へ移動
        void slot_down();

        // 最下位へ移動
        void slot_bottom();

        // 削除ボタン
        void slot_delete();

        // 追加ボタン
        void slot_add();

        // デフォルトボタン
        virtual void slot_default() = 0;

        // 適用ボタン
        void slot_apply_clicked() override { slot_ok_clicked(); }
    };
}

#endif
