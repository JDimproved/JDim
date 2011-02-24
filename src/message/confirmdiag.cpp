// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "confirmdiag.h"

#include "dbtree/interface.h"

#include "skeleton/view.h"

using namespace MESSAGE;

ConfirmDiag::ConfirmDiag( const std::string& url, const std::string& message )
    : SKELETON::DetailDiag( MESSAGE::get_admin()->get_win(), url,
                            true,
                            message, "投稿確認",
                            "", "ローカルルール" 
        ),
      m_chkbutton( "今後表示しない(常にOK)(_D)", true )
{
    const int mrg = 16;
    Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox );
    hbox->pack_start( m_chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
    get_vbox()->pack_start( *hbox, Gtk::PACK_SHRINK );

    set_title( "投稿確認" );
    show_all_children();
    grab_ok();
}

void ConfirmDiag::slot_switch_page( GtkNotebookPage*, guint page )
{
    if( get_notebook().get_nth_page( page ) == get_detail() ){
        get_detail()->set_command( "clear_screen" );
        get_detail()->set_command( "append_html", DBTREE::localrule( get_url() ) );
    }
}
