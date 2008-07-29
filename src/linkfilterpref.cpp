// ライセンス: GPL2

#define _DEBUG
#include "jddebug.h"

#include "linkfilterpref.h"

using namespace CORE;

LinkFilterPref::LinkFilterPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_button_top( Gtk::Stock::GOTO_TOP ),
      m_button_up( Gtk::Stock::GO_UP ),
      m_button_down( Gtk::Stock::GO_DOWN ),
      m_button_bottom( Gtk::Stock::GOTO_BOTTOM ),
      m_button_delete( Gtk::Stock::DELETE ),
      m_button_add( Gtk::Stock::ADD )
{
    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &LinkFilterPref::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "アドレス", m_columns.m_col_link ) );
    column->set_fixed_width( 480 );
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

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_SHRINK );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox );

    show_all_children();

    append_rows();
}


void LinkFilterPref::append_rows()
{
    append_row( "abc", "123" );
    append_row( "def", "456" );
}


void LinkFilterPref::append_row( const std::string& link, const std::string& cmd )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );

    row[ m_columns.m_col_link ] = link;
    row[ m_columns.m_col_cmd ] = cmd;
}


void LinkFilterPref::slot_ok_clicked()
{
}


void LinkFilterPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "LinkFilterPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    LinkFilterDiag diag( this, row[ m_columns.m_col_link ], row[ m_columns.m_col_cmd ] );
    if( diag.run() == Gtk::RESPONSE_OK ){
        row[ m_columns.m_col_link ] = diag.get_link();
        row[ m_columns.m_col_cmd ] = diag.get_cmd();
    }
}
