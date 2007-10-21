// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgitempref.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

MsgItemPref::MsgItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url, true, false, false )
{
    // 項目設定
    append_hidden( ITEM_NAME_PREVIEW );
    append_hidden( ITEM_NAME_WRITEMSG );
    append_hidden( ITEM_NAME_NAME );
    append_hidden( ITEM_NAME_UNDO );
    append_hidden( ITEM_NAME_INSERTTEXT );
    append_hidden( ITEM_NAME_NOTCLOSE );
    append_hidden( ITEM_NAME_QUIT );

    std::string order = SESSION::get_items_msg_toolbar_str();
    std::list< std::string > list_order = MISC::split_line( order );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){
        append_shown( *it );
        erase_hidden( *it );
    }

    set_title( "ツールバー項目設定(書き込みビュー)" );
}


// OKを押した
void MsgItemPref::slot_ok_clicked()
{
    SESSION::set_items_msg_toolbar_str( get_items() );
    CORE::core_set_command( "update_message_toolbar" );
}


// デフォルトボタン
void MsgItemPref::slot_def()
{
    clear();

    // 項目設定
    append_shown( ITEM_NAME_PREVIEW );
    append_shown( ITEM_NAME_WRITEMSG );
    append_shown( ITEM_NAME_NAME );
    append_shown( ITEM_NAME_UNDO );
    append_shown( ITEM_NAME_INSERTTEXT );
    append_shown( ITEM_NAME_NOTCLOSE );
    append_shown( ITEM_NAME_QUIT );
}
