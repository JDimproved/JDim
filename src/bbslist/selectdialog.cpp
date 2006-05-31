// ライセンス: 最新のGPL

#include "selectdialog.h"
#include "bbslistviewbase.h"

#include "viewfactory.h"

using namespace BBSLIST;


SelectListDialog::SelectListDialog( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& store )
{
    m_selectview = dynamic_cast< BBSListViewBase* > ( CORE::ViewFactory( CORE::VIEW_SELECTLIST, url ) );
    if( m_selectview ){
        m_selectview->copy_treestore( store );
        get_vbox()->pack_start( *m_selectview );
        m_selectview->focus_view();
    }

    add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
    add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );

    set_title( "挿入先選択" );
    resize( 600, 400 );
    show_all_children();
}


SelectListDialog::~SelectListDialog()
{
    if( m_selectview ) delete m_selectview;
}


Gtk::TreePath SelectListDialog::get_path()
{
    if( m_selectview ) return m_selectview->get_current_path();

    return Gtk::TreePath();
}
