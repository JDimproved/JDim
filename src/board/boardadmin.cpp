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
#include "jdlib/miscmsg.h"

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

    setup_menu();
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

    const std::list< std::string >& list_url = SESSION::get_board_URLs();
    std::list< std::string >::const_iterator it_url = list_url.begin();

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

        // 板がDBに登録されていない場合は表示しない
        if( command_arg.url != URL_ALLLOG && DBTREE::url_boardbase( command_arg.url ).empty() ){
            MISC::ERRMSG(  *it_url + " is not registered" );
            continue;
        }

        open_view( command_arg );
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
    if( regex.exec( std::string( "(.*)" ) + NEXT_SIGN + ARTICLE_SIGN + "(.*)", url )){

        command_arg.url = regex.str( 1 );

        command_arg.arg4 = "NEXT";
        command_arg.arg5 = regex.str( 2 ); // 前スレのアドレス
    }

    // 全ログ一覧
    else if( url == URL_ALLLOG ){

        command_arg.url = URL_ALLLOG;

        command_arg.arg4 = "LOG";
    }

    // ログ一覧
    else if( regex.exec( std::string( "(.*)" ) + LOG_SIGN, url )){

        command_arg.url = regex.str( 1 );

        command_arg.arg4 = "LOG";
    }

    // スレビュー
    else{

        command_arg.url = url;

        command_arg.arg4 = "MAIN";
    }

#ifdef _DEBUG
    std::cout << "open " << command_arg.arg4 << std::endl;
#endif    

    return command_arg;
}


const std::string BoardAdmin::command_to_url( const COMMAND_ARGS& command )
{
    if( command.arg4 == "NEXT" ) return command.url + NEXT_SIGN + ARTICLE_SIGN + command.arg5;

    else if( command.arg4 == "LOG" ){

        if( command.url == URL_ALLLOG ) return URL_ALLLOG;
        else return command.url + LOG_SIGN;
    }

    return command.url;
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
    command_arg.arg4 = "MAIN";

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

        if( SESSION::get_show_board_toolbar() ) m_toolbar->open_buttonbar();
    }

    get_notebook()->show_toolbar();
}


//
// ツールバー表示/非表示切り替え
//
void BoardAdmin::toggle_toolbar()
{
    if( ! m_toolbar ) return;

#ifdef _DEBUG    
    std::cout << "BoardAdmin::toggle_toolbar\n";
#endif

    // 検索関係の wiget の位置を変更
    m_toolbar->unpack_pack();

    if( SESSION::get_show_board_toolbar() ) m_toolbar->open_buttonbar();
    else m_toolbar->close_buttonbar();

    m_toolbar->close_searchbar();
    m_toolbar->show_toolbar();
}


//
// 検索バー表示
//
void BoardAdmin::open_searchbar()
{
    if( ! m_toolbar ) return;

    // ツールバー表示時は検索関係の wiget はツールバーに表示されている
    if( ! SESSION::get_show_board_toolbar() ){
        m_toolbar->open_searchbar();
        m_toolbar->show_toolbar();
    }

    m_toolbar->focus_entry_search();
}


//
// 検索バー非表示
//
void BoardAdmin::close_searchbar()
{
    if( ! m_toolbar ) return;

    if( ! SESSION::get_show_board_toolbar() ) m_toolbar->close_searchbar();
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
        view_args.arg1 = command.arg5;  // 前スレのアドレス
    }
    else if( command.arg4 == "LOG" ){
        type = CORE::VIEW_BOARDLOG;
    }
    else return NULL;

    SKELETON::View* view = CORE::ViewFactory( type, command_to_url( command ), view_args );
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

    // 指定したスレを強調して表示
    else if( command.command == "draw_bg_articles" ){

        SKELETON::View* view = get_view( command.url );
        if( view ) view->set_command( "draw_bg_articles" );
    }
}


//
// タブをサイドバーにドロップした時にお気に入りがデータ送信を要求してきた
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
