// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistadmin.h"
#include "toolbar.h"

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
    : SKELETON::Admin( url ), m_toolbar( NULL )
{
    get_notebook()->set_dragable( false );
    get_notebook()->set_fixtab( true );
}


BBSListAdmin::~BBSListAdmin()
{
#ifdef _DEBUG    
    std::cout << "BBSListAdmin::~BBSListAdmin\n";
#endif

    if( m_toolbar ) delete m_toolbar;

    // bbslistのページの位置保存
    SESSION::set_bbslist_page( get_current_page() );
}



// 前回開いていたURLを復元
void BBSListAdmin::restore( const bool only_locked )
{
    COMMAND_ARGS command_arg;

    // bbslist
    command_arg = url_to_openarg( URL_BBSLISTVIEW, true, false );
    open_view( command_arg );

    // favorite
    command_arg = url_to_openarg( URL_FAVORITEVIEW, true, false );
    open_view( command_arg );

    set_command( "set_page", std::string(), MISC::itostr( SESSION::bbslist_page() ) );
    set_command( "hide_tabs" );
}


COMMAND_ARGS BBSListAdmin::url_to_openarg( const std::string& url, const bool tab, const bool lock )
{
    COMMAND_ARGS command_arg;

    command_arg.command = "open_view";
    command_arg.url = url;
    if( tab ) command_arg.arg1 = "true";  // タブで開く

    return command_arg;
}


void BBSListAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_sidebar" );
}


//
// ツールバー表示
//
void BBSListAdmin::show_toolbar()
{
    // まだ作成されていない場合は作成する
    if( ! m_toolbar ){
        m_toolbar = new BBSListToolBar();
        get_notebook()->append_toolbar( *m_toolbar );

        if( SESSION::get_show_bbslist_toolbar() ) m_toolbar->show_toolbar();
    }

    get_notebook()->show_toolbar();
}


//
// ツールバー表示切り替え
//
void BBSListAdmin::toggle_toolbar()
{
    if( ! m_toolbar ) return;

    if( SESSION::get_show_bbslist_toolbar() ) m_toolbar->show_toolbar();
    else m_toolbar->hide_toolbar();
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

        // お気に入りのスレの url と 名前を変更
        else if( command.command == "replace_thread" ) view->set_command( "replace_thread", command.arg1, command.arg2 );

        // XML保存
        else if( command.command  == "save_xml" ) view->set_command( "save_xml" );
    }
}


//
// (お気に入りの)アイコン表示切り替え
//
void BBSListAdmin::toggle_icon( const std::string& url )
{
    SKELETON::View* view = get_view( URL_FAVORITEVIEW );
    if( view ) view->set_command( "toggle_icon", url );
}
