// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarnote.h"


using namespace SKELETON;


ToolBarNotebook::ToolBarNotebook( DragableNoteBook* )
    : Gtk::Notebook()
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( 0 );

    set_margin_top( 1 );
    set_margin_bottom( 1 );
}


ToolBarNotebook::~ToolBarNotebook() noexcept = default;
