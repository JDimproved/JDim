// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarsearch.h"
#include "articleadmin.h"

#include "skeleton/compentry.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "compmanager.h"

using namespace ARTICLE;

SearchToolBar::SearchToolBar() :
    SKELETON::ToolBar( ARTICLE::get_admin() )
{
    // 検索バー    
    get_searchbar()->append( *get_tool_search( CORE::COMP_SEARCH_ARTICLE ) );
    get_searchbar()->append( *get_button_close_searchbar() );

    pack_buttons();
}


//
// ボタンのパッキング
//
void SearchToolBar::pack_buttons()
{
    set_tooltip( *get_button_stop(), "検索中止 " + CONTROL::get_str_motions( CONTROL::StopLoading ) );
    set_tooltip( *get_button_reload(), "再検索 " + CONTROL::get_str_motions( CONTROL::Reload ) );

    pack_transparent_separator();
    get_buttonbar().append( *get_label() );
    get_buttonbar().append( *get_button_open_searchbar() );
    get_buttonbar().append( *get_button_reload() );
    get_buttonbar().append( *get_button_stop() );
    get_buttonbar().append( *get_button_close() );

    set_relief();
    show_all_children();
}
