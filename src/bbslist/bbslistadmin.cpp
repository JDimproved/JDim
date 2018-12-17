// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistadmin.h"
#include "bbslistviewbase.h"
#include "toolbar.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"
#include "skeleton/undobuffer.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "viewfactory.h"
#include "session.h"
#include "command.h"


// お気に入りの共通UNDOバッファ
SKELETON::UNDO_BUFFER *instance_undo_buffer_favorite = NULL;

SKELETON::UNDO_BUFFER* BBSLIST::get_undo_buffer_favorite()
{
    if( ! instance_undo_buffer_favorite ) instance_undo_buffer_favorite = new SKELETON::UNDO_BUFFER();
    assert( instance_undo_buffer_favorite );

    return instance_undo_buffer_favorite;
}

void BBSLIST::delete_undo_buffer_favorite()
{
    if( instance_undo_buffer_favorite ) delete instance_undo_buffer_favorite;
    instance_undo_buffer_favorite = NULL;
}


//////////////////////////////////////////////


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


//////////////////////////////////////////////


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
    BBSLIST::delete_undo_buffer_favorite();
}


void BBSListAdmin::save_session()
{
    Admin::save_session();

    // bbslistのページの位置保存
    SESSION::set_bbslist_page( get_current_page() );
}


// 前回開いていたURLを復元
void BBSListAdmin::restore( const bool only_locked )
{
    COMMAND_ARGS command_arg;

    // 板一覧
    // 初回起動時は板一覧の読み込みもここで行われる
    command_arg = url_to_openarg( URL_BBSLISTVIEW, true, false );
    open_view( command_arg );

    // お気に入り
    command_arg = url_to_openarg( URL_FAVORITEVIEW, true, false );
    open_view( command_arg );

    // スレ履歴
    command_arg = url_to_openarg( URL_HISTTHREADVIEW, true, false );
    open_view( command_arg );

    // 板履歴
    command_arg = url_to_openarg( URL_HISTBOARDVIEW, true, false );
    open_view( command_arg );

    // 最近閉じたスレ
    command_arg = url_to_openarg( URL_HISTCLOSEVIEW, true, false );
    open_view( command_arg );

    // 最近閉じた板
    command_arg = url_to_openarg( URL_HISTCLOSEBOARDVIEW, true, false );
    open_view( command_arg );

    // 最近閉じた画像
    command_arg = url_to_openarg( URL_HISTCLOSEIMGVIEW, true, false );
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

        if( SESSION::get_show_bbslist_toolbar() ) m_toolbar->open_buttonbar();
    }

    get_notebook()->show_toolbar();
}


//
// ツールバー表示/非表示切り替え
//
void BBSListAdmin::toggle_toolbar()
{
    if( ! m_toolbar ) return;

    if( SESSION::get_show_bbslist_toolbar() ){
        m_toolbar->open_buttonbar();
        m_toolbar->show_toolbar();
    }
    else m_toolbar->close_buttonbar();
}


SKELETON::View* BBSListAdmin::create_view( const COMMAND_ARGS& command )
{
    int type = CORE::VIEW_NONE;

    if( command.url == URL_BBSLISTVIEW ) type = CORE::VIEW_BBSLISTVIEW;
    if( command.url == URL_FAVORITEVIEW ) type = CORE::VIEW_FAVORITELIST;
    if( command.url == URL_HISTTHREADVIEW ) type = CORE::VIEW_HISTTHREAD;
    if( command.url == URL_HISTCLOSEVIEW ) type = CORE::VIEW_HISTCLOSE;
    if( command.url == URL_HISTBOARDVIEW ) type = CORE::VIEW_HISTBOARD;
    if( command.url == URL_HISTCLOSEBOARDVIEW ) type = CORE::VIEW_HISTCLOSEBOARD;
    if( command.url == URL_HISTCLOSEIMGVIEW ) type = CORE::VIEW_HISTCLOSEIMG;

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
        // 共有バッファにコピーデータをセットしておくこと
        if( command.command  == "append_item" ) view->set_command( "append_item" );

        // 履歴のセット
        // 先頭にアイテムを追加する。ツリーにアイテムが含まれている場合は移動する
        // 共有バッファにコピーデータをセットしておくこと
        else if( command.command  == "append_history" ) view->set_command( "append_history" );

        // 項目削除
        else if( command.command  == "remove_item" ) view->set_command( "remove_item", command.arg1 );

        // 先頭項目削除
        else if( command.command  == "remove_headitem" ) view->set_command( "remove_headitem" );

        // 全項目削除
        else if( command.command  == "remove_allitems" ) view->set_command( "remove_allitems" );

        // ツリーの編集
        else if( command.command  == "edit_tree" ) view->set_command( "edit_tree" );

        // お気に入りルート更新チェック
        else if( command.command  == "check_update_root" ) view->set_command( "check_update_root" );
        else if( command.command == "check_update_open_root" ) view->set_command( "check_update_open_root" );
        else if( command.command == "cancel_check_update" ) view->set_command( "cancel_check_update" );

        // お気に入りのスレの url と 名前を変更
        else if( command.command == "replace_thread" ) view->set_command( "replace_thread", command.arg1, command.arg2 );

        // XML保存
        else if( command.command  == "save_xml" ) view->set_command( "save_xml" );

        // スレのアイコン表示を更新
        else if( command.command  == "toggle_articleicon" ) view->set_command( "toggle_articleicon", command.arg1 );

        // 板のアイコン表示を更新
        else if( command.command  == "toggle_boardicon" ) view->set_command( "toggle_boardicon", command.arg1 );

        // URLを選択
        else if( command.command  == "select_item" ) view->set_command( "select_item", command.arg1 );
    }
}


// 履歴を DATA_INFO_LIST 型で取得
void BBSListAdmin::get_history( const std::string& url, CORE::DATA_INFO_LIST& info_list )
{
    info_list.clear();

    BBSListViewBase* view = dynamic_cast< BBSListViewBase* >( get_view( url ) );
    if( view ) return view->get_history( info_list );
}


// サイドバーの指定したidのディレクトリに含まれるスレのアドレスを取得
void BBSListAdmin::get_threads( const std::string& url, const int dirid, std::vector< std::string >& list_url )
{
    list_url.clear();

    BBSListViewBase* view = dynamic_cast< BBSListViewBase* >( get_view( url ) );
    if( view ) view->get_threads( dirid, list_url );
}


// サイドバーの指定したidのディレクトリの名前を取得
std::string BBSListAdmin::get_dirname( const std::string& url, const int dirid )
{
    BBSListViewBase* view = dynamic_cast< BBSListViewBase* >( get_view( url ) );
    if( view ) return view->get_dirname( dirid );

    return std::string();
}

