// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "toolbar.h"

#include "dbtree/interface.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"

#include "icons/iconmanager.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "global.h"
#include "type.h"
#include "viewfactory.h"
#include "sharedbuffer.h"
#include "session.h"
#include "command.h"
#include "dndmanager.h"

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
    : SKELETON::Admin( url ) , m_toolbar( NULL )
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

    if( m_toolbar ) delete m_toolbar;

    // 開いているURLを保存
    SESSION::set_board_URLs( get_URLs() );
    SESSION::set_board_locked( get_locked() );
    SESSION::set_board_page( get_current_page() );
}



// 前回開いていたURLを復元
void BoardAdmin::restore( const bool only_locked )
{
    int set_page_num = 0;
    const bool online = SESSION::is_online();
    SESSION::set_online( false );

    std::list< std::string >& list_url = SESSION::get_board_URLs();
    std::list< std::string >::iterator it_url = list_url.begin();

    std::list< bool > list_locked = SESSION::get_board_locked();
    std::list< bool >::iterator it_locked = list_locked.begin();

    for( int page = 0; it_url != list_url.end(); ++it_url, ++page ){

        // タブのロック状態
        bool lock = false;
        if( it_locked != list_locked.end() ){
            if( (*it_locked ) ) lock = true;
            ++it_locked;
        }

        // ロックされているものだけ表示
        if( only_locked && ! lock ) continue;

        if( page == SESSION::board_page() ) set_page_num = get_tab_nums();

        COMMAND_ARGS command_arg = url_to_openarg( *it_url, true, lock );
        if( ! command_arg.url.empty() ) open_view( command_arg );
    }

    SESSION::set_online( online );
    if( get_tab_nums() ) set_command( "set_page", std::string(), MISC::itostr( set_page_num ) );
}


COMMAND_ARGS BoardAdmin::url_to_openarg( const std::string& url, const bool tab, const bool lock )
{
    JDLIB::Regex regex;

    COMMAND_ARGS command_arg;
    command_arg.command = "open_view";
    command_arg.url = std::string();
    if( tab ) command_arg.arg1 = "true";  // タブで開く
    command_arg.arg2 = "false";           // 既に開いているかチェック
    if( lock ) command_arg.arg3 = "lock"; // 開き方のモード ( Admin::open_view 参照 )

#ifdef _DEBUG
    std::cout << "BoardAdmin::url_to_openarg url = " << url << std::endl;
#endif    

    // 次スレ検索
    if( regex.exec( std::string( "(.*)" ) + NEXT_SIGN + ARTICLE_SIGN + "(.*)" + TIME_SIGN, url )){

        command_arg.url = regex.str( 1 );

        command_arg.arg4 = "NEXT";
        command_arg.arg5 = regex.str( 2 );
    }

    // スレビュー
    else{

        command_arg.url = url;

        command_arg.arg4 = "MAIN";
    }

    return command_arg;
}


void BoardAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_board" );
}


//
// リストで与えられたページをタブで連続して開くとき(Admin::open_list())の引数セット
//
COMMAND_ARGS BoardAdmin::get_open_list_args( const std::string& url, const COMMAND_ARGS& command_list )
{
    COMMAND_ARGS command_arg;

    CORE::core_set_command( "set_history_board", url );

    return command_arg;
}


//
// ツールバー表示
//
void BoardAdmin::show_toolbar()
{
    // まだ作成されていない場合は作成する
    if( ! m_toolbar ){
        m_toolbar = new BoardToolBar();
        get_notebook()->append_toolbar( *m_toolbar );

        if( SESSION::get_show_board_toolbar() ) m_toolbar->show_toolbar();
    }

    get_notebook()->show_toolbar();
}


//
// ツールバー表示切り替え
//
void BoardAdmin::toggle_toolbar()
{
    if( ! m_toolbar ) return;

    if( SESSION::get_show_board_toolbar() ) m_toolbar->show_toolbar();
    else m_toolbar->hide_toolbar();
}


SKELETON::View* BoardAdmin::create_view( const COMMAND_ARGS& command )
{
    int type = CORE::VIEW_NONE; 
    CORE::VIEWFACTORY_ARGS view_args;

    // メインビュー
    if( command.arg4 == "MAIN" ){
        type = CORE::VIEW_BOARDVIEW;
    }
    else if( command.arg4 == "NEXT" ){
        type = CORE::VIEW_BOARDNEXT;
        view_args.arg1 = command.arg5;
    }
    else return NULL;

    SKELETON::View* view = CORE::ViewFactory( type, command.url, view_args );
    assert( view != NULL );

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
// タブをお気に入りにドロップした時にお気に入りがデータ送信を要求してきた
//
void BoardAdmin::slot_drag_data_get( Gtk::SelectionData& selection_data, const int page )
{
#ifdef _DEBUG    
    std::cout << "BoardAdmin::slot_drag_data_get page = " << page  << std::endl;
#endif

    SKELETON::View* view = ( SKELETON::View* )get_notebook()->get_nth_page( page );
    if( ! view ) return;

    const std::string url = view->get_url();
    
    CORE::DATA_INFO info;
    info.type = TYPE_BOARD;
    info.url = DBTREE::url_boardbase( url );
    info.name = DBTREE::board_name( info.url );
    info.data = std::string();
    info.path = Gtk::TreePath( "0" ).to_string();

    if( info.url.empty() ) return;

#ifdef _DEBUG    
    std::cout << "name = " << info.name << std::endl;
#endif

    CORE::DATA_INFO_LIST list_info;
    list_info.push_back( info );
    CORE::SBUF_set_list( list_info );

    selection_data.set( DNDTARGET_FAVORITE, get_url() );
}
