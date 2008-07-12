// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewetc.h"
#include "drawareamain.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "global.h"

#include <sys/time.h>
#include <cstring>

using namespace ARTICLE;


// レス抽出ビュー
ArticleViewRes::ArticleViewRes( const std::string& url,
                                const std::string& num, bool show_title, const std::string& center  )
    : ArticleViewBase( url ),
      m_str_num( num ),
      m_str_center( center ),
      m_show_title( show_title )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    std::string url_tmp = url_article() + ARTICLE_SIGN + RES_SIGN + m_str_num + CENTER_SIGN;
    if( !m_str_center.empty() ) url_tmp += m_str_center;
    else url_tmp += "0";
    url_tmp += TIME_SIGN + MISC::timevaltostr( tv );
    set_url( url_tmp, false );

#ifdef _DEBUG
    std::cout << "ArticleViewRes::ArticleViewRes " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewRes::~ArticleViewRes()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewRes::~ArticleViewRes : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewRes::show_view()
{
    show_res( m_str_num, m_show_title );

    // ラベル更新
    set_label( " [ RES:" + m_str_num + " ] - " + DBTREE::article_subject( url_article() ) );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewRes::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewRes::relayout\n";
#endif

    drawarea()->clear_screen();
    show_res( m_str_num, m_show_title );
    drawarea()->redraw_view();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// 名前抽出ビュー


ArticleViewName::ArticleViewName( const std::string& url, const std::string& name )
    : ArticleViewBase( url ),
      m_str_name( name )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url_article() + ARTICLE_SIGN + NAME_SIGN + m_str_name + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewName::ArticleViewName " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewName::~ArticleViewName()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewName::~ArticleViewName : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewName::show_view()
{
    show_name( m_str_name, true );

    // ラベル更新
    set_label( " [ 名前：" + m_str_name + " ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewName::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewName::relayout\n";
#endif

    drawarea()->clear_screen();
    show_name( m_str_name, true );
    drawarea()->redraw_view();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// ID抽出ビュー


ArticleViewID::ArticleViewID( const std::string& url, const std::string& id )
    : ArticleViewBase( url ),
      m_str_id( id )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url_article() + ARTICLE_SIGN + ID_SIGN + m_str_id + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewID::ArticleViewID " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewID::~ArticleViewID()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewID::~ArticleViewID : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewID::show_view()
{
    show_id( m_str_id, true );

    // ラベル更新
    set_label( " [ ID:" + m_str_id.substr( strlen( PROTO_ID ) ) + " ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewID::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewID::relayout\n";
#endif

    drawarea()->clear_screen();
    show_id( m_str_id, true );
    drawarea()->redraw_view();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// ブックマーク抽出ビュー


ArticleViewBM::ArticleViewBM( const std::string& url )
    : ArticleViewBase( url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    set_url( url_article() + ARTICLE_SIGN + BOOKMK_SIGN + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewBM::ArticleViewBM " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewBM::~ArticleViewBM()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewBM::~ArticleViewBM : " << get_url() << std::endl;
#endif
}



//
// 抽出表示
//
void ArticleViewBM::show_view()
{
    show_bm();

    // ラベル更新
    set_label( " [ しおり ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewBM::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBM::relayout\n";
#endif

    drawarea()->clear_screen();
    show_bm();
    drawarea()->redraw_view();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 書き込み抽出ビュー

ArticleViewPost::ArticleViewPost( const std::string& url )
    : ArticleViewBase( url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    set_url( url_article() + ARTICLE_SIGN + POST_SIGN + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewPost::ArticleViewPost " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewPost::~ArticleViewPost()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPost::~ArticleViewPost : " << get_url() << std::endl;
#endif
}



//
// 抽出表示
//
void ArticleViewPost::show_view()
{
    show_post();

    // ラベル更新
    set_label( " [ 書き込み ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewPost::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewPost::relayout\n";
#endif

    drawarea()->clear_screen();
    show_post();
    drawarea()->redraw_view();
}




////////////////////////////////////////////////////////////////////////////////////////////////////

// URL抽出ビュー

ArticleViewURL::ArticleViewURL( const std::string& url )
    : ArticleViewBase( url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url_article() + ARTICLE_SIGN + URL_SIGN + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewURL::ArticleViewURL " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewURL::~ArticleViewURL()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewURL::~ArticleViewURL : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewURL::show_view()
{
    show_res_with_url();

    // ラベル更新
    set_label( " [ URL ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewURL::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewURL::relayout\n";
#endif

    drawarea()->clear_screen();
    show_res_with_url();
    drawarea()->redraw_view();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// 参照抽出ビュー


ArticleViewRefer::ArticleViewRefer( const std::string& url, const std::string& num )
    : ArticleViewBase( url ),
      m_str_num( num )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    set_url( url_article() + ARTICLE_SIGN + REFER_SIGN + m_str_num + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewRefer::ArticleViewRefer " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewRefer::~ArticleViewRefer()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewRefer::~ArticleViewRefer : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewRefer::show_view()
{
    show_refer( atol( m_str_num.c_str() ) );

    // ラベル更新
    set_label( " [ Re:" + m_str_num + " ] - " + DBTREE::article_subject( url_article() ));

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewRefer::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewRefer::relayout\n";
#endif

    drawarea()->clear_screen();
    show_refer( atol( m_str_num.c_str() ) );
    drawarea()->redraw_view();
}




////////////////////////////////////////////////////////////////////////////////////////////////////

// キーワード抽出ビュー


ArticleViewDrawout::ArticleViewDrawout( const std::string& url, const std::string& query, bool mode_or )
    : ArticleViewBase( url ), m_mode_or( mode_or )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    set_search_query( query );
    set_pre_query( query );

    //viewのURL更新
    std::string url_tmp = url_article() + ARTICLE_SIGN + KEYWORD_SIGN + query + ORMODE_SIGN;
    if( mode_or ) url_tmp += "1";
    else url_tmp += "0";
    url_tmp += TIME_SIGN + MISC::timevaltostr( tv );
    set_url( url_tmp, false );

#ifdef _DEBUG
    std::cout << "ArticleViewDrawout::ArticleViewDrawout " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewDrawout::~ArticleViewDrawout()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewDrawout::~ArticleViewDrawout : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewDrawout::show_view()
{
    drawout_keywords( get_search_query(), m_mode_or, true );

    // ラベル更新
    std::string str_label;
    if( m_mode_or ) str_label = "[ OR 抽出 ] - ";
    else str_label = "[ AND 抽出 ] - ";
    set_label( str_label + DBTREE::article_subject( url_article() ) );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewDrawout::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewDrawout::relayout\n";
#endif

    drawarea()->clear_screen();
    drawarea()->clear_highlight();
    drawout_keywords( get_search_query(), m_mode_or, true );
    drawarea()->redraw_view();
}



////////////////////////////////////////////////////////////////////////////////////////////////////

// 書き込みログ表示ビュー

ArticleViewPostlog::ArticleViewPostlog( const std::string& url, const int num )
    : ArticleViewBase( url ), m_num( num )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url + POSTLOG_SIGN + MISC::itostr( m_num ) + TIME_SIGN + MISC::timevaltostr( tv ), false );

#ifdef _DEBUG
    std::cout << "ArticleViewPostlog::ArticleViewPostlog " << get_url()
              << " num = " << m_num << std::endl;
#endif

    set_id_toolbar( TOOLBAR_SIMPLE );

    setup_view();
}



ArticleViewPostlog::~ArticleViewPostlog()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPostlog::~ArticleViewPostlog : " << get_url() << std::endl;
#endif
}


//
// 抽出表示
//
void ArticleViewPostlog::show_view()
{
    show_postlog( m_num );

    // ラベル更新
    set_label( " [ 書き込みログ ] " );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );

    goto_bottom();
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewPostlog::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewPostlog::relayout\n";
#endif

    drawarea()->clear_screen();
    show_postlog( m_num );
    drawarea()->redraw_view();
    goto_bottom();
}

