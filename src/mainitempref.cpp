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


MainItemPref::MainItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_BBSLISTVIEW, ICON::get_icon( ICON::BBSLISTVIEW ) );
    append_default_pair( ITEM_NAME_FAVORITEVIEW, ICON::get_icon( ICON::FAVORITEVIEW ) );
    append_default_pair( ITEM_NAME_HISTVIEW, ICON::get_icon( ICON::HISTVIEW ) );
    append_default_pair( ITEM_NAME_HIST_BOARDVIEW, ICON::get_icon( ICON::HIST_BOARDVIEW ) );
    append_default_pair( ITEM_NAME_HIST_CLOSEVIEW, ICON::get_icon( ICON::HIST_CLOSEVIEW ) );
    append_default_pair( ITEM_NAME_HIST_CLOSEIMGVIEW, ICON::get_icon( ICON::HIST_CLOSEIMGVIEW ) );

    append_default_pair( ITEM_NAME_BOARDVIEW, ICON::get_icon( ICON::BOARDVIEW ) );

    append_default_pair( ITEM_NAME_ARTICLEVIEW, ICON::get_icon( ICON::ARTICLEVIEW ) );

    if( CONFIG::get_use_image_view() )
    {
        append_default_pair( ITEM_NAME_IMAGEVIEW, ICON::get_icon( ICON::IMAGEVIEW ) );
    }
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_URL, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_GO, ICON::get_icon( ICON::GO ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );

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


//
// デフォルトボタン
//
void MainItemPref::slot_default()
{
    append_rows( SESSION::get_items_main_toolbar_default_str() );
}
