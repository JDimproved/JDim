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

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

BoardItemColumnPref::BoardItemColumnPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には後に", Glib::RefPtr< Gdk::Pixbuf >(), false" を付ける )
    append_default_pair( ITEM_NAME_MARK );
    append_default_pair( ITEM_NAME_ID );
    append_default_pair( ITEM_NAME_NAME );
    append_default_pair( ITEM_NAME_RES );
    append_default_pair( ITEM_NAME_LOAD );
    append_default_pair( ITEM_NAME_NEW );
    append_default_pair( ITEM_NAME_SINCE );
    append_default_pair( ITEM_NAME_LASTWRITE );
    append_default_pair( ITEM_NAME_SPEED );

    // 文字列を元に列を追加
    append_rows( SESSION::get_items_board_str() );

    set_title( "リスト項目設定(スレ一覧)" );
}


// OKを押した
void BoardItemColumnPref::slot_ok_clicked()
{
    SESSION::set_items_board_str( get_items() );
    CORE::core_set_command( "update_board_columns" );
}



///////////////////////////////////



BoardItemPref::BoardItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には最後に false を付ける )
    append_default_pair( ITEM_NAME_NEWARTICLE, ICON::get_icon( ICON::WRITE ) );
    append_default_pair( ITEM_NAME_SEARCHBOX, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_SEARCH_NEXT, STOCK_ICON( Gtk::Stock::GO_DOWN ) );
    append_default_pair( ITEM_NAME_SEARCH_PREV, STOCK_ICON( Gtk::Stock::GO_UP ) );
    append_default_pair( ITEM_NAME_RELOAD, STOCK_ICON( Gtk::Stock::REFRESH ) );
    append_default_pair( ITEM_NAME_STOPLOADING, STOCK_ICON( Gtk::Stock::STOP ) );
    append_default_pair( ITEM_NAME_FAVORITE, STOCK_ICON( Gtk::Stock::COPY ) );
    append_default_pair( ITEM_NAME_DELETE, STOCK_ICON( Gtk::Stock::DELETE ) );
    append_default_pair( ITEM_NAME_QUIT, STOCK_ICON( Gtk::Stock::CLOSE ) );
    append_default_pair( ITEM_NAME_PREVVIEW, STOCK_ICON( Gtk::Stock::GO_BACK ), false );
    append_default_pair( ITEM_NAME_NEXTVIEW, STOCK_ICON( Gtk::Stock::GO_FORWARD ), false );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ), false );

    // 文字列を元に列を追加
    append_rows( SESSION::get_items_board_toolbar_str() );

    set_title( "ツールバー項目表示設定(スレ一覧)" );
}


// OKを押した
void BoardItemPref::slot_ok_clicked()
{
    SESSION::set_items_board_toolbar_str( get_items() );
    CORE::core_set_command( "update_board_toolbar" );
}

