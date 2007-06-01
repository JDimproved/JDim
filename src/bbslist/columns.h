// ライセンス: GPL2

//
// コラム
//

#ifndef _BBSLISTCOLUMNS_H
#define _BBSLISTCOLUMNS_H

#include <gtkmm.h>

namespace BBSLIST
{
    // 列ID
    enum
    {
        COL_NAME = 0,

        COL_VISIBLE_END,

        // 以下は不可視

        COL_UNDERLINE = COL_VISIBLE_END,
        COL_URL,
        COL_IMAGE,
        COL_TYPE,
        COL_EXPAND,

        COL_NUM_COL
    };


    class TreeColumns : public Gtk::TreeModel::ColumnRecord
    {

    public:
        
        Gtk::TreeModelColumn< Glib::ustring > m_col_name;
        Gtk::TreeModelColumn< bool > m_underline;
        Gtk::TreeModelColumn< Glib::ustring > m_col_url;
        Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > >  m_col_image;
        Gtk::TreeModelColumn< int > m_type;
        Gtk::TreeModelColumn< bool > m_expand; // Dom::parse() で使用

        TreeColumns(){
            add( m_col_name );
            add( m_underline );
            add( m_col_url );
            add( m_col_image );
            add( m_type );
            add( m_expand );
        }

        ~TreeColumns(){}
    };
}

#endif
