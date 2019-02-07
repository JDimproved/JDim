// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#if !GTKMM_CHECK_VERSION(3,0,0)
#include "dragnote.h"
#endif
#include "viewnote.h"
#include "view.h"

#include "config/globalconf.h"

#if !GTKMM_CHECK_VERSION(3,0,0)
#include <gtk/gtk.h>
#endif

using namespace SKELETON;


ViewNotebook::ViewNotebook( DragableNoteBook* parent )
    : Gtk::Notebook()
#if !GTKMM_CHECK_VERSION(3,0,0)
    , m_parent( parent )
#endif
{
#if GTKMM_CHECK_VERSION(3,0,0)
    static_cast< void >( parent );
#endif
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( CONFIG::get_view_margin() );
}


//
// 描画イベント
//
// 自前で枠を描画する
//
#if !GTKMM_CHECK_VERSION(3,0,0)
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
#endif // !GTKMM_CHECK_VERSION(3,0,0)


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
