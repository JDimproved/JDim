// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "viewnote.h"

#include "config/globalconf.h"


using namespace SKELETON;


ViewNotebook::ViewNotebook( DragableNoteBook* )
    : Gtk::Notebook()
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( CONFIG::get_view_margin() );
}


ViewNotebook::~ViewNotebook() noexcept = default;
