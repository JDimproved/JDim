// ライセンス: GPL2

// コラム

#ifndef _BOARDCOLUMNS_H
#define _BOARDCOLUMNS_H

#include "boardcolumnsid.h"

#include <gtkmm.h>
#include <ctime>

namespace DBTREE
{
    class ArticleBase;
}

namespace BOARD
{
    // 列
    class TreeColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        
        Gtk::TreeModelColumn< Glib::RefPtr< Gio::Icon > > m_col_mark;
        Gtk::TreeModelColumn< int > m_col_id;
        Gtk::TreeModelColumn< Glib::ustring > m_col_subject;
        Gtk::TreeModelColumn< int > m_col_res;
        Gtk::TreeModelColumn< Glib::ustring > m_col_str_load;        
        Gtk::TreeModelColumn< Glib::ustring > m_col_str_new;
        Gtk::TreeModelColumn< Glib::ustring > m_col_since;
        Gtk::TreeModelColumn< Glib::ustring > m_col_write;
        Gtk::TreeModelColumn< Glib::ustring > m_col_access;
        Gtk::TreeModelColumn< int > m_col_speed;
        Gtk::TreeModelColumn< int > m_col_diff;
        Gtk::TreeModelColumn< Glib::ustring > m_col_board;

        // 以下は不可視
        Gtk::TreeModelColumn< int > m_col_mark_val;
        Gtk::TreeModelColumn< bool > m_col_drawbg; // true なら背景を塗る
        Gtk::TreeModelColumn< int > m_col_new;
        Gtk::TreeModelColumn< time_t > m_col_write_t;
        Gtk::TreeModelColumn< time_t > m_col_access_t;
        Gtk::TreeModelColumn< DBTREE::ArticleBase* > m_col_article;
        
        TreeColumns(){
            
            add( m_col_mark );            
            add( m_col_id );
            add( m_col_subject );
            add( m_col_res );
            add( m_col_str_load );            
            add( m_col_str_new );
            add( m_col_since );
            add( m_col_write );
            add( m_col_access );
            add( m_col_speed );
            add( m_col_diff );
            add( m_col_board );

            add( m_col_mark_val );
            add( m_col_drawbg );
            add( m_col_new );
            add( m_col_write_t );
            add( m_col_access_t );
            add( m_col_article );
        }

        ~TreeColumns() noexcept override = default;
    };
}

#endif
