// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dragnote.h"
#include "toolbarnote.h"

#include <gtk/gtk.h>

using namespace SKELETON;


ToolBarNotebook::ToolBarNotebook( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent )
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( 0 );

#if GTKMM_CHECK_VERSION(3,0,0)
    set_margin_top( 1 );
    set_margin_bottom( 1 );
#else
    Glib::RefPtr< Gtk::RcStyle > rcst = get_modifier_style();
    rcst->set_ythickness( 1 );
    modify_style( rcst );
#endif // GTKMM_CHECK_VERSION(3,0,0)
}


//
// 描画イベント
//
// 自前でビュー領域の枠を描画する
//
bool ToolBarNotebook::on_expose_event( GdkEventExpose* event )
{
    // 枠描画
    m_parent->draw_box( this, event );

    // 枠は自前で書いたので gtk_notebook_expose では枠を描画させない
    GtkNotebook *notebook = gobj();
    notebook->show_border = false;
    bool ret = Notebook::on_expose_event( event );
    notebook->show_border = true;

    return ret;
}
