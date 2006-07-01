// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleview.h"
#include "drawareamain.h"
#include "toolbar.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "jdlib/misctime.h"

#include "command.h"
#include "global.h"

#include <sstream>
#include <sys/time.h>


using namespace ARTICLE;


// メインビュー

ArticleViewMain::ArticleViewMain( const std::string& url )
    :  ArticleViewBase( url ), m_gotonum_reserve( 0 )
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::ArticleViewMain " << get_url() << " url_article = " << url_article() << std::endl;
#endif

    setup_view();
}



ArticleViewMain::~ArticleViewMain()
{
#ifdef _DEBUG    
    std::cout << "ArticleViewMain::~ArticleViewMain : " << get_url() << " url_article = " << url_article() << std::endl;
#endif
    int seen = drawarea()->seen_current();
        
#ifdef _DEBUG    
        std::cout << "set seen to " << seen << std::endl;
#endif
        if( seen >= 1 ) get_article()->set_number_seen( seen );
}


//
// num 番にジャンプ
//
// ローディング中ならジャンプ予約をしてロード後に update_finish() の中で改めて goto_num() を呼び出す
//
void ArticleViewMain::goto_num( int num )
{
    if( get_article()->get_number_load() < num  && get_article()->is_loading() ){

        m_gotonum_reserve = num;
        return;
    }

    m_gotonum_reserve = 0;
    ArticleViewBase::goto_num( num );
}



//
// 再読み込み
//
void ArticleViewMain::reload()
{
    View::reset_autoreload_counter();

    // DAT落ちしてるとロードしないので状態をリセットしておく
    get_article()->reset_status();
    show_view();
    CORE::core_set_command( "set_history_article", url_article() );
}



//
//  キャッシュ表示 & 差分ロード開始
//
void ArticleViewMain::show_view()
{
    m_gotonum_reserve = 0;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_view\n";
#endif
    
    CORE::core_set_command( "switch_article" );

    if( get_url().empty() ){
        set_status( "invalid URL" );
        CORE::core_set_command( "set_status","", get_status() );
        return;
    }

    // キャッシュに含まれているレスを表示
    int from_num = drawarea()->max_number() + 1;
    int to_num = get_article()->get_number_load();
    if( from_num <= to_num ){

        drawarea()->append_res( from_num, to_num );

        // 以前見ていたところにジャンプ
        drawarea()->goto_num( get_article()->get_number_seen() );
    }

    // セパレータを最後に移動
    drawarea()->set_separator_new( to_num + 1 );

    update_finish();    
    
    // 差分 download 開始
    get_article()->download_dat();
    if( get_article()->is_loading() ){
        set_status( "loading..." );
        CORE::core_set_command( "set_status","", get_status() );
    }
}



//
// ロード中にノード構造が変わったら呼ばれる
//
void ArticleViewMain::update_view()
{
    int num_from = drawarea()->max_number() + 1;
    int num_to = get_article()->get_number_load();

#ifdef _DEBUG
    std::cout << "ArticleViewMain::update_view : from " << num_from << " to " << num_to << std::endl;
#endif

    if( num_from > num_to ) return;

    drawarea()->append_res( num_from, num_to );
    drawarea()->redraw_view();
}



//
// ロードが終わったときに呼ばれる
//
void ArticleViewMain::update_finish()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::update_finish\n";
#endif

    int status = DBTREE::article_status( url_article() );
    std::string str_stat;
    if( status & STATUS_OLD ) str_stat = "[ DAT落ち or 移転しました ]";
    if( status & STATUS_BROKEN ) str_stat = "[ 壊れています ]";

    if( ! DBTREE::article_ext_err( url_article() ).empty() ) str_stat += " [ " + DBTREE::article_ext_err( url_article() ) + " ]";

    // ラベルセット
    toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
    toolbar()->set_label( str_stat + DBTREE::article_subject( url_article() ) );

    // タブのラベルセット
    std::string str_label = str_stat + DBTREE::article_subject( url_article() );
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label ); 

    std::ostringstream ss_tmp;
    ss_tmp << DBTREE::article_str_code( url_article() )
           << " [ 全 " << DBTREE::article_number_load( url_article() )
           << " / 新着 " << DBTREE::article_number_new( url_article() ) << " ] "
           << str_stat
           << " " << DBTREE::article_lng_dat( url_article() )/1024 << " k";

    set_status( ss_tmp.str() );
    CORE::core_set_command( "set_status", "", get_status() );
    ARTICLE::get_admin()->set_command( "focus_current_view" );

    // 全体再描画
    drawarea()->redraw_view();

    if( m_gotonum_reserve ) goto_num( m_gotonum_reserve );
    m_gotonum_reserve = 0;
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewMain::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::relayout\n";
#endif

    int seen = drawarea()->seen_current();

    drawarea()->clear_screen();
    drawarea()->append_res( 1, get_article()->get_number_load() );
    drawarea()->goto_num( seen );
    drawarea()->redraw_view();
}




////////////////////////////////////////////////////////////////////////////////////////////////////

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
    set_url( url_tmp );

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
    CORE::core_set_command( "switch_article" );

    show_res( m_str_num, m_show_title );

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( " [ RES:" + m_str_num + " ] - " + DBTREE::article_subject( url_article() ) );
        std::string str_label = "[RES] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );
    }
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

// ID抽出ビュー


ArticleViewID::ArticleViewID( const std::string& url, const std::string& id )
    : ArticleViewBase( url ),
      m_str_id( id )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url_article() + ARTICLE_SIGN + ID_SIGN + m_str_id + TIME_SIGN + MISC::timevaltostr( tv ) );

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
    CORE::core_set_command( "switch_article" );

    show_id( m_str_id );

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( " [ ID:" + m_str_id.substr( strlen( PROTO_ID ) ) + " ] - "
                                     + DBTREE::article_subject( url_article() ));
        std::string str_label = "[ID] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );
    }
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
    show_id( m_str_id );
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

    set_url( url_article() + ARTICLE_SIGN + BOOKMK_SIGN + TIME_SIGN + MISC::timevaltostr( tv ) );

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
    CORE::core_set_command( "switch_article" );

    show_bm();

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( " [ BM ] - " + DBTREE::article_subject( url_article() ));
        std::string str_label = "[BM] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );
    }
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

// URL抽出ビュー

ArticleViewURL::ArticleViewURL( const std::string& url )
    : ArticleViewBase( url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // viewのURL更新
    set_url( url_article() + ARTICLE_SIGN + URL_SIGN + TIME_SIGN + MISC::timevaltostr( tv ) );

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
    CORE::core_set_command( "switch_article" );

    show_res_with_url();

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( " [ URL ] - " + DBTREE::article_subject( url_article() ));
        std::string str_label = "[URL] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );
    }
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

    set_url( url_article() + ARTICLE_SIGN + REFER_SIGN + m_str_num + TIME_SIGN + MISC::timevaltostr( tv ) );

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
    CORE::core_set_command( "switch_article" );

    show_refer( atol( m_str_num.c_str() ) );

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( " [ Re:" + m_str_num + " ] - " + DBTREE::article_subject( url_article() ));
        std::string str_label = "[Re] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );
    }
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
    : ArticleViewBase( url ), m_query( query ), m_mode_or( mode_or )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    //viewのURL更新
    std::string url_tmp = url_article() + ARTICLE_SIGN + KEYWORD_SIGN + m_query + ORMODE_SIGN;
    if( mode_or ) url_tmp += "1";
    else url_tmp += "0";
    url_tmp += TIME_SIGN + MISC::timevaltostr( tv );
    set_url( url_tmp );

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
    CORE::core_set_command( "switch_article" );

    drawout_keywords( m_query, m_mode_or );

    // ラベルとタブ
    if( toolbar() ){

        toolbar()->m_button_board.set_label( "[ " + DBTREE::board_name( url_article() ) + " ]" );
        toolbar()->set_label( DBTREE::article_subject( url_article() ) );

        std::string str_label;
        if( m_mode_or ) str_label = "[OR] " + DBTREE::article_subject( url_article() );
        else str_label = "[AND] " + DBTREE::article_subject( url_article() );
        ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label );

        slot_push_open_search();
        toolbar()->m_entry_search.set_text( m_query );
    }
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
    drawout_keywords( m_query, m_mode_or );
    drawarea()->redraw_view();
}



