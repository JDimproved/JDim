// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "bbslistadmin.h"

#include "skeleton/view.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "viewfactory.h"
#include "session.h"

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
{}


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
    set_command( "open_view", URL_BBSLISTVIEW );
    set_command( "open_view", URL_ETCVIEW, "true" );
    set_command( "open_view", URL_FAVORITEVIEW, "true" );
    set_command( "set_page", std::string(), MISC::itostr( SESSION::bbslist_page() ) );
}


SKELETON::View* BBSListAdmin::create_view( const COMMAND_ARGS& command )
{
    int type = CORE::VIEW_BBSLISTVIEW;
    if( command.url == URL_FAVORITEVIEW ) type = CORE::VIEW_FAVORITELIST;
    else if( command.url == URL_ETCVIEW ) type = CORE::VIEW_ETCLIST;

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

        // お気に入り追加
        // append_favorite を呼ぶ前に共有バッファにコピーデータをセットしておくこと
        if( command.command  == "append_favorite" ) view->set_command( "append_favorite" );
    }

    // お気に入り保存
    else  if( command.command  == "save_favorite" ){

        view = get_view( URL_FAVORITEVIEW );
        if( view ) view->set_command( "save_favorite" );
    }
}
