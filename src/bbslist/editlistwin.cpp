// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "editlistwin.h"
#include "selectlistview.h"

#include "viewfactory.h"

using namespace BBSLIST;

enum
{
    EDITWIN_WIDTH = 800,
    EDITWIN_HEIGHT = 500
};


EditListWin::EditListWin( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore )
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
      m_label( "マウスの中ボタンドラッグで行の複数選択が可能です。" ),
      m_bt_close( "閉じる" )
{
    m_vbox.pack_start( m_label, Gtk::PACK_SHRINK );

    m_selectview = dynamic_cast< SelectListView* > ( Gtk::manage( CORE::ViewFactory( CORE::VIEW_SELECTLIST, url ) ) );
    if( m_selectview ){

        m_selectview->copy_treestore( treestore );
        m_selectview->sig_close_dialog().connect( sigc::mem_fun(*this, &EditListWin::hide ) );

        m_vbox.pack_start( *m_selectview );
    }

    m_bt_close.signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_close ) );
    m_hbox.pack_end( m_bt_close, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

    add( m_vbox );
    set_title( "お気に入りの編集" );
    resize( EDITWIN_WIDTH, EDITWIN_HEIGHT );

    show_all_children();
}


void EditListWin::clock_in()
{
    if( m_selectview ) m_selectview->clock_in();
}


void EditListWin::slot_close()
{
    hide();
}
