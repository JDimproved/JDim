// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boarditempref.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

BoardItemColumnPref::BoardItemColumnPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // 項目設定
    append_hidden( ITEM_NAME_MARK );
    append_hidden( ITEM_NAME_ID );
    append_hidden( ITEM_NAME_NAME );
    append_hidden( ITEM_NAME_RES );
    append_hidden( ITEM_NAME_LOAD );
    append_hidden( ITEM_NAME_NEW );
    append_hidden( ITEM_NAME_SINCE );
    append_hidden( ITEM_NAME_LASTWRITE );
    append_hidden( ITEM_NAME_SPEED );

    std::string order = SESSION::get_items_board_str();
    std::list< std::string > list_order = MISC::split_line( order );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){
        append_shown( *it );
        erase_hidden( *it );
    }

    set_title( "リスト項目設定(スレ一覧)" );
}


// OKを押した
void BoardItemColumnPref::slot_ok_clicked()
{
    SESSION::set_items_board_str( get_items() );
    CORE::core_set_command( "update_board_columns" );
}


// デフォルトボタン
void BoardItemColumnPref::slot_def()
{
    clear();

    append_shown( ITEM_NAME_MARK );
    append_shown( ITEM_NAME_ID );
    append_shown( ITEM_NAME_NAME );
    append_shown( ITEM_NAME_RES );
    append_shown( ITEM_NAME_LOAD );
    append_shown( ITEM_NAME_NEW );
    append_shown( ITEM_NAME_SINCE );
    append_shown( ITEM_NAME_LASTWRITE );
    append_shown( ITEM_NAME_SPEED );
}



///////////////////////////////////



BoardItemPref::BoardItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // 項目設定
    append_hidden( ITEM_NAME_NEWARTICLE );
    append_hidden( ITEM_NAME_SEARCHBOX );
    append_hidden( ITEM_NAME_SEARCH_NEXT );
    append_hidden( ITEM_NAME_SEARCH_PREV );
    append_hidden( ITEM_NAME_RELOAD );
    append_hidden( ITEM_NAME_STOPLOADING );
    append_hidden( ITEM_NAME_FAVORITE );
    append_hidden( ITEM_NAME_DELETE );
    append_hidden( ITEM_NAME_QUIT );
    append_hidden( ITEM_NAME_SEPARATOR );

    std::string order = SESSION::get_items_board_toolbar_str();
    std::list< std::string > list_order = MISC::split_line( order );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){
        append_shown( *it );
        erase_hidden( *it );
    }

    set_title( "ツールバー項目表示設定(スレ一覧)" );
}


// OKを押した
void BoardItemPref::slot_ok_clicked()
{
    SESSION::set_items_board_toolbar_str( get_items() );
    CORE::core_set_command( "update_board_toolbar" );
}


// デフォルトボタン
void BoardItemPref::slot_def()
{
    clear();

    append_shown( ITEM_NAME_NEWARTICLE );
    append_shown( ITEM_NAME_SEARCHBOX );
    append_shown( ITEM_NAME_SEARCH_NEXT );
    append_shown( ITEM_NAME_SEARCH_PREV );
    append_shown( ITEM_NAME_RELOAD );
    append_shown( ITEM_NAME_STOPLOADING );
    append_shown( ITEM_NAME_FAVORITE );
    append_shown( ITEM_NAME_DELETE );
    append_shown( ITEM_NAME_QUIT );
    append_hidden( ITEM_NAME_SEPARATOR );
}
