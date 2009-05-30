// ライセンス: GPL2

// コラム

#ifndef _BOARDCOLUMNS_H
#define _BOARDCOLUMNS_H

#include "boardcolumnsid.h"

#include <gtkmm.h>

namespace BOARD
{
    // 列
    class TreeColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        
        Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > > m_col_mark;
        Gtk::TreeModelColumn< int > m_col_id;
        Gtk::TreeModelColumn< Glib::ustring > m_col_subject;
        Gtk::TreeModelColumn< int > m_col_res;
        Gtk::TreeModelColumn< Glib::ustring > m_col_str_load;        
        Gtk::TreeModelColumn< Glib::ustring > m_col_str_new;
        Gtk::TreeModelColumn< Glib::ustring > m_col_since;
        Gtk::TreeModelColumn< Glib::ustring > m_col_write;
        Gtk::TreeModelColumn< int > m_col_speed;
        
        Gtk::TreeModelColumn< int > m_col_mark_val;  // マークの優先順(新着を一番上にする)
        Gtk::TreeModelColumn< bool > m_col_drawbg; // true なら背景を塗る
        Gtk::TreeModelColumn< int > m_col_load;        
        Gtk::TreeModelColumn< int > m_col_new;
        Gtk::TreeModelColumn< time_t > m_col_since_t;
        Gtk::TreeModelColumn< time_t > m_col_write_t;
        Gtk::TreeModelColumn< Glib::ustring > m_col_id_dat;
        
        TreeColumns(){
            
            add( m_col_mark );            
            add( m_col_id );
            add( m_col_subject );
            add( m_col_res );
            add( m_col_str_load );            
            add( m_col_str_new );
            add( m_col_since );
            add( m_col_write );
            add( m_col_speed );

            add( m_col_mark_val );
            add( m_col_drawbg );
            add( m_col_load );            
            add( m_col_new );
            add( m_col_since_t );
            add( m_col_write_t );
            add( m_col_id_dat );
        }

        ~TreeColumns(){}
    };
}

#endif
