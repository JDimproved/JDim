// ライセンス: 最新のGPL

//
// コラム
//

#ifndef _BBSLISTCOLUMNS_H
#define _BBSLISTCOLUMNS_H

#include <gtkmm.h>

namespace BBSLIST
{
    class TreeColumns : public Gtk::TreeModel::ColumnRecord
    {

    public:
        
        Gtk::TreeModelColumn< Glib::ustring > m_col_name;
        Gtk::TreeModelColumn< bool > m_underline;
        Gtk::TreeModelColumn< Glib::ustring > m_col_url;
        Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > >  m_col_image;
        Gtk::TreeModelColumn< int > m_type;
        
        TreeColumns(){
            add( m_col_name );
            add( m_underline );
            add( m_col_url );
            add( m_col_image );
            add( m_type );
        }

        ~TreeColumns(){}
    };
}

#endif
