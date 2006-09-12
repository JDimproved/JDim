// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "font.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"

#include "icons/iconmanager.h"

#include "global.h"
#include "viewfactory.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "session.h"
#include "command.h"


ARTICLE::ArticleAdmin *instance_articleadmin = NULL;

ARTICLE::ArticleAdmin* ARTICLE::get_admin()
{
    if( ! instance_articleadmin ) instance_articleadmin = new ARTICLE::ArticleAdmin( URL_ARTICLEADMIN );
    assert( instance_articleadmin );

    return instance_articleadmin;
}

void ARTICLE::delete_admin()
{
    if( instance_articleadmin ) delete instance_articleadmin;
    instance_articleadmin = NULL;
}


using namespace ARTICLE;

ArticleAdmin::ArticleAdmin( const std::string& url )
    : SKELETON::Admin( url )
{
    ARTICLE::init_font();

    get_notebook()->set_dragable( true );
    get_notebook()->set_fixtab( false );
}


ArticleAdmin::~ArticleAdmin()
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::~ArticleAdmin\n";
#endif

    SESSION::set_article_URLs( get_URLs() );
    SESSION::set_article_page( get_current_page() );
    ARTICLE::init_font();
}


// 前回開いていたURLを復元
void ArticleAdmin::restore()
{
#ifdef _DEBUG
    std::cout << "ArticleAdmin::restore\n";
#endif

    JDLIB::Regex regex;

    bool online = SESSION::is_online();
    SESSION::set_online( false );

    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    list_tmp = SESSION::article_URLs();
    it_tmp = list_tmp.begin();
    for( ; it_tmp != list_tmp.end(); ++it_tmp ){

        std::string url = (*it_tmp);
        COMMAND_ARGS command_arg;
        command_arg.command = "open_view";
        command_arg.url = std::string();

        // レス抽出
        if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + RES_SIGN + "(.*)"
                        + CENTER_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "RES";
            command_arg.arg5 = regex.str( 2 );
            if( regex.str( 3 ) != "0" ) command_arg.arg6 = regex.str( 3 );
        }

        // ID抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + ID_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "ID";
            command_arg.arg5 = regex.str( 2 );
        }

        // ブックマーク抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + BOOKMK_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "BM";
        }

        // URL抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + URL_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "URL";
        }

        // 参照
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + REFER_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "REF";
            command_arg.arg5 = regex.str( 2 );
        }

        // キーワード
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + KEYWORD_SIGN + "(.*)"
                             + ORMODE_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            if( regex.str( 3 ) == "1" ) command_arg.arg4 = "KEYWORD_OR";
            else command_arg.arg4 = "KEYWORD";
            command_arg.arg5 = regex.str( 2 );
        }

        // MAIN
        else if( !url.empty() ){
            command_arg.url = url;
            command_arg.arg1 = "true";   // タブで開く
            command_arg.arg2 = "false";  // 既に開いているかチェック
            command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

            command_arg.arg4 = "MAIN";
        }

#ifdef _DEBUG
        std::cout << command_arg.url << std::endl
                  << command_arg.arg1 << std::endl
                  << command_arg.arg2 << std::endl
                  << command_arg.arg3 << std::endl
                  << command_arg.arg4 << std::endl
                  << command_arg.arg5 << std::endl;
#endif

        if( ! command_arg.url.empty() ) open_view( command_arg );
    }



    SESSION::set_online( online );
    set_command( "set_page", std::string(), MISC::itostr( SESSION::article_page() ) );
}


void ArticleAdmin::switch_admin()
{
    CORE::core_set_command( "switch_article" );
}



//
// タブにアイコンをセットする
//
void ArticleAdmin::set_tabicon( const std::string& url, const std::string& iconname )
{
    SKELETON::View* view = get_view( url );
    if( view ) get_notebook()->set_tabicon( iconname, get_notebook()->page_num( *view ),
                                            ICON::THREAD, ICON::THREAD_UPDATE );
}



// リストで与えられたページをタブで連続して開く
//
// 連続してリロードかけるとサーバに負担をかけるので、オフラインで開いて
// タイミングをずらしながらリロードする
//
void ArticleAdmin::open_list( const std::string& str_list )
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
        command_arg.arg2 = "false";  // 既に開いているかチェック
        command_arg.arg3 = "false";  // オフラインで開く(上でオフラインにしているので関係なし)

        command_arg.arg4 = "MAIN";

        open_view( command_arg );
        CORE::core_set_command( "set_history_article", command_arg.url );

        // 一番最初のスレは普通にオンラインで開く
        // 二番目からは ウェイトを入れてリロード
        if( !waittime ) SESSION::set_online( false );
        else set_autoreload_mode( command_arg.url, AUTORELOAD_ONCE, waittime );
    }

    SESSION::set_online( online );
    switch_view( *( list_url.begin() ) );
}



//
// カレントビューでポップアップ表示していたら消す
//
void ArticleAdmin::delete_popup()
{
    SKELETON::View* view = get_current_view();
    if( view ) view->set_command( "delete_popup" );
}


//
// view の作成
//
SKELETON::View* ArticleAdmin::create_view( const COMMAND_ARGS& command )
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::create_view : " << command.arg4 << std::endl;
#endif

    delete_popup();

    int type = CORE::VIEW_NONE; 
    CORE::VIEWFACTORY_ARGS view_args;
    std::string str_jump;

    // メインビュー
    if( command.arg4 == "MAIN" ){
        type = CORE::VIEW_ARTICLEVIEW;
    }

    // キーワード抽出ビュー(AND)
    else if( command.arg4 == "KEYWORD" ){
        type = CORE::VIEW_ARTICLEDRAWOUT;
        view_args.arg1 = command.arg5;  // query
        view_args.arg2 = "AND";
    }

    // キーワード抽出ビュー(OR)
    else if( command.arg4 == "KEYWORD_OR" ){
        type = CORE::VIEW_ARTICLEDRAWOUT;
        view_args.arg1 = command.arg5; // query
        view_args.arg2 = "OR";
    }

    // レス抽出ビュー
    if( command.arg4 == "RES" ){
        type = CORE::VIEW_ARTICLERES;
        view_args.arg1 = command.arg5; // レス番号 ( from-to )
        view_args.arg2 = "false";
        view_args.arg3 = command.arg6; // ジャンプ番号
        str_jump = command.arg6;
    }

    // ID 抽出ビュー
    if( command.arg4 == "ID" ){
        type = CORE::VIEW_ARTICLEID;
        view_args.arg1 = command.arg5; // ID
    }

    // ブックマーク抽出ビュー
    if( command.arg4 == "BM" ){
        type = CORE::VIEW_ARTICLEBM;
    }

    // URL抽出ビュー
    if( command.arg4 == "URL" ){
        type = CORE::VIEW_ARTICLEURL;
    }

    // 参照抽出ビュー
    if( command.arg4 == "REF" ){
        type = CORE::VIEW_ARTICLEREFER;
        view_args.arg1 = command.arg5; // 対象レス番号
    }

    SKELETON::View* view = CORE::ViewFactory( type, command.url, view_args );
    assert( view != NULL );

    // ジャンプ
    if( ! str_jump.empty() ){
#ifdef _DEBUG    
        std::cout << "goto " << str_jump << std::endl;
#endif
        set_command( "goto_num", view->get_url(), str_jump );
    }

    return view;
}


//
// ローカルなコマンド
//
void ArticleAdmin::command_local( const COMMAND_ARGS& command )
{
    if( command.command == "goto_num" ){
        SKELETON::View* view = get_view( command.url );
        if( view ) view->set_command( "goto_num", command.arg1 );
    }

    // ポップアップ消去
    else if( command.command == "delete_popup" ) delete_popup();

    // command.url を含むビューを全て再レイアウト
    else if( command.command == "relayout_views" ){

        std::list< SKELETON::View* > list_view = get_list_view( command.url );
        std::list< SKELETON::View* >::iterator it = list_view.begin();

        for( ; it != list_view.end(); ++it ){
            SKELETON::View* view = ( *it );
            if( view ) view->relayout();
        }
    }

    // フォント初期化
    else if( command.command == "init_font" ) ARTICLE::init_font();
}





//
// タブのD&Dを開始
//
void ArticleAdmin::slot_drag_begin( int page )
{
    SKELETON::View* view = ( SKELETON::View* )get_notebook()->get_nth_page( page );
    if( !view ) return;

    std::string url = view->get_url();
    
    CORE::DND_Begin( get_url() );

    CORE::DATA_INFO info;
    info.type = TYPE_THREAD;
    info.url = DBTREE::url_readcgi( url, 0, 0 );
    info.name = DBTREE::article_subject( info.url );

#ifdef _DEBUG    
    std::cout << "ArticleAdmin::slot_drag_begin " << info.name  << std::endl;
#endif

    CORE::SBUF_clear_info();
    CORE::SBUF_append( info );
}



//
// タブのD&D終了
//
void ArticleAdmin::slot_drag_end()
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::slot_drag_end\n";
#endif

    CORE::DND_End();
}
