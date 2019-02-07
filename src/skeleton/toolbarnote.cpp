// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarnote.h"

#if !GTKMM_CHECK_VERSION(3,0,0)
#include "dragnote.h"
#include <gtk/gtk.h>
#endif

using namespace SKELETON;


ToolBarNotebook::ToolBarNotebook( DragableNoteBook* parent )
    : Gtk::Notebook()
#if !GTKMM_CHECK_VERSION(3,0,0)
    , m_parent( parent )
#endif
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( 0 );

#if GTKMM_CHECK_VERSION(3,0,0)
    static_cast< void >( parent );
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
#if !GTKMM_CHECK_VERSION(3,0,0)
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
#endif // !GTKMM_CHECK_VERSION(3,0,0)
