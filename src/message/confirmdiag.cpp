// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "confirmdiag.h"

#include "dbtree/interface.h"

#include "skeleton/view.h"

#include "viewfactory.h"


using namespace MESSAGE;


ConfirmDiag::ConfirmDiag( const std::string& url, const std::string& message )
    : SKELETON::PrefDiag( MESSAGE::get_admin()->get_win(), url ),
      m_message( message ),
      m_localrule( NULL ),
      m_chkbutton( "今後表示しない(常にOK)(_D)", true )
{
    m_message.set_width_chars( 60 );
    m_message.set_line_wrap( true );
    m_message.set_padding( 8, 8 );

    m_localrule = CORE::ViewFactory( CORE::VIEW_ARTICLEINFO, get_url() );

    m_notebook.append_page( m_message, "投稿確認" );
    m_notebook.append_page( *m_localrule, "ローカルルール" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &ConfirmDiag::slot_switch_page ) );

    get_vbox()->pack_start( m_notebook );

    const int mrg = 16;
    Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox );
    hbox->pack_start( m_chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
    get_vbox()->pack_start( *hbox, Gtk::PACK_SHRINK );

    set_title( "投稿確認" );
    show_all_children();
    grab_ok();
}


ConfirmDiag::~ConfirmDiag()
{
    if( m_localrule ) delete m_localrule;
    m_localrule = NULL;
}


void ConfirmDiag::slot_switch_page( GtkNotebookPage*, guint page )
{
    if( m_notebook.get_nth_page( page ) == m_localrule ){
        m_localrule->set_command( "clear_screen" );
        m_localrule->set_command( "append_html", DBTREE::localrule( get_url() ) );
    }
}


void ConfirmDiag::timeout()
{
    if( m_localrule ) m_localrule->clock_in();    
}
