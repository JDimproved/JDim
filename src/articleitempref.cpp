// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleitempref.h"

#include "icons/iconmanager.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

ArticleItemPref::ArticleItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定( 無効にする場合には最後に false を付ける )
    append_default_pair( ITEM_NAME_WRITEMSG, ICON::get_icon( ICON::WRITE ) );
    append_default_pair( ITEM_NAME_OPENBOARD, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_NAME, ICON::get_icon( ICON::TRANSPARENT ) );
    append_default_pair( ITEM_NAME_SEARCH, STOCK_ICON( Gtk::Stock::FIND ) );
    append_default_pair( ITEM_NAME_RELOAD, STOCK_ICON( Gtk::Stock::REFRESH ) );
    append_default_pair( ITEM_NAME_STOPLOADING, STOCK_ICON( Gtk::Stock::STOP ) );
    append_default_pair( ITEM_NAME_FAVORITE, STOCK_ICON( Gtk::Stock::COPY ) );
    append_default_pair( ITEM_NAME_DELETE, STOCK_ICON( Gtk::Stock::DELETE ) );
    append_default_pair( ITEM_NAME_QUIT, STOCK_ICON( Gtk::Stock::CLOSE ) );
    append_default_pair( ITEM_NAME_PREVVIEW, STOCK_ICON( Gtk::Stock::GO_BACK ), false );
    append_default_pair( ITEM_NAME_NEXTVIEW, STOCK_ICON( Gtk::Stock::GO_FORWARD ), false );
    append_default_pair( ITEM_NAME_LOCK, STOCK_ICON( Gtk::Stock::NO ), false );
    append_default_pair( ITEM_NAME_LIVE, STOCK_ICON( Gtk::Stock::MEDIA_PLAY ), false );
    append_default_pair( ITEM_NAME_SEPARATOR, ICON::get_icon( ICON::TRANSPARENT ), false );

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

