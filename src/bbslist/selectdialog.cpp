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
    : SKELETON::PrefDiag( parent, url, true ),
      m_treestore( treestore ),
      m_label_name( CORE::SBUF_size() == 1, "名前 ：" ),
      m_label_dirs( "ディレクトリ ：" ),
      m_bt_show_tree( "詳細" ),
      m_selectview( NULL )
{
    const int mrg = 8;

    if( CORE::SBUF_size() ) m_label_name.set_text(  ( *CORE::SBUF_list_info().begin() ).name );
    set_activate_entry( m_label_name );

    m_hbox_dirs.set_spacing( mrg );
    m_hbox_dirs.pack_start( m_label_dirs, Gtk::PACK_SHRINK );
    m_hbox_dirs.pack_start( m_combo_dirs, Gtk::PACK_EXPAND_WIDGET );
    m_hbox_dirs.pack_start( m_bt_show_tree, Gtk::PACK_SHRINK );

    // コンボボックスにディレクトリをセット
    int active_row = 0;
    Glib::ustring name;

    name = "お気に入りの先頭に追加";
    m_combo_dirs.append( name );
    if( ! active_row && name == SESSION::get_dir_select_favorite() ) active_row = m_vec_path.size();
    m_vec_path.push_back( "-1" );

    name = "お気に入りの最後に追加";
    m_combo_dirs.append( name );
    if( ! active_row && name == SESSION::get_dir_select_favorite() ) active_row = m_vec_path.size();
    m_vec_path.push_back( "" );

    BBSLIST::TreeColumns columns;
    Gtk::TreeModel::Children child = m_treestore->children();
    Gtk::TreeModel::Children::iterator it = child.begin();
    for( ; it != child.end() ; ++it ){

        Gtk::TreeModel::Row row = ( *it );
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

    get_vbox()->pack_start( m_label_name, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_hbox_dirs, Gtk::PACK_SHRINK );
    m_bt_show_tree.signal_clicked().connect( sigc::mem_fun( this, &SelectListDialog::slot_show_tree ) );

    set_title( "お気に入り追加先選択" );
    resize( SELECTDIAG_WIDTH, 1 );

    set_default_response( Gtk::RESPONSE_OK );
    grab_ok();
    show_all_children();
}


SelectListDialog::~SelectListDialog()
{
    if( m_selectview ) delete m_selectview;
}


//
// OKを押した
//
void SelectListDialog::slot_ok_clicked()
{
    if( ! m_selectview ) SESSION::set_dir_select_favorite( m_combo_dirs.get_active_text() );
}


const std::string SelectListDialog::get_name()
{
    return m_label_name.get_text();
}

const std::string SelectListDialog::get_path()
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

        if( ! m_selectview ) m_selectview = dynamic_cast< SelectListView* > ( Gtk::manage( CORE::ViewFactory( CORE::VIEW_SELECTLIST, get_url() ) ) );
        if( m_selectview ){
            m_selectview->set_parent_win( this );
            m_selectview->copy_treestore( m_treestore );
            m_selectview->sig_close_dialog().connect( sigc::mem_fun(*this, &SelectListDialog::hide ) );

            get_vbox()->pack_start(* m_selectview );
            m_selectview->set_size_request( -1, SELECTDIAG_TREEHEIGHT );
            m_selectview->focus_view();
            show_all_children();
        }

    }
    else if( m_selectview ){
        get_vbox()->remove( *m_selectview );
        resize( get_width(), 1 );
        delete m_selectview;
        m_selectview = NULL;
    }
}


void SelectListDialog::timeout()
{
    if( m_selectview ) m_selectview->clock_in();
}

