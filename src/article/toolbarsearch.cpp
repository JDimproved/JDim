// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarsearch.h"
#include "articleadmin.h"

#include "skeleton/compentry.h"

#include "controlutil.h"
#include "controlid.h"

using namespace ARTICLE;

SearchToolBar::SearchToolBar() :
    SKELETON::ToolBar( ARTICLE::get_admin() )
{
    // 検索バー    
    get_searchbar()->pack_start( *get_entry_search(), Gtk::PACK_EXPAND_WIDGET );
    get_searchbar()->pack_end( *get_button_close_searchbar(), Gtk::PACK_SHRINK );

    get_entry_search()->add_mode( CONTROL::MODE_COMMON );

    pack_buttons();
}


//
// ボタンのパッキング
//
void SearchToolBar::pack_buttons()
{
    set_tooltip( *get_button_stop(), "検索中止 " + CONTROL::get_motion( CONTROL::StopLoading ) );
    set_tooltip( *get_button_reload(), "再検索 " + CONTROL::get_motion( CONTROL::Reload ) );

    get_buttonbar().pack_start( *get_label(), Gtk::PACK_EXPAND_WIDGET, 2 );
    get_buttonbar().pack_end( *get_button_close(), Gtk::PACK_SHRINK );
    get_buttonbar().pack_end( *get_button_stop(), Gtk::PACK_SHRINK );
    get_buttonbar().pack_end( *get_button_reload(), Gtk::PACK_SHRINK );
    get_buttonbar().pack_end( *get_button_open_searchbar(), Gtk::PACK_SHRINK );

    show_all_children();
}
