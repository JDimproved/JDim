// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "boardadmin.h"

#include "skeleton/compentry.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BOARD;


BoardToolBar::BoardToolBar() :
    SKELETON::ToolBar( BOARD::get_admin() )
{
    pack_buttons();
    get_entry_search()->add_mode( CONTROL::MODE_BOARD );
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
                get_buttonbar().pack_start( *get_button_write(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_write(), CONTROL::get_label_motion( CONTROL::NewArticle ) );
                break;

            case ITEM_SEARCHBOX:
                get_buttonbar().pack_start( *get_entry_search(), Gtk::PACK_EXPAND_WIDGET );
                break;

            case ITEM_SEARCH_NEXT:
                get_buttonbar().pack_start( *get_button_down_search(), Gtk::PACK_SHRINK );
                break;

            case ITEM_SEARCH_PREV:
                get_buttonbar().pack_start( *get_button_up_search(), Gtk::PACK_SHRINK );
                break;

            case ITEM_RELOAD:
                get_buttonbar().pack_start( *get_button_reload(), Gtk::PACK_SHRINK );
                break;

            case ITEM_STOPLOADING:
                get_buttonbar().pack_start( *get_button_stop(), Gtk::PACK_SHRINK );
                break;

            case ITEM_FAVORITE:
                get_buttonbar().pack_start( *get_button_favorite(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motion( CONTROL::AppendFavorite )
                             + "\n\nスレ一覧のタブか選択したスレをお気に入りに直接Ｄ＆Ｄしても登録可能" );
                break;

            case ITEM_DELETE:
                get_buttonbar().pack_start( *get_button_delete(), Gtk::PACK_SHRINK );
                break;

            case ITEM_QUIT:
                get_buttonbar().pack_start( *get_button_close(), Gtk::PACK_SHRINK );
                break;

            case ITEM_PREVVIEW:
                get_buttonbar().pack_start( *get_button_back(), Gtk::PACK_SHRINK );
                break;

            case ITEM_NEXTVIEW:
                get_buttonbar().pack_start( *get_button_forward(), Gtk::PACK_SHRINK );
                break;

            case ITEM_LOCK:
                get_buttonbar().pack_start( *get_button_lock(), Gtk::PACK_SHRINK );
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
