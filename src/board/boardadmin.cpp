// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"

#include "dbtree/interface.h"

#include "skeleton/view.h"

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
    //D&D可
    get_notebook().set_dragable( true );
}


BoardAdmin::~BoardAdmin()
{
#ifdef _DEBUG    
    std::cout << "BoardAdmin::~BoardAdmin\n";
#endif

    // 開いているURLを保存
    SESSION::set_board_URLs( get_URLs() );
    SESSION::set_board_page( get_current_page() );
}



// 前回開いていたURLを復元
void BoardAdmin::restore()
{
    bool online = SESSION::is_online();
    SESSION::set_online( false );

    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    list_tmp = SESSION::board_URLs();
    it_tmp = list_tmp.begin();
    for( ; it_tmp != list_tmp.end(); ++it_tmp ){

        if( !(*it_tmp).empty() ){
            COMMAND_ARGS command_arg;
            command_arg.command = "open_view";
            command_arg.url = (*it_tmp);
            command_arg.arg1 = "true";
            command_arg.arg2 = "false"; // オフラインで開く(上でオフラインにしているので関係なし)

            open_view( command_arg );
        }
    }

    SESSION::set_online( online );
    set_command( "set_page", std::string(), MISC::itostr( SESSION::board_page() ) );
}


void BoardAdmin::switch_admin()
{
    CORE::core_set_command( "switch_board" );
}



// リストで与えられたページをタブで連続して開く
//
// 連続してリロードかけるとサーバに負担をかけるので、オフラインで開いて
// タイミングをずらしながらリロードする
//
void BoardAdmin::open_list( const std::string& str_list )
{
    std::list< std::string > list_url = MISC::split_line( str_list );
    if( list_url.empty() ) return;

    int waittime = 0;
    bool online = SESSION::is_online();

    std::list< std::string >::iterator it = list_url.begin();
    for( ; it != list_url.end(); ++it, waittime += AUTORELOAD_MINSEC ){

        COMMAND_ARGS command_arg;
        command_arg.command = "open_view";
        command_arg.url = (*it);
        command_arg.arg1 = "true";   // タブで開く
        command_arg.arg2 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

        open_view( command_arg );
        CORE::core_set_command( "set_history_board", command_arg.url );

        // 一番最初のスレは普通にオンラインで開く
        // 二番目からは ウェイトを入れてリロード
        if( !waittime ) SESSION::set_online( false );
        else set_autoreload_mode( command_arg.url, AUTORELOAD_ONCE, waittime );
    }

    SESSION::set_online( online );
    switch_view( *( list_url.begin() ) );
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
// タブのD&Dを開始
//
void BoardAdmin::slot_drag_begin( int page )
{
    SKELETON::View* view = ( SKELETON::View* )get_notebook().get_nth_page( page );
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
