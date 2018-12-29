// ライセンス: GPL2
//
// 編集可能な ColumnRecord クラス
//
// SKELETON::EditTreeView と組み合わせて使う
//

#ifndef _EDITCOLUMNS_H
#define _EDITCOLUMNS_H

#include <gtkmm.h>

namespace SKELETON
{
    // 列ID
    enum
    {
        EDITCOL_NAME = 0,
        EDITCOL_IMAGE,

        // 以下不可視

        EDITCOL_TYPE,
        EDITCOL_URL,
        EDITCOL_DATA,
        EDITCOL_UNDERLINE,
        EDITCOL_EXPAND,
        EDITCOL_FGCOLOR,
        EDITCOL_DIRID,

        EDITCOL_NUM_COL
    };


    class EditColumns : public Gtk::TreeModel::ColumnRecord
    {

    public:
        
        Gtk::TreeModelColumn< Glib::ustring > m_name; // サブジェクト
        Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > >  m_image; // アイコン画像

        Gtk::TreeModelColumn< int > m_type; // 行のタイプ
        Gtk::TreeModelColumn< Glib::ustring > m_url; // アドレス
        Gtk::TreeModelColumn< Glib::ustring > m_data; // ユーザデータ
        Gtk::TreeModelColumn< bool > m_underline; // 行に下線を引く
        Gtk::TreeModelColumn< bool > m_expand; // Dom::parse() で使用
        Gtk::TreeModelColumn< Gdk::Color > m_fgcolor; // 文字色
        Gtk::TreeModelColumn< size_t > m_dirid; // ディレクトリID

        EditColumns();
        ~EditColumns();

        virtual void setup_row( Gtk::TreeModel::Row& row,
                                const Glib::ustring url, const Glib::ustring name, const Glib::ustring data, const int type, const size_t dirid );
        virtual void copy_row( const Gtk::TreeModel::Row& row_src, Gtk::TreeModel::Row& row_dest );
    };
}

#endif
