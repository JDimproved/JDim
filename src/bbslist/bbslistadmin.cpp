// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistadmin.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "viewfactory.h"
#include "session.h"
#include "command.h"

BBSLIST::BBSListAdmin *instance_bbslistadmin = NULL;

BBSLIST::BBSListAdmin* BBSLIST::get_admin()
{
    if( ! instance_bbslistadmin ) instance_bbslistadmin = new BBSLIST::BBSListAdmin( URL_BBSLISTADMIN );
    assert( instance_bbslistadmin );

    return instance_bbslistadmin;
}


void BBSLIST::delete_admin()
{
    if( instance_bbslistadmin ) delete instance_bbslistadmin;
    instance_bbslistadmin = NULL;
}


using namespace BBSLIST;

BBSListAdmin::BBSListAdmin( const std::string& url )
    : SKELETON::Admin( url )
{
    get_notebook()->set_dragable( false );
    get_notebook()->set_fixtab( true );
}


BBSListAdmin::~BBSListAdmin()
{
#ifdef _DEBUG    
    std::cout << "BBSListAdmin::~BBSListAdmin\n";
#endif
    // bbslistのページの位置保存
    SESSION::set_bbslist_page( get_current_page() );
}



// 前回開いていたURLを復元
void BBSListAdmin::restore()
{
    COMMAND_ARGS command_arg;
    command_arg.command = "open_view";
    command_arg.arg1 = "true";

    // bbslist
    command_arg.url = URL_BBSLISTVIEW;
    open_view( command_arg );

    // favorite
    command_arg.url = URL_FAVORITEVIEW;
    open_view( command_arg );

    set_command( "set_page", std::string(), MISC::itostr( SESSION::bbslist_page() ) );
    set_command( "hide_tabs" );
}


void BBSListAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_sidebar" );
}


SKELETON::View* BBSListAdmin::create_view( const COMMAND_ARGS& command )
{
    int type = CORE::VIEW_BBSLISTVIEW;
    if( command.url == URL_FAVORITEVIEW ) type = CORE::VIEW_FAVORITELIST;

    CORE::VIEWFACTORY_ARGS view_args;
    view_args.arg1 = command.arg4;
    view_args.arg2 = command.arg5;    

    SKELETON::View* view = CORE::ViewFactory( type, command.url, view_args );
    return view;
}



//
// ローカルなコマンド
//
void BBSListAdmin::command_local( const COMMAND_ARGS& command )
{
    SKELETON::View* view = get_view( command.url );
    if( view ){

        // アイテム追加
        // append_favorite を呼ぶ前に共有バッファにコピーデータをセットしておくこと
        if( command.command  == "append_item" ) view->set_command( "append_item" );

        // お気に入りルート更新チェック
        else if( command.command  == "check_update_root" ) view->set_command( "check_update_root" );
        else if( command.command == "check_update_open_root" ) view->set_command( "check_update_open_root" );
        else if( command.command == "cancel_check_update" ) view->set_command( "cancel_check_update" );

        // XML保存
        else if( command.command  == "save_xml" ) view->set_command( "save_xml" );
    }
}


//
// アイコン表示切り替え
//
void BBSListAdmin::toggle_icon( const std::string& url )
{
    // お気に入り
    SKELETON::View* view = get_view( URL_FAVORITEVIEW );
    if( view ) view->set_command( "toggle_icon", url );
}
