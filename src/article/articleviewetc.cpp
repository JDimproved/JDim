// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewetc.h"
#include "drawareamain.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "control/controlid.h"
#include "jdlib/miscutil.h"

#include "global.h"

#include <cstring>

using namespace ARTICLE;


// レス抽出ビュー
ArticleViewRes::ArticleViewRes( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
    const int pos0 = url.find( RES_SIGN ) + strlen( RES_SIGN );
    const int pos1 = url.find( CENTER_SIGN );

    m_str_num = url.substr( pos0, pos1 - pos0  );
    m_str_center = url.substr( pos1 + strlen( CENTER_SIGN ) );

#ifdef _DEBUG
    std::cout << "ArticleViewRes::ArticleViewRes " << get_url()
              << " num = " << m_str_num << " center = " << m_str_center << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ RES:" + m_str_num + " ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewRes::~ArticleViewRes()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewRes::~ArticleViewRes : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewRes::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewRes::relayout\n";
#endif

    drawarea()->clear_screen();
    show_res( m_str_num, false );
    drawarea()->redraw_view();

    ARTICLE::get_admin()->set_command( "goto_num", get_url(), m_str_center );
}


//
// 抽出表示
//
void ArticleViewRes::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewRes::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewRes::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 名前抽出ビュー


ArticleViewName::ArticleViewName( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
    , m_str_name{ url.substr( url.find( NAME_SIGN ) + strlen( NAME_SIGN ) ) }
{

#ifdef _DEBUG
    std::cout << "ArticleViewName::ArticleViewName " << get_url()
              << " name = " << m_str_name << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ 名前：" + m_str_name + " ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewName::~ArticleViewName()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewName::~ArticleViewName : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewName::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewName::relayout\n";
#endif

    drawarea()->clear_screen();
    show_name( m_str_name, true );
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewName::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewName::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewName::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// ID抽出ビュー


ArticleViewID::ArticleViewID( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
    , m_str_id{ url.substr( url.find( ID_SIGN ) + strlen( ID_SIGN ) ) }
{

#ifdef _DEBUG
    std::cout << "ArticleViewID::ArticleViewID " << get_url() << " ID = " << m_str_id << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ " + m_str_id.substr( strlen( PROTO_ID ) ) + " ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewID::~ArticleViewID()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewID::~ArticleViewID : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewID::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewID::relayout\n";
#endif

    drawarea()->clear_screen();
    show_id( m_str_id, true );
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewID::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewID::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewID::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// ブックマーク抽出ビュー


ArticleViewBM::ArticleViewBM( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBM::ArticleViewBM " << get_url() << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ しおり ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewBM::~ArticleViewBM()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewBM::~ArticleViewBM : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewBM::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBM::relayout\n";
#endif

    drawarea()->clear_screen();
    show_bm();
    drawarea()->redraw_view();
}



//
// 抽出表示
//
void ArticleViewBM::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewBM::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewBM::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 書き込み抽出ビュー

ArticleViewPost::ArticleViewPost( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
#ifdef _DEBUG
    std::cout << "ArticleViewPost::ArticleViewPost " << get_url() << std::endl;
#endif

    setup_view();


    // ラベル更新
    set_label( " [ 書き込み ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewPost::~ArticleViewPost()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPost::~ArticleViewPost : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewPost::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewPost::relayout\n";
#endif

    drawarea()->clear_screen();
    show_post();
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewPost::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewPost::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewPost::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 高参照レス抽出ビュー

ArticleViewHighRefRes::ArticleViewHighRefRes( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
#ifdef _DEBUG
    std::cout << "ArticleViewHighRefRes::ArticleViewHighRefRes " << get_url() << std::endl;
#endif

    setup_view();


    // ラベル更新
    set_label( " [ 高参照レス ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewHighRefRes::~ArticleViewHighRefRes()
{

#ifdef _DEBUG
    std::cout << "ArticleViewHighRefRes::~ArticleViewHighRefRes : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewHighRefRes::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewHighRefRes::relayout\n";
#endif

    drawarea()->clear_screen();
    show_highly_referenced_res();
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewHighRefRes::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewHighRefRes::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewHighRefRes::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// URL抽出ビュー

ArticleViewURL::ArticleViewURL( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
#ifdef _DEBUG
    std::cout << "ArticleViewURL::ArticleViewURL " << get_url() << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ URL ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewURL::~ArticleViewURL()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewURL::~ArticleViewURL : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewURL::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewURL::relayout\n";
#endif

    drawarea()->clear_screen();
    show_res_with_url();
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewURL::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewURL::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewURL::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 参照抽出ビュー


ArticleViewRefer::ArticleViewRefer( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
    , m_str_num{ url.substr( url.find( REFER_SIGN ) + strlen( REFER_SIGN ) ) }
{

#ifdef _DEBUG
    std::cout << "ArticleViewRefer::ArticleViewRefer " << get_url()
              << " num = " << m_str_num << std::endl;
#endif

    setup_view();

    // ラベル更新
    set_label( " [ Re:" + m_str_num + " ] - " + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewRefer::~ArticleViewRefer()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewRefer::~ArticleViewRefer : " << get_url() << std::endl;
#endif
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewRefer::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewRefer::relayout\n";
#endif

    drawarea()->clear_screen();
    show_refer( atol( m_str_num.c_str() ) );
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewRefer::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewRefer::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewRefer::exec_reload()
{
    relayout();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// キーワード抽出ビュー


ArticleViewDrawout::ArticleViewDrawout( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( ARTICLE_SIGN ) ) )
{
    const int pos0 = url.find( KEYWORD_SIGN ) + strlen( KEYWORD_SIGN );
    const int pos1 = url.find( ORMODE_SIGN );

    m_query = url.substr( pos0, pos1 - pos0  );
    m_mode_or = ( url.substr( pos1 + strlen( ORMODE_SIGN ) ) == "1" );

    set_search_query( m_query );
    set_pre_query( m_query );

#ifdef _DEBUG
    std::cout << "ArticleViewDrawout::ArticleViewDrawout " << get_url() << std::endl;
#endif

    setup_view();

    // ラベル更新
    std::string str_label;
    if( m_mode_or ) str_label = "[ OR 抽出 ] - ";
    else str_label = "[ AND 抽出 ] - ";
    set_label( str_label + MISC::to_markup( DBTREE::article_modified_subject( url_article() ) ), true );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewDrawout::~ArticleViewDrawout()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewDrawout::~ArticleViewDrawout : " << get_url() << std::endl;
#endif
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewDrawout::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewDrawout::relayout\n";
#endif

    drawarea()->clear_screen();
    drawarea()->clear_highlight();
    drawout_keywords( m_query, m_mode_or, true );
    drawarea()->redraw_view();
}


//
// 抽出表示
//
void ArticleViewDrawout::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewDrawout::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewDrawout::exec_reload()
{
    relayout();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 書き込みログ表示ビュー
ArticleViewPostlog::ArticleViewPostlog( const std::string& url )
    : ArticleViewBase( url, url.substr( 0, url.find( POSTLOG_SIGN ) ) )
{
    m_num = atoi( url.substr( url.find( POSTLOG_SIGN ) + strlen( POSTLOG_SIGN ) ).c_str() );

#ifdef _DEBUG
    std::cout << "ArticleViewPostlog::ArticleViewPostlog " << get_url()
              << " num = " << m_num << std::endl;
#endif

    set_id_toolbar( TOOLBAR_SIMPLE );
    set_writeable( false );

    setup_view();

    // ラベル更新
    set_label( " [ 書き込みログ ] " );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}



ArticleViewPostlog::~ArticleViewPostlog()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPostlog::~ArticleViewPostlog : " << get_url() << std::endl;
#endif
}


//
// 検索entryの操作
//
void ArticleViewPostlog::operate_search( const std::string& controlid )
{
    const int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ){
        focus_view();
        ARTICLE::get_admin()->set_command( "close_searchbar" );
    }
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewPostlog::relayout( const bool completely )
{
#ifdef _DEBUG
    std::cout << "ArticleViewPostlog::relayout\n";
#endif

    drawarea()->clear_screen();
    show_postlog( m_num );
    drawarea()->redraw_view();
    goto_bottom();
}


//
// 抽出表示
//
void ArticleViewPostlog::show_view()
{
    relayout();
}


//
// 再読み込みボタンを押した
//
void ArticleViewPostlog::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
void ArticleViewPostlog::exec_reload()
{
    relayout();
}

