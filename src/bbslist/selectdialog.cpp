// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "selectdialog.h"
#include "selectlistview.h"
#include "columns.h"

#include "viewfactory.h"

#include "command.h"
#include "session.h"
#include "type.h"
#include "sharedbuffer.h"

using namespace BBSLIST;


enum
{
    SELECTDIAG_WIDTH = 600,
    SELECTDIAG_TREEHEIGHT = 300
};



SelectListDialog::SelectListDialog( Gtk::Window* parent, const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore )
    : SKELETON::PrefDiag( parent, url, true )
    , m_treestore( treestore )
    , m_label_name{ "名前:" }
    , m_label_dirs{ "ディレクトリ:" }
    , m_bt_show_tree( "詳細" )
{
    if( CORE::SBUF_size() ) m_entry_name.set_text(  ( *CORE::SBUF_list_info().begin() ).name );
    m_entry_name.set_editable( CORE::SBUF_size() == 1 );
    set_activate_entry( m_entry_name );

    // コンボボックスにディレクトリをセット
    int active_row = 0;
    Glib::ustring name;

    name = "お気に入りの先頭に追加";
    m_combo_dirs.append( name );
    if( name == SESSION::get_dir_select_favorite() ) active_row = m_vec_path.size();
    m_vec_path.push_back( "-1" );

    name = "お気に入りの最後に追加";
    m_combo_dirs.append( name );
    if( ! active_row && name == SESSION::get_dir_select_favorite() ) active_row = m_vec_path.size();
    m_vec_path.push_back( "" );

    BBSLIST::TreeColumns columns;
    Gtk::TreeModel::Children child = m_treestore->children();
    for( Gtk::TreeModel::Row row : child ) {
        const int type = row[ columns.m_type ];
        if( type == TYPE_DIR ){

            name = row[ columns.m_name ];
            m_combo_dirs.append( name );
            if( ! active_row && name == SESSION::get_dir_select_favorite() ) active_row = m_vec_path.size();

            Gtk::TreePath path = m_treestore->get_path( row );

            m_vec_path.push_back( path.to_string() );

#ifdef _DEBUG
            std::cout << row[ columns.m_name ] << " path = " << path.to_string() << std::endl;
#endif 
        }
    }

#ifdef _DEBUG
    std::cout << "active_row = " << active_row << std::endl;
#endif 
    m_combo_dirs.set_active( active_row );

    m_label_name.set_halign( Gtk::ALIGN_START );
    m_entry_name.set_hexpand( true );
    m_label_dirs.set_halign( Gtk::ALIGN_START );
    m_combo_dirs.set_hexpand( true );

    m_bt_show_tree.signal_clicked().connect( sigc::mem_fun( *this, &SelectListDialog::slot_show_tree ) );

    m_grid.set_margin_bottom( 8 );
    m_grid.set_column_spacing( 10 );
    m_grid.set_row_spacing( 8 );

    m_grid.attach( m_label_name, 0, 0, 1, 1 );
    m_grid.attach( m_entry_name, 1, 0, 2, 1 );

    m_grid.attach( m_label_dirs, 0, 1, 1, 1 );
    m_grid.attach( m_combo_dirs, 1, 1, 1, 1 );
    m_grid.attach( m_bt_show_tree, 2, 1, 1, 1 );

    get_content_area()->property_margin() = 8;
    get_content_area()->pack_start( m_grid );

    set_title( "お気に入り追加先選択" );
    set_default_size( SELECTDIAG_WIDTH, -1 );

    set_default_response( Gtk::RESPONSE_OK );
    grab_ok();
    show_all_children();
}


// SelectListView が不完全型のためヘッダーではデストラクタを定義できない
SelectListDialog::~SelectListDialog() noexcept = default;


//
// OKを押した
//
void SelectListDialog::slot_ok_clicked()
{
    if( ! m_selectview ) SESSION::set_dir_select_favorite( m_combo_dirs.get_active_text() );
}


std::string SelectListDialog::get_name() const
{
    return m_entry_name.get_text();
}

std::string SelectListDialog::get_path() const
{
    std::string path;
    if( m_selectview ) path = m_selectview->get_current_path().to_string();
    else path = m_vec_path[ m_combo_dirs.get_active_row_number() ];

#ifdef _DEBUG
    std::cout << "SelectListDialog::get_path path = " << path << std::endl;
#endif

    return path;
}


void SelectListDialog::slot_show_tree()
{
#ifdef _DEBUG
    std::cout << "SelectListDialog::slot_show_tree\n";
#endif

    if( m_bt_show_tree.get_active() ){

        if( ! m_selectview ) m_selectview = std::make_unique<SelectListView>( get_url() );

        m_selectview->set_parent_win( this );
        m_selectview->copy_treestore( m_treestore );
        m_selectview->sig_close_dialog().connect( sigc::mem_fun( *this, &SelectListDialog::hide ) );

        get_content_area()->pack_start( *m_selectview );
        m_selectview->set_size_request( -1, SELECTDIAG_TREEHEIGHT );
        m_selectview->focus_view();
        show_all_children();
    }
    else if( m_selectview ){
        get_content_area()->remove( *m_selectview );
        // CSDによる装飾を含まないコンテンツ領域の幅を取得し、
        // resize()に指定することで、意図したサイズ変更を実現します。
        int width = 1;
        gtk_window_get_size( static_cast<Gtk::Window*>(this)->gobj(), &width, nullptr );
        resize( width, 1 );
        m_selectview.reset();
    }
}


void SelectListDialog::timeout()
{
    if( m_selectview ) m_selectview->clock_in();
}

