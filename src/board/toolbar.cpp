// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "boardadmin.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "session.h"
#include "compmanager.h"
#include "global.h"

using namespace BOARD;


BoardToolBar::BoardToolBar() :
    SKELETON::ToolBar( BOARD::get_admin() )
{
    BoardToolBar::pack_buttons();

    // JDEntry::on_key_release_event()で
    // CONTROL::SearchCache を有効にする
    add_search_control_mode( CONTROL::MODE_BOARD );
}


BoardToolBar::~BoardToolBar() noexcept = default;


// ボタンのパッキング
// virtual
void BoardToolBar::pack_buttons()
{
    // ツールバー非表示の場合は検索バーに検索関係の wiget を表示する
    if( SESSION::get_show_board_toolbar() ) pack_toolbar();
    else pack_search_toolbar();

    set_relief();
    show_all_children();
}

void BoardToolBar::pack_toolbar()
{
#ifdef _DEBUG    
    std::cout << "BoardToolBar::pack_toolbar\n";
#endif

    int num = 0;
    for(;;){
        int item = SESSION::get_item_board_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_NEWARTICLE:
                if( auto button = get_button_write() ) {
                    get_buttonbar().append( *button );
                    button->set_label( CONTROL::get_label( CONTROL::NewArticle ) );
                    set_tooltip( *button, CONTROL::get_label_motions( CONTROL::NewArticle ) );
                }
                break;

            case ITEM_SEARCHBOX:
                get_buttonbar().append( *get_tool_search( CORE::COMP_SEARCH_BOARD ) );
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

            case ITEM_APPENDFAVORITE:
                get_buttonbar().append( *get_button_favorite() );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motions( CONTROL::AppendFavorite )
                             + "\n\nスレ一覧のタブか選択したスレをお気に入りに直接Ｄ＆Ｄしても登録可能" );
                // ポップアップメニューの配置に利用するためツールバーボタンをIDに紐づけします。
                get_admin()->set_anchor_widget( kToolbarWidgetFavoriteAdd, get_button_favorite() );
                break;

            case ITEM_DELETE:
                get_buttonbar().append( *get_button_delete() );
                get_admin()->set_anchor_widget( kToolbarWidgetDelete, get_button_delete() );
                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                break;

            case ITEM_BACK:
                get_buttonbar().append( *get_button_back() );
                break;

            case ITEM_FORWARD:
                get_buttonbar().append( *get_button_forward() );
                break;

            case ITEM_LOCK:
                get_buttonbar().append( *get_button_lock() );
                break;

            case ITEM_CLEAR_HIGHLIGHT:
                get_buttonbar().append( *get_button_clear_highlight() );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }
}

void BoardToolBar::pack_search_toolbar()
{
#ifdef _DEBUG    
    std::cout << "BoardToolBar::pack_search_toolbar\n";
#endif

    get_searchbar()->append( *get_tool_search( CORE::COMP_SEARCH_BOARD ) );        
    get_searchbar()->append( *get_button_down_search() );
    get_searchbar()->append( *get_button_up_search() );
    get_searchbar()->append( *get_button_close_searchbar() );
}


// ツールバー表示切り替え時に検索関係の wiget の位置を変更する
void BoardToolBar::unpack_pack()
{
    unpack_buttons();
    unpack_search_buttons();

    pack_buttons();
}
