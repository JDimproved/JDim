// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mainitempref.h"

#include "config/globalconf.h"
#include "icons/iconmanager.h"
#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

MainItemPref::MainItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には最後に false を付ける )
    append_default_pair( ITEM_NAME_BBSLISTVIEW, ICON::get_icon( ICON::DIR ) );
    append_default_pair( ITEM_NAME_FAVORITEVIEW, ICON::get_icon( ICON::FAVORITE ) );
    append_default_pair( ITEM_NAME_HISTVIEW, ICON::get_icon( ICON::HIST ) );
    append_default_pair( ITEM_NAME_HIST_BOARDVIEW, ICON::get_icon( ICON::HIST_BOARD ) );
    append_default_pair( ITEM_NAME_HIST_CLOSEVIEW, ICON::get_icon( ICON::HIST_CLOSE ) );

    append_default_pair( ITEM_NAME_BOARDVIEW, ICON::get_icon( ICON::BOARD ) );

    append_default_pair( ITEM_NAME_ARTICLEVIEW, ICON::get_icon( ICON::THREAD ) );

    if( CONFIG::get_use_image_view() )
    {
        append_default_pair( ITEM_NAME_IMAGEVIEW, ICON::get_icon( ICON::IMAGE ) );
    }
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_URL, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_GO, STOCK_ICON( Gtk::Stock::JUMP_TO ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ), false );

    // 文字列を元に列を追加
    append_rows( SESSION::get_items_main_toolbar_str() );

    set_title( "ツールバー項目設定(メイン)" );
}


// OKを押した
void MainItemPref::slot_ok_clicked()
{
    SESSION::set_items_main_toolbar_str( get_items() );
    CORE::core_set_command( "update_main_toolbar_button" );
}


