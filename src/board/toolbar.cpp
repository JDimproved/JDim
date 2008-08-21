// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "boardadmin.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "compmanager.h"
#include "global.h"

using namespace BOARD;


BoardToolBar::BoardToolBar() :
    SKELETON::ToolBar( BOARD::get_admin() )
{
    pack_buttons();
    add_search_mode( CONTROL::MODE_BOARD );
}

// ボタンのパッキング
// virtual
void BoardToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_board_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_NEWARTICLE:
                get_buttonbar().append( *get_button_write() );
                set_tooltip( *get_button_write(), CONTROL::get_label_motion( CONTROL::NewArticle ) );
                break;

            case ITEM_SEARCHBOX:
                get_buttonbar().append( *get_entry_search( CORE::COMP_SEARCH_BOARD ) );
                break;

            case ITEM_SEARCH_NEXT:
                get_buttonbar().append( *get_button_down_search() );
                break;

            case ITEM_SEARCH_PREV:
                get_buttonbar().append( *get_button_up_search() );
                break;

            case ITEM_RELOAD:
                get_buttonbar().append( *get_button_reload() );
                break;

            case ITEM_STOPLOADING:
                get_buttonbar().append( *get_button_stop() );
                break;

            case ITEM_FAVORITE:
                get_buttonbar().append( *get_button_favorite() );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motion( CONTROL::AppendFavorite )
                             + "\n\nスレ一覧のタブか選択したスレをお気に入りに直接Ｄ＆Ｄしても登録可能" );
                break;

            case ITEM_DELETE:
                get_buttonbar().append( *get_button_delete() );
                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                break;

            case ITEM_PREVVIEW:
                get_buttonbar().append( *get_button_back() );
                break;

            case ITEM_NEXTVIEW:
                get_buttonbar().append( *get_button_forward() );
                break;

            case ITEM_LOCK:
                get_buttonbar().append( *get_button_lock() );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    set_relief();
    show_all_children();
}    
