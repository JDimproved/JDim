// ライセンス: GPL2

// スレ一覧の表示項目設定

#ifndef _BOARDITEMPREF_H
#define _BOARDITEMPREF_H

//#define _DEBUG
#include "jddebug.h"

#include "skeleton/prefdiag.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"

namespace CORE
{
    class BoardItemPref : public SKELETON::PrefDiag
    {
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


        Gtk::TreeView m_tree_hidden;
        Gtk::TreeModelColumn< Glib::ustring > m_col_hidden;
        Gtk::TreeModel::ColumnRecord m_rec_hidden;
        Glib::RefPtr< Gtk::ListStore > m_store_hidden;



        // OK押した
        virtual void slot_ok_clicked(){

            std::string items;

            const Gtk::TreeModel::Children children = m_store_shown->children();
            Gtk::TreeModel::iterator it = children.begin();
            for( ; it != children.end() ; ++it ){
                Gtk::TreeModel::Row row = *it;
                items += row[ m_col_shown ] + " ";
            }

            SESSION::set_items_board( items );
        }

        void slot_up()
        {
            if( ! m_tree_shown.get_selection()->get_selected() ) return;

            Gtk::TreePath src = m_store_shown->get_path( m_tree_shown.get_selection()->get_selected() );
            Gtk::TreePath dst( src );
            if( dst.prev() ) m_store_shown->iter_swap( m_store_shown->get_iter( src ), m_store_shown->get_iter( dst ) );
        }

        void slot_down()
        {
            if( ! m_tree_shown.get_selection()->get_selected() ) return;

            Gtk::TreePath src = m_store_shown->get_path( m_tree_shown.get_selection()->get_selected() );
            Gtk::TreePath dst( src );
            dst.next();
            if( m_store_shown->get_iter( dst ) )
                m_store_shown->iter_swap( m_store_shown->get_iter( src ), m_store_shown->get_iter( dst ) );
        }

        void slot_del()
        {
            Gtk::TreeModel::Row row = *m_tree_shown.get_selection()->get_selected();
            if( row ){
                Glib::ustring item = row[ m_col_shown ];
                m_store_shown->erase( row );

                row = *( m_store_hidden->append() );
                row[ m_col_hidden ] = item;
                m_tree_hidden.get_selection()->select( row );
            }
        }

        void slot_add()
        {
            Gtk::TreeModel::Row row = *m_tree_hidden.get_selection()->get_selected();
            if( row ){
                Glib::ustring item = row[ m_col_hidden ];
                m_store_hidden->erase( row );

                row = *( m_store_shown->append() );
                row[ m_col_shown ] = item;
                m_tree_shown.get_selection()->select( row );
            }
        }

        inline void append_shown( const std::string& name ){
            Gtk::TreeModel::Row row = *( m_store_shown->append() );
            row[ m_col_shown ] = name;
        }

        inline void append_hidden( const std::string& name ){
            Gtk::TreeModel::Row row = *( m_store_hidden->append() );
            row[ m_col_hidden ] = name;
        }

        void slot_def()
        {
            m_store_shown->clear();
            m_store_hidden->clear();

            append_shown( COLUMN_TITLE_MARK );
            append_shown( COLUMN_TITLE_ID );
            append_shown( COLUMN_TITLE_NAME );
            append_shown( COLUMN_TITLE_RES );
            append_shown( COLUMN_TITLE_LOAD );
            append_shown( COLUMN_TITLE_NEW );
            append_shown( COLUMN_TITLE_SINCE );
            append_shown( COLUMN_TITLE_WRITE );
            append_shown( COLUMN_TITLE_SPEED );
        }

        void erase_hidden( const std::string& name ){

            const Gtk::TreeModel::Children children = m_store_hidden->children();
            Gtk::TreeModel::iterator it = children.begin();
            for( ; it != children.end() ; ++it ){
                Gtk::TreeModel::Row row = *it;
                if( row[ m_col_hidden ] == name ){
                    m_store_hidden->erase( *it );
                    break;
                }
            }
        }


      public:

        BoardItemPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url ),
        m_label( "以下の設定は次に開いたスレ一覧から適用されます。" ),
        m_bt_up( "上へ" ),
        m_bt_down( "下へ" ),
        m_bt_del( "→" ),
        m_bt_add( "←" ),
        m_bt_def( "デフォルト" )
        {
            Gtk::TreeModel::Row row;

            // 非表示項目
            m_rec_hidden.add( m_col_hidden );
            m_store_hidden = Gtk::ListStore::create( m_rec_hidden );
            m_tree_hidden.set_model( m_store_hidden );
            m_tree_hidden.append_column( "非表示", m_col_hidden );

            append_hidden( COLUMN_TITLE_MARK );
            append_hidden( COLUMN_TITLE_ID );
            append_hidden( COLUMN_TITLE_NAME );
            append_hidden( COLUMN_TITLE_RES );
            append_hidden( COLUMN_TITLE_LOAD );
            append_hidden( COLUMN_TITLE_NEW );
            append_hidden( COLUMN_TITLE_SINCE );
            append_hidden( COLUMN_TITLE_WRITE );
            append_hidden( COLUMN_TITLE_SPEED );

            // ボタン
            m_vbox_buttons.pack_start( m_bt_up, Gtk::PACK_SHRINK );
            m_vbox_buttons.pack_start( m_bt_down, Gtk::PACK_SHRINK );
            m_vbox_buttons.pack_start( m_bt_del, Gtk::PACK_SHRINK );
            m_vbox_buttons.pack_start( m_bt_add, Gtk::PACK_SHRINK );
            m_vbox_buttons.pack_start( m_bt_def, Gtk::PACK_SHRINK );

            m_bt_up.signal_clicked().connect( sigc::mem_fun( *this, &BoardItemPref::slot_up ) );
            m_bt_down.signal_clicked().connect( sigc::mem_fun( *this, &BoardItemPref::slot_down ) );
            m_bt_del.signal_clicked().connect( sigc::mem_fun( *this, &BoardItemPref::slot_del ) );
            m_bt_add.signal_clicked().connect( sigc::mem_fun( *this, &BoardItemPref::slot_add ) );
            m_bt_def.signal_clicked().connect( sigc::mem_fun( *this, &BoardItemPref::slot_def ) );


            // 表示項目
            m_rec_shown.add( m_col_shown );
            m_store_shown = Gtk::ListStore::create( m_rec_shown );
            m_tree_shown.set_model( m_store_shown );
            m_tree_shown.append_column( "表示", m_col_shown );

            std::string order = SESSION::get_items_board();
            std::list< std::string > list_order = MISC::split_line( order );
            std::list< std::string >::iterator it = list_order.begin();
            for( ; it != list_order.end(); ++it ){
                append_shown( *it );
                erase_hidden( *it );
            }


            // 全体のパッキング
            m_tree_shown.set_size_request( 200, 300 );
            m_tree_hidden.set_size_request( 200, 300 );

            m_table.resize( 3, 1 );
            m_table.attach( m_tree_shown, 0, 1, 0, 1 );//, Gtk::FILL, Gtk::FILL );
            m_table.attach( m_vbox_buttons, 1, 2, 0, 1 , Gtk::SHRINK, Gtk::SHRINK );
            m_table.attach( m_tree_hidden, 2, 3, 0, 1 );//, Gtk::EXPAND, Gtk::EXPAND );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_label );
            get_vbox()->pack_start( m_table );

            set_title( "スレ一覧表示項目設定" );
            show_all_children();
        }

        virtual ~BoardItemPref(){}
    };
}

#endif
