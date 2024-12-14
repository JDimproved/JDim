// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "linkfilterpref.h"
#include "linkfiltermanager.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

#include "control/controlid.h"

#include "command.h"
#include "environment.h"

#include <glib/gi18n.h>


using namespace CORE;

LinkFilterDiag::LinkFilterDiag( Gtk::Window* parent, const std::string& url, const std::string& cmd )
    : SKELETON::PrefDiag( parent, "" ),
      m_label_url( "アドレス", Gtk::ALIGN_START ),
      m_label_cmd( "実行するコマンド", Gtk::ALIGN_START ),
      m_button_manual( "オンラインマニュアルの置換文字一覧を表示" )
{
    resize( 640, 1 );

    m_entry_url.set_text( url );
    m_entry_cmd.set_text( cmd );

    m_button_manual.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterDiag::slot_show_manual ) );

    m_vbox.set_spacing( 8 );
    m_vbox.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_entry_url, Gtk::PACK_SHRINK );

    m_hbox_cmd.pack_start( m_label_cmd, Gtk::PACK_SHRINK );
    m_hbox_cmd.pack_end( m_button_manual, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_cmd, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_entry_cmd, Gtk::PACK_SHRINK );

    set_activate_entry( m_entry_url );
    set_activate_entry( m_entry_cmd );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_vbox );

    set_title( "フィルタ設定" );
    show_all_children();
}

void LinkFilterDiag::slot_show_manual()
{
    CORE::core_set_command( "open_url_browser", ENVIRONMENT::get_jdhelpcmd() );
}


///////////////////////////////////////////////


LinkFilterPref::LinkFilterPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url )
    , m_label( "追加ボタンを押すとフィルタ設定を追加出来ます。編集するにはダブルクリックします。" )
    , m_button_top( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Top", 24 ), true )
    , m_button_up( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Up", 24 ), true )
    , m_button_down( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Down", 24 ), true )
    , m_button_bottom( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Bottom", 24 ), true )
    , m_button_delete( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Delete", 12 ), true )
    , m_button_add( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Add", 12 ), true )
{
    const bool use_symbolic = CONFIG::get_use_symbolic_icon();
    m_button_top.set_image_from_icon_name( use_symbolic ? "go-top-symbolic" : "go-top" );
    m_button_up.set_image_from_icon_name( use_symbolic ? "go-up-symbolic" : "go-up" );
    m_button_down.set_image_from_icon_name( use_symbolic ? "go-down-symbolic" : "go-down" );
    m_button_bottom.set_image_from_icon_name( use_symbolic ? "go-bottom-symbolic" : "go-bottom" );

    m_button_top.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_top ) );
    m_button_up.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_up ) );
    m_button_down.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_down ) );
    m_button_bottom.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_bottom ) );
    m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_delete ) );
    m_button_add.signal_clicked().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_add ) );

    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_row_activated ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &LinkFilterPref::slot_key_release ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "アドレス", m_columns.m_col_url ) );
    column->set_fixed_width( 240 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    column = Gtk::manage( new Gtk::TreeViewColumn( "コマンド", m_columns.m_col_cmd ) );
    column->set_fixed_width( 240 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );

    m_vbuttonbox.pack_start( m_button_top, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_up, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_down, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_bottom, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_delete, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_add, Gtk::PACK_SHRINK );
    m_vbuttonbox.set_layout( Gtk::BUTTONBOX_START );
    m_vbuttonbox.set_spacing( 4 );

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_EXPAND_WIDGET );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_label, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_hbox );

    show_all_children();
    set_title( "リンクフィルタ設定" );

    append_rows();
}


void LinkFilterPref::append_rows()
{
    const std::vector<LinkFilterItem>& list_item = CORE::get_linkfilter_manager()->get_list();
    if( list_item.empty() ) return;
    for( const LinkFilterItem& item : list_item ) append_row( item.url, item.cmd );

    select_row( get_top_row() );
}


void LinkFilterPref::append_row( const std::string& url, const std::string& cmd )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );
    if( row ){
        row[ m_columns.m_col_url ] = url;
        row[ m_columns.m_col_cmd ] = cmd;

        select_row( row );
    }
}


Gtk::TreeModel::iterator LinkFilterPref::get_selected_row()
{
    Gtk::TreeModel::iterator row;

    std::vector< Gtk::TreeModel::Path > paths = m_treeview.get_selection()->get_selected_rows();
    if( ! paths.size() ) return row;

    row = *( m_liststore->get_iter( *paths.begin() ) );
    return row;
}


Gtk::TreeModel::iterator LinkFilterPref::get_top_row()
{
    Gtk::TreeModel::iterator row;

    Gtk::TreeModel::Children children = m_liststore->children();
    if( children.empty() ) return row;

    row = children.begin();
    return row;
}


Gtk::TreeModel::iterator LinkFilterPref::get_bottom_row()
{
    Gtk::TreeModel::iterator row;

    Gtk::TreeModel::Children children = m_liststore->children();
    if( children.empty() ) return row;

    row = --children.end();
    return row;
}


void LinkFilterPref::select_row( const Gtk::TreeModel::iterator& row )
{
    const Gtk::TreePath path( row );
    m_treeview.get_selection()->select( path );
}


//
// OK ボタンを押した
//
void LinkFilterPref::slot_ok_clicked()
{
#ifdef _DEBUG
    std::cout << "LinkFilterPref::slot_ok_clicked\n";
#endif

    std::vector< LinkFilterItem >& list_item = CORE::get_linkfilter_manager()->get_list();
    list_item.clear();

    const Gtk::TreeModel::Children children = m_liststore->children();
    for( const Gtk::TreeRow& row : children ) {
        if( row ){
            LinkFilterItem item;
            item.url = row[ m_columns.m_col_url ];
            item.cmd = row[ m_columns.m_col_cmd ];
#ifdef _DEBUG
            std::cout << item.url << " " << item.cmd << std::endl;
#endif
            list_item.push_back( std::move( item ) );
        }
    }

    CORE::get_linkfilter_manager()->save_xml();
}


void LinkFilterPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "LinkFilterPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    LinkFilterDiag diag( this, row[ m_columns.m_col_url ], row[ m_columns.m_col_cmd ] );
    if( diag.run() == Gtk::RESPONSE_OK ){
        row[ m_columns.m_col_url ] = diag.get_entry_url();
        row[ m_columns.m_col_cmd ] = diag.get_cmd();
        static_cast<void>( row ); // cppcheck: unreadVariable
    }
}


bool LinkFilterPref::slot_key_release( GdkEventKey* event )
{
    const int id = m_control.key_press( event );

#ifdef _DEBUG
    std::cout << "LinkFilterPref::slot_key_release id = " << id << std::endl;
#endif

    if( id == CONTROL::Delete ) slot_delete();

    return true;
}


//
// 一番上へ移動
//
void LinkFilterPref::slot_top()
{
    Gtk::TreeModel::iterator row = get_selected_row();
    Gtk::TreeModel::iterator row_top = get_top_row();

    if( row && row != row_top )  m_liststore->move( row, row_top );
}


//
// 上へ移動
//
void LinkFilterPref::slot_up()
{
    Gtk::TreeModel::iterator row = get_selected_row();
    Gtk::TreeModel::iterator row_top = get_top_row();

    if( row && row != row_top ){

        Gtk::TreePath path_dst( row );
        if( path_dst.prev() ){

            Gtk::TreeModel::iterator row_dst = m_liststore->get_iter( path_dst );
            m_liststore->iter_swap( row, row_dst );
        }
    }
}


//
// 下へ移動
//
void LinkFilterPref::slot_down()
{
    Gtk::TreeModel::iterator row = get_selected_row();
    Gtk::TreeModel::iterator row_bottom = get_bottom_row();

    if( row && row != row_bottom ){

        Gtk::TreePath path_dst( row );
        path_dst.next();
        Gtk::TreeModel::iterator row_dst = m_liststore->get_iter( path_dst );
        if( row_dst ) m_liststore->iter_swap( row, row_dst );
    }
}



//
// 一番下へ移動
//
void LinkFilterPref::slot_bottom()
{
    Gtk::TreeModel::iterator row = get_selected_row();
    Gtk::TreeModel::iterator row_bottom = get_bottom_row();

    if( row && row != row_bottom ){
        m_liststore->move( row, m_liststore->children().end() );
    }
}


//
// 削除ボタン
//
void LinkFilterPref::slot_delete()
{
    Gtk::TreeModel::iterator row = get_selected_row();
    if( ! row ) return;

    SKELETON::MsgDiag mdiag( nullptr, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    Gtk::TreePath path_next( row );
    path_next.next();
    Gtk::TreeModel::iterator row_next = m_liststore->get_iter( path_next );

    m_liststore->erase( row );

    if( row_next ) select_row( row_next );
    else{
        Gtk::TreeModel::iterator row_bottom = get_bottom_row();
        if( row_bottom ) select_row( row_bottom );
    }
}


//
// 追加ボタン
//
void LinkFilterPref::slot_add()
{
    LinkFilterDiag diag( this, "", "" );
    if( diag.run() == Gtk::RESPONSE_OK ) append_row( diag.get_entry_url(), diag.get_cmd() );
}
