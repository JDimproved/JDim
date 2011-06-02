// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "searchitempref.h"

#include "icons/iconmanager.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;


SearchItemPref::SearchItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_NAME, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_SEARCH, ICON::get_icon( ICON::SEARCH ) );
    append_default_pair( ITEM_NAME_RELOAD, ICON::get_icon( ICON::RELOAD ) );
    append_default_pair( ITEM_NAME_STOPLOADING, ICON::get_icon( ICON::STOPLOADING ) );
    append_default_pair( ITEM_NAME_QUIT, ICON::get_icon( ICON::QUIT ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );

    // 文字列を元に行を追加
    append_rows( SESSION::get_items_search_toolbar_str() );

    set_title( "ツールバー項目設定(ログ/スレタイ検索)" );
}


//
// OKを押した
//
void SearchItemPref::slot_ok_clicked()
{
    SESSION::set_items_search_toolbar_str( get_items() );
    CORE::core_set_command( "update_article_toolbar_button" );
}


//
// デフォルトボタン
//
void SearchItemPref::slot_default()
{
    append_rows( SESSION::get_items_search_toolbar_default_str() );
}
