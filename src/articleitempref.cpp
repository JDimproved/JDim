// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleitempref.h"

#include "icons/iconmanager.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;


ArticleItemPref::ArticleItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_WRITEMSG, ICON::get_icon( ICON::WRITE ) );
    append_default_pair( ITEM_NAME_OPENBOARD, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_NAME, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_SEARCH, ICON::get_icon( ICON::SEARCH ) );
    append_default_pair( ITEM_NAME_RELOAD, ICON::get_icon( ICON::RELOAD ) );
    append_default_pair( ITEM_NAME_STOPLOADING, ICON::get_icon( ICON::STOPLOADING ) );
    append_default_pair( ITEM_NAME_APPENDFAVORITE, ICON::get_icon( ICON::APPENDFAVORITE ) );
    append_default_pair( ITEM_NAME_DELETE, ICON::get_icon( ICON::DELETE ) );
    append_default_pair( ITEM_NAME_QUIT, ICON::get_icon( ICON::QUIT ) );
    append_default_pair( ITEM_NAME_BACK, ICON::get_icon( ICON::BACK ) );
    append_default_pair( ITEM_NAME_FORWARD, ICON::get_icon( ICON::FORWARD ) );
    append_default_pair( ITEM_NAME_LOCK, ICON::get_icon( ICON::LOCK ) );
    append_default_pair( ITEM_NAME_LIVE, ICON::get_icon( ICON::LIVE ) );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ) );

    // 文字列を元に行を追加
    append_rows( SESSION::get_items_article_toolbar_str() );

    set_title( "ツールバー項目設定(スレビュー)" );
}


//
// OKを押した
//
void ArticleItemPref::slot_ok_clicked()
{
    SESSION::set_items_article_toolbar_str( get_items() );
    CORE::core_set_command( "update_article_toolbar_button" );
}


//
// デフォルトボタン
//
void ArticleItemPref::slot_default()
{
    append_rows( SESSION::get_items_article_toolbar_default_str() );
}
