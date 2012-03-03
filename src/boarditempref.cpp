// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boarditempref.h"

#include "icons/iconmanager.h"
#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;


BoardItemColumnPref::BoardItemColumnPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には後に", Glib::RefPtr< Gdk::Pixbuf >(), false" を付ける )
    append_default_pair( ITEM_NAME_MARK );
    append_default_pair( ITEM_NAME_ID );
    append_default_pair( ITEM_NAME_BOARD );
    append_default_pair( ITEM_NAME_NAME );
    append_default_pair( ITEM_NAME_RES );
    append_default_pair( ITEM_NAME_LOAD );
    append_default_pair( ITEM_NAME_NEW );
    append_default_pair( ITEM_NAME_SINCE );
    append_default_pair( ITEM_NAME_LASTWRITE );
    append_default_pair( ITEM_NAME_ACCESS );
    append_default_pair( ITEM_NAME_SPEED );
    append_default_pair( ITEM_NAME_DIFF );

    // 文字列を元に列を追加
    append_rows( SESSION::get_items_board_col_str() );

    set_title( "リスト項目設定(スレ一覧)" );
}


// OKを押した
void BoardItemColumnPref::slot_ok_clicked()
{
    SESSION::set_items_board_col_str( get_items() );
    CORE::core_set_command( "update_board_columns" );
}



//
// デフォルトボタン
//
void BoardItemColumnPref::slot_default()
{
    append_rows( SESSION::get_items_board_col_default_str() );
}


///////////////////////////////////



BoardItemPref::BoardItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_NEWARTICLE, ICON::get_icon( ICON::WRITE ) );
    append_default_pair( ITEM_NAME_SEARCHBOX, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_SEARCH_NEXT, ICON::get_icon( ICON::SEARCH_NEXT) );
    append_default_pair( ITEM_NAME_SEARCH_PREV, ICON::get_icon( ICON::SEARCH_PREV ) );
    append_default_pair( ITEM_NAME_RELOAD, ICON::get_icon( ICON::RELOAD ) );
    append_default_pair( ITEM_NAME_STOPLOADING, ICON::get_icon( ICON::STOPLOADING ) );
    append_default_pair( ITEM_NAME_APPENDFAVORITE, ICON::get_icon( ICON::APPENDFAVORITE ) );
    append_default_pair( ITEM_NAME_DELETE, ICON::get_icon( ICON::DELETE ) );
    append_default_pair( ITEM_NAME_QUIT, ICON::get_icon( ICON::QUIT ) );
    append_default_pair( ITEM_NAME_BACK, ICON::get_icon( ICON::BACK ) );
    append_default_pair( ITEM_NAME_FORWARD, ICON::get_icon( ICON::FORWARD ) );
    append_default_pair( ITEM_NAME_LOCK, ICON::get_icon( ICON::LOCK ) );
    append_default_pair( ITEM_NAME_CLEAR_HIGHLIGHT, ICON::get_icon( ICON::CLEAR_SEARCH ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );

    // 文字列を元に列を追加
    append_rows( SESSION::get_items_board_toolbar_str() );

    set_title( "ツールバー項目表示設定(スレ一覧)" );
}


// OKを押した
void BoardItemPref::slot_ok_clicked()
{
    SESSION::set_items_board_toolbar_str( get_items() );
    CORE::core_set_command( "update_board_toolbar_button" );
}


//
// デフォルトボタン
//
void BoardItemPref::slot_default()
{
    append_rows( SESSION::get_items_board_toolbar_default_str() );
}
