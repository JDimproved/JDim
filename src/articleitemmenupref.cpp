// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleitemmenupref.h"

#include "icons/iconmanager.h"

#include "skeleton/msgdiag.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

ArticleItemMenuPref::ArticleItemMenuPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_DRAWOUT );
    append_default_pair( ITEM_NAME_GO );
    append_default_pair( ITEM_NAME_SEARCH );
    append_default_pair( ITEM_NAME_NGWORD );
    append_default_pair( ITEM_NAME_QUOTE_SELECTION );
    append_default_pair( ITEM_NAME_OPEN_BROWSER );
    append_default_pair( ITEM_NAME_USER_COMMAND );
    append_default_pair( ITEM_NAME_COPY_URL );
    append_default_pair( ITEM_NAME_COPY_TITLE_URL_THREAD );
    append_default_pair( ITEM_NAME_COPY );
    append_default_pair( ITEM_NAME_ETC );
    append_default_pair( ITEM_NAME_RELOAD );
    append_default_pair( ITEM_NAME_SAVE_DAT );
    append_default_pair( ITEM_NAME_COPY_THREAD_INFO );
    append_default_pair( ITEM_NAME_FAVORITE );
    append_default_pair( ITEM_NAME_ABONE_SELECTION );
    append_default_pair( ITEM_NAME_SELECTIMG );
    append_default_pair( ITEM_NAME_PREF_THREAD );
    
    append_default_pair( ITEM_NAME_SEPARATOR );

    // 文字列を元に行を追加
    append_rows( SESSION::get_items_article_menu_str() );

    set_title( "コンテキストメニュー項目設定(スレビュー)" );
}


//
// OKを押した
//
void ArticleItemMenuPref::slot_ok_clicked()
{
    SKELETON::MsgDiag mdiag( NULL, "次に開いたスレビューから有効になります" );
    mdiag.run();

    SESSION::set_items_article_menu_str( get_items() );
}


//
// デフォルトボタン
//
void ArticleItemMenuPref::slot_default()
{
    append_rows( SESSION::get_items_article_menu_default_str() );
}
