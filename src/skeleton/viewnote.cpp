// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dragnote.h"
#include "viewnote.h"
#include "view.h"

#include "config/globalconf.h"

#include <gtk/gtk.h>

using namespace SKELETON;


ViewNotebook::ViewNotebook( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent )
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( CONFIG::get_view_margin() );
}


//
// 描画イベント
//
// 自前で枠を描画する
//
bool ViewNotebook::on_expose_event( GdkEventExpose* event )
{
    if( ! get_n_pages() ) return Notebook::on_expose_event( event );

    // 枠描画
    m_parent->draw_box( this, event );

    // 枠は自前で書いたので gtk_notebook_expose では枠を描画させない
    GtkNotebook *notebook = gobj();
    notebook->show_border = false;
    bool ret = Notebook::on_expose_event( event );
    notebook->show_border = true;

    return ret;
}


//
// スクロールバー再描画
//
// テーマによってはビューのスクロールバーが消えるときがあるので明示的に再描画する
// DragableNoteBook::on_expose_event()を参照せよ
//
void ViewNotebook::redraw_scrollbar()
{
    int page = get_current_page();
    if( page == -1 ) return;
    SKELETON::View* view =  dynamic_cast< View* >( get_nth_page( page ) );
    if( ! view ) return;

    view->redraw_scrollbar();
}
