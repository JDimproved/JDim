// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleitempref.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

ArticleItemPref::ArticleItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // 項目設定
    append_hidden( ITEM_NAME_WRITEMSG );
    append_hidden( ITEM_NAME_OPENBOARD );
    append_hidden( ITEM_NAME_NAME );
    append_hidden( ITEM_NAME_SEARCH );
    append_hidden( ITEM_NAME_RELOAD );
    append_hidden( ITEM_NAME_STOPLOADING );
    append_hidden( ITEM_NAME_FAVORITE );
    append_hidden( ITEM_NAME_DELETE );
    append_hidden( ITEM_NAME_QUIT );
    append_hidden( ITEM_NAME_SEPARATOR );

    std::string order = SESSION::get_items_article_toolbar_str();
    std::list< std::string > list_order = MISC::split_line( order );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){
        append_shown( *it );
        erase_hidden( *it );
    }

    set_title( "ツールバー項目設定(スレビュー)" );
}


// OKを押した
void ArticleItemPref::slot_ok_clicked()
{
    SESSION::set_items_article_toolbar_str( get_items() );
    CORE::core_set_command( "update_article_toolbar" );
}


// デフォルトボタン
void ArticleItemPref::slot_def()
{
    clear();

    // 項目設定
    append_shown( ITEM_NAME_WRITEMSG );
    append_shown( ITEM_NAME_OPENBOARD );
    append_shown( ITEM_NAME_NAME );
    append_shown( ITEM_NAME_SEARCH );
    append_shown( ITEM_NAME_RELOAD );
    append_shown( ITEM_NAME_STOPLOADING );
    append_shown( ITEM_NAME_FAVORITE );
    append_shown( ITEM_NAME_DELETE );
    append_shown( ITEM_NAME_QUIT );
    append_hidden( ITEM_NAME_SEPARATOR );
}
