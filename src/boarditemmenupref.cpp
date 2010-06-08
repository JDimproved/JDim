// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boarditemmenupref.h"

#include "icons/iconmanager.h"

#include "skeleton/msgdiag.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace CORE;

#define STOCK_ICON( id ) Gtk::Widget::render_icon( id, Gtk::ICON_SIZE_MENU )

BoardItemMenuPref::BoardItemMenuPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::SelectItemPref( parent, url )
{
    // デフォルトの項目を設定
    append_default_pair( ITEM_NAME_BOOKMARK );
    append_default_pair( ITEM_NAME_OPENARTICLETAB );
    append_default_pair( ITEM_NAME_OPEN_BROWSER );
    append_default_pair( ITEM_NAME_REGETARTICLE );
    append_default_pair( ITEM_NAME_COPY_URL );
    append_default_pair( ITEM_NAME_COPY_TITLE_URL_THREAD );
    append_default_pair( ITEM_NAME_SAVE_DAT );
    append_default_pair( ITEM_NAME_FAVORITE_ARTICLE );
    append_default_pair( ITEM_NAME_NEXTARTICLE );
    append_default_pair( ITEM_NAME_ABONE_ARTICLE );
    append_default_pair( ITEM_NAME_DELETE );
    append_default_pair( ITEM_NAME_PREF_THREAD );
    append_default_pair( ITEM_NAME_PREF_BOARD );
    append_default_pair( ITEM_NAME_ETC );
    append_default_pair( ITEM_NAME_SEPARATOR );

    // 文字列を元に行を追加
    append_rows( SESSION::get_items_board_menu_str() );

    set_title( "コンテキストメニュー項目設定(スレ一覧)" );
}


//
// OKを押した
//
void BoardItemMenuPref::slot_ok_clicked()
{
    SKELETON::MsgDiag mdiag( NULL, "次に開いたスレ一覧から有効になります" );
    mdiag.run();

    SESSION::set_items_board_menu_str( get_items() );
}


//
// デフォルトボタン
//
void BoardItemMenuPref::slot_default()
{
    append_rows( SESSION::get_items_board_menu_default_str() );
}
