// ライセンス: GPL2

#include "selectdialog.h"
#include "selectlistview.h"

#include "viewfactory.h"

#include "command.h"
#include "session.h"

using namespace BBSLIST;


SelectListDialog::SelectListDialog( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& store )
    : m_selectview( NULL )
{
    m_selectview = dynamic_cast< SelectListView* > ( CORE::ViewFactory( CORE::VIEW_SELECTLIST, url ) );
    if( m_selectview ){
        m_selectview->copy_treestore( store );
        get_vbox()->pack_start( *m_selectview );
        m_selectview->focus_view();
        m_selectview->sig_close_dialog().connect( sigc::mem_fun(*this, &SelectListDialog::slot_close_dialog ) );
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


int SelectListDialog::run()
{
#ifdef _DEBUG
    std::cout << "SelectListDialog::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "SelectListDialog::run fin\n";
#endif

    return ret;
}


void SelectListDialog::slot_close_dialog()
{
    hide();
}
