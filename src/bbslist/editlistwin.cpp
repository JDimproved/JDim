// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "editlistwin.h"
#include "selectlistview.h"

#include "viewfactory.h"
#include "command.h"

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

    SelectListView* selectview = dynamic_cast< SelectListView* > ( Gtk::manage( CORE::ViewFactory( CORE::VIEW_SELECTLIST, url ) ) );
    if( selectview ){

        selectview->copy_treestore( treestore );
        selectview->sig_close_dialog().connect( sigc::mem_fun(*this, &EditListWin::hide ) );

        m_vbox.pack_start( *selectview );
    }

    m_bt_close.signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_close ) );
    m_hbox.pack_end( m_bt_close, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

    add( m_vbox );
    set_title( "お気に入りの編集" );
    resize( EDITWIN_WIDTH, EDITWIN_HEIGHT );
    set_transient_for( *CORE::get_mainwindow() );

    show_all_children();
}


void EditListWin::slot_close()
{
    hide();
}
