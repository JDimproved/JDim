// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarsimple.h"
#include "articleadmin.h"

#include "compmanager.h"

using namespace ARTICLE;


ArticleToolBarSimple::ArticleToolBarSimple() :
    SKELETON::ToolBar( ARTICLE::get_admin() )
{
    // 検索バー    
    get_searchbar()->append( *get_tool_search( CORE::COMP_SEARCH_ARTICLE ) );
    get_searchbar()->append( *get_button_down_search() );
    get_searchbar()->append( *get_button_up_search() );
    get_searchbar()->append( *get_button_close_searchbar() );

    pack_buttons();
}


ArticleToolBarSimple::~ArticleToolBarSimple() noexcept = default;


//
// ボタンのパッキング
//
void ArticleToolBarSimple::pack_buttons()
{
    pack_transparent_separator();
    get_buttonbar().append( *get_label() );
    get_buttonbar().append( *get_button_open_searchbar() );
    get_buttonbar().append( *get_button_reload() );
    get_buttonbar().append( *get_button_stop() );
    get_buttonbar().append( *get_button_close() );

    set_relief();
    show_all_children();
}
