// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mainitempref.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

MainItemPref::MainItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // 項目設定
    append_hidden( ITEM_NAME_BBSLISTVIEW );
    append_hidden( ITEM_NAME_FAVORITEVIEW );
    append_hidden( ITEM_NAME_BOARDVIEW );
    append_hidden( ITEM_NAME_ARTICLEVIEW );
    append_hidden( ITEM_NAME_IMAGEVIEW );
    append_hidden( ITEM_NAME_URL );
    append_hidden( ITEM_NAME_GO );
    append_hidden( ITEM_NAME_SEPARATOR );

    std::string order = SESSION::get_items_main_toolbar_str();
    std::list< std::string > list_order = MISC::split_line( order );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){
        append_shown( *it );
        erase_hidden( *it );
    }

    set_title( "ツールバー項目設定(メイン)" );
}


// OKを押した
void MainItemPref::slot_ok_clicked()
{
    SESSION::set_items_main_toolbar_str( get_items() );
    CORE::core_set_command( "update_main_toolbar" );
}


// デフォルトボタン
void MainItemPref::slot_def()
{
    clear();

    // 項目設定
    append_shown( ITEM_NAME_BBSLISTVIEW );
    append_shown( ITEM_NAME_FAVORITEVIEW );
    append_shown( ITEM_NAME_BOARDVIEW );
    append_shown( ITEM_NAME_ARTICLEVIEW );
    append_shown( ITEM_NAME_IMAGEVIEW );
    append_shown( ITEM_NAME_SEPARATOR );
    append_shown( ITEM_NAME_URL );
    append_shown( ITEM_NAME_GO );

    append_hidden( ITEM_NAME_SEPARATOR );
}
