// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "selectitempref.h"

#include "jdlib/miscutil.h"

using namespace SKELETON;

SelectItemPref::SelectItemPref( Gtk::Window* parent, const std::string& url, bool use_apply, bool use_label, bool use_separator )
    : SKELETON::PrefDiag( parent, url, true, use_apply ),
      m_use_label( use_label ),
      m_use_separator( use_separator ),
      m_bt_up( "上へ" ),
      m_bt_down( "下へ" ),
      m_bt_del( "→" ),
      m_bt_add( "←" ),
      m_bt_def( "デフォルト" ),
      m_bt_separator( "区切り" )
{
    pack_widgets();
}


// widgetのパック
void SelectItemPref::pack_widgets()
{
    // 非表示項目
    m_rec_hidden.add( m_col_hidden );
    m_store_hidden = Gtk::ListStore::create( m_rec_hidden );
    m_tree_hidden.set_model( m_store_hidden );
    m_tree_hidden.append_column( "非表示", m_col_hidden );

    // ボタン
    m_vbox_buttons.pack_start( m_bt_up, Gtk::PACK_SHRINK );
    m_vbox_buttons.pack_start( m_bt_down, Gtk::PACK_SHRINK );
    m_vbox_buttons.pack_start( m_bt_del, Gtk::PACK_SHRINK );
    m_vbox_buttons.pack_start( m_bt_add, Gtk::PACK_SHRINK );
    if( m_use_separator ) m_vbox_buttons.pack_start( m_bt_separator, Gtk::PACK_SHRINK );
    m_vbox_buttons.pack_start( m_bt_def, Gtk::PACK_SHRINK );

    m_bt_up.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_up ) );
    m_bt_down.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_down ) );
    m_bt_del.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_del ) );
    m_bt_add.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_add ) );
    m_bt_separator.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_sepalator ) );
    m_bt_def.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_def ) );

    // 表示項目
    m_rec_shown.add( m_col_shown );
    m_store_shown = Gtk::ListStore::create( m_rec_shown );
    m_tree_shown.set_model( m_store_shown );
    m_tree_shown.append_column( "表示", m_col_shown );

    // 全体のパッキング
    m_tree_shown.set_size_request( 200, 300 );
    m_tree_hidden.set_size_request( 200, 300 );

    m_table.resize( 3, 1 );
    m_table.attach( m_tree_shown, 0, 1, 0, 1 );//, Gtk::FILL, Gtk::FILL );
    m_table.attach( m_vbox_buttons, 1, 2, 0, 1 , Gtk::SHRINK, Gtk::SHRINK );
    m_table.attach( m_tree_hidden, 2, 3, 0, 1 );//, Gtk::EXPAND, Gtk::EXPAND );

    get_vbox()->set_spacing( 8 );
    if( m_use_label ) get_vbox()->pack_start( m_label );
    get_vbox()->pack_start( m_table );

    show_all_children();
}


// 表示項目のクリア
void SelectItemPref::clear()
{
    m_store_shown->clear();
    m_store_hidden->clear();
}


// 項目の現在値取得
std::string SelectItemPref::get_items()
{
    std::string items;

    const Gtk::TreeModel::Children children = m_store_shown->children();
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end() ; ++it ){
        Gtk::TreeModel::Row row = *it;
        items += row[ m_col_shown ] + " ";
    }

    return items;
}


// 上へ
void SelectItemPref::slot_up()
{
    if( ! m_tree_shown.get_selection()->get_selected() ) return;

    Gtk::TreePath src = m_store_shown->get_path( m_tree_shown.get_selection()->get_selected() );
    Gtk::TreePath dst( src );
    if( dst.prev() ) m_store_shown->iter_swap( m_store_shown->get_iter( src ), m_store_shown->get_iter( dst ) );
}


// 下へ
void SelectItemPref::slot_down()
{
    if( ! m_tree_shown.get_selection()->get_selected() ) return;

    Gtk::TreePath src = m_store_shown->get_path( m_tree_shown.get_selection()->get_selected() );
    Gtk::TreePath dst( src );
    dst.next();
    if( m_store_shown->get_iter( dst ) )
        m_store_shown->iter_swap( m_store_shown->get_iter( src ), m_store_shown->get_iter( dst ) );
}


// 削除ボタン
void SelectItemPref::slot_del()
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


// 追加ボタン
void SelectItemPref::slot_add()
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


// 表示項目に指定した項目を追加
void SelectItemPref::append_shown( const std::string& name )
{
    Gtk::TreeModel::Row row = *( m_store_shown->append() );
    row[ m_col_shown ] = name;
}


// 非表示項目に指定した項目を追加
void SelectItemPref::append_hidden( const std::string& name )
{
    Gtk::TreeModel::Row row = *( m_store_hidden->append() );
    row[ m_col_hidden ] = name;
}


// 非表示項目から指定した項目を削除
void SelectItemPref::erase_hidden( const std::string& name )
{
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
