// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgitempref.h"

#include "icons/iconmanager.h"
#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

MsgItemPref::MsgItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には最後に false を付ける )
    append_default_pair( ITEM_NAME_PREVIEW, ICON::get_icon( ICON::THREAD ) );
    append_default_pair( ITEM_NAME_WRITEMSG, ICON::get_icon( ICON::WRITE ) );
    append_default_pair( ITEM_NAME_OPENBOARD, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_NAME, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_UNDO, STOCK_ICON( Gtk::Stock::UNDO ) );
    append_default_pair( ITEM_NAME_INSERTTEXT, STOCK_ICON( Gtk::Stock::OPEN ) );
    append_default_pair( ITEM_NAME_LOCK_MESSAGE, STOCK_ICON( Gtk::Stock::NO ) );
    append_default_pair( ITEM_NAME_QUIT, STOCK_ICON( Gtk::Stock::CLOSE ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ), false );

    // 文字列を元に行を追加
    append_rows( SESSION::get_items_msg_toolbar_str() );

    set_title( "ツールバー項目設定(書き込みビュー)" );
}


// OKを押した
void MsgItemPref::slot_ok_clicked()
{
    SESSION::set_items_msg_toolbar_str( get_items() );
    CORE::core_set_command( "update_message_toolbar_button" );
}


