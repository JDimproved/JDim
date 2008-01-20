// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"

#include "dbtree/interface.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"

#include "icons/iconmanager.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "viewfactory.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "session.h"
#include "command.h"

BOARD::BoardAdmin *instance_boardadmin = NULL;

BOARD::BoardAdmin* BOARD::get_admin()
{
    if( ! instance_boardadmin ) instance_boardadmin = new BOARD::BoardAdmin(  URL_BOARDADMIN );
    assert( instance_boardadmin );

    return instance_boardadmin;
}


void BOARD::delete_admin()
{
    if( instance_boardadmin ) delete instance_boardadmin;
    instance_boardadmin = NULL;
}


using namespace BOARD;

BoardAdmin::BoardAdmin( const std::string& url )
    : SKELETON::Admin( url )
{
    set_use_viewhistory( true );
    get_notebook()->set_dragable( true );
    get_notebook()->set_fixtab( false );
    if( ! SESSION::get_show_board_tab() ) get_notebook()->set_show_tabs( false );

    setup_menu( false );
}


BoardAdmin::~BoardAdmin()
{
#ifdef _DEBUG    
    std::cout << "BoardAdmin::~BoardAdmin\n";
#endif

    // 開いているURLを保存
    SESSION::set_board_URLs( get_URLs() );
    SESSION::set_board_locked( get_locked() );
    SESSION::set_board_page( get_current_page() );
}



// 前回開いていたURLを復元
void BoardAdmin::restore()
{
    bool online = SESSION::is_online();
    SESSION::set_online( false );

    std::list< std::string >& list_url = SESSION::get_board_URLs();
    std::list< std::string >::iterator it_url = list_url.begin();

    std::list< bool > list_locked = SESSION::get_board_locked();
    std::list< bool >::iterator it_locked = list_locked.begin();

    for( ; it_url != list_url.end(); ++it_url ){

        // タブのロック状態
        bool lock = false;
        if( it_locked != list_locked.end() ){
            if( (*it_locked ) ) lock = true;
            ++it_locked;
        }

        COMMAND_ARGS command_arg = url_to_openarg( *it_url, true, lock );
        if( ! command_arg.url.empty() ) open_view( command_arg );
    }

    SESSION::set_online( online );
    set_command( "set_page", std::string(), MISC::itostr( SESSION::board_page() ) );
}


COMMAND_ARGS BoardAdmin::url_to_openarg( const std::string& url, const bool tab, const bool lock )
{
    COMMAND_ARGS command_arg;

    command_arg.command = "open_view";
    command_arg.url = url;
    if( tab ) command_arg.arg1 = "true";  // タブで開く
    command_arg.arg2 = "false";           // 既に開いているかチェック
    if( lock ) command_arg.arg3 = "lock";

    return command_arg;
}


void BoardAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_board" );
}


//
// リストで与えられたページをタブで連続して開くとき(Admin::open_list())の引数セット
//
COMMAND_ARGS BoardAdmin::get_open_list_args( const std::string& url )
{
    COMMAND_ARGS command_arg;

    CORE::core_set_command( "set_history_board", url );

    return command_arg;
}



SKELETON::View* BoardAdmin::create_view( const COMMAND_ARGS& command )
{
    CORE::VIEWFACTORY_ARGS view_args;
    view_args.arg1 = command.arg4;
    view_args.arg2 = command.arg5;    

    SKELETON::View* view = CORE::ViewFactory( CORE::VIEW_BOARDVIEW, command.url, view_args );
    return view;
}


//
// ローカルなコマンド
//
void BoardAdmin::command_local( const COMMAND_ARGS& command )
{
    // 列項目の更新
    if( command.command == "update_columns" ){

        std::list< SKELETON::View* > list_view = get_list_view( command.url );
        std::list< SKELETON::View* >::iterator it = list_view.begin();
        for( ; it != list_view.end(); ++it ){
            SKELETON::View* view = ( *it );
            if( view ) view->set_command( "update_columns" );
        }
    }
}


//
// タブのD&Dを開始
//
void BoardAdmin::slot_drag_begin( int page )
{
    SKELETON::View* view = ( SKELETON::View* )get_notebook()->get_nth_page( page );
    if( !view ) return;

    std::string url = view->get_url();
    
    CORE::DND_Begin( get_url() );

    CORE::DATA_INFO info;
    info.type = TYPE_BOARD;
    info.url = DBTREE::url_boardbase( url );
    info.name = DBTREE::board_name( info.url );

#ifdef _DEBUG    
    std::cout << "BoardAdmin::slot_drag_begin " << info.name  << std::endl;
#endif

    CORE::SBUF_clear_info();
    CORE::SBUF_append( info );
}



//
// タブのD&D終了
//
void BoardAdmin::slot_drag_end()
{
#ifdef _DEBUG    
    std::cout << "BoardAdmin::slot_drag_end\n";
#endif

    CORE::DND_End();
}
