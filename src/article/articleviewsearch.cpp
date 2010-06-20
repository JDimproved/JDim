// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewsearch.h"
#include "drawareamain.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "control/controlid.h"

#include "global.h"
#include "session.h"
#include "usrcmdmanager.h"

#include <sstream>
#include <sys/time.h>

using namespace ARTICLE;

// ログやスレタイ検索抽出ビュー
ArticleViewSearch::ArticleViewSearch( const std::string& url_board,
                                      const std::string& query, const int searchmode, const bool exec_search, const bool mode_or, const bool bm )
    : ArticleViewBase( url_board )
    , m_url_board( url_board ), m_searchmode( searchmode ), m_mode_or( mode_or ), m_bm( bm ),
      m_loading( false ), m_search_executed( false )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    m_time_str = MISC::timevaltostr( tv );

    set_id_toolbar( TOOLBAR_SEARCH );

    if( m_searchmode == CORE::SEARCHMODE_TITLE ) m_enable_bm = false;
    else m_enable_bm = true;

    set_search_query( query );
    update_url_query( false );
    set_writeable( false );

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::ArticleViewSearch " << get_url() << std::endl;
#endif

    setup_view();

    CORE::get_search_manager()->sig_search_fin().connect( sigc::mem_fun( *this, &ArticleViewSearch::slot_search_fin ) );

    if( exec_search ) reload();
}



ArticleViewSearch::~ArticleViewSearch()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewSearch::~ArticleViewSearch : " << get_url() << std::endl;
#endif

    stop();
}


//
//viewのURL更新
//
// queryが変化したときにurlを更新しないと再起動したときのrestoreで
// 古いqueryのままになる
//
// update_history == true の場合はView履歴も更新する
//
void ArticleViewSearch::update_url_query( const bool update_history )
{
    std::string url_tmp = m_url_board;

    if( m_searchmode == CORE::SEARCHMODE_TITLE ) url_tmp += TITLE_SIGN;
    else url_tmp += BOARD_SIGN;
    
    url_tmp += KEYWORD_SIGN + get_search_query() + ORMODE_SIGN;
    if( m_mode_or ) url_tmp += "1";
    else url_tmp += "0";

    if( m_searchmode != CORE::SEARCHMODE_TITLE ){
        url_tmp += BOOKMK_SIGN;
        if( get_bm() ) url_tmp += "1";
        else url_tmp += "0";
    }

    url_tmp += TIME_SIGN + m_time_str;
    set_url( url_tmp, update_history );

    // ツールバーのラベルも更新しておく
    std::string label;
    if( m_searchmode == CORE::SEARCHMODE_TITLE ) label =  "スレタイ検索";
    else label = "ログ検索";
    if( ! get_search_query().empty() ) label += " ： " + get_search_query();
    set_label( label );
    if( update_history ) ARTICLE::get_admin()->set_command( "redraw_toolbar" );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}


//
// コピー用URL( readcgi型 )
//
// メインウィンドウのURLバーなどの表示用にも使う
//
const std::string ArticleViewSearch::url_for_copy()
{
    if( m_searchmode == CORE::SEARCHMODE_TITLE ) return m_url_title;

    return std::string();
}


//
// フォーカスイン 
//
void ArticleViewSearch::focus_view()
{
    if( ! m_loading && m_list_searchdata.empty() ){

        ARTICLE::get_admin()->set_command( "open_searchbar", get_url() );
    }
    else ArticleViewBase::focus_view();
}


//
// 表示
//
void ArticleViewSearch::show_view()
{
#ifdef _DEBUG
    std::cout << "ArticleViewSearch::show_view()\n";
#endif

    drawarea()->clear_screen();
    drawarea()->clear_highlight();

    std::ostringstream comment;

    if( m_searchmode == CORE::SEARCHMODE_ALLLOG ) comment << "検索対象：キャッシュ内の全ログ<br>";
    else if( m_searchmode == CORE::SEARCHMODE_TITLE ) comment << "検索サイト : "
                                                      + MISC::get_hostname( CONFIG::get_url_search_title() ) + "<br>";
    else comment << "検索対象：" + DBTREE::board_name( m_url_board ) + "<br>";

    if( get_bm() ) comment << "検索条件：しおり<br>";

    append_html( comment.str() );

    if( CORE::get_search_manager()->is_searching( get_url() ) ){
        append_html( "検索中・・・" );
        m_loading = true;
        ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
    }
    else if( ! m_search_executed ){

        append_html( "検索条件を入れて再検索ボタンを押してください。" );
    }
}


//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewSearch::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewSearch::relayout\n";
#endif

    drawarea()->clear_screen();
    drawarea()->clear_highlight();

    std::ostringstream comment;
    const bool has_query = ! get_search_query().empty();

    if( m_searchmode == CORE::SEARCHMODE_ALLLOG ) comment <<  "検索対象：キャッシュ内の全ログ<br>";
    else if( m_searchmode == CORE::SEARCHMODE_TITLE ) comment << "検索サイト : "
                                                + MISC::get_hostname( CONFIG::get_url_search_title() ) + "<br>";
    else comment <<  "検索対象：" << DBTREE::board_name( m_url_board ) << "<br>";

    if( get_bm() ) comment << "検索条件：しおり<br>";

    if( m_search_executed ){
        if( has_query ) comment << get_search_query() << " ";
        comment << m_list_searchdata.size() << " 件<br>";
    }
    else{
        comment << "<br><br>検索条件を入れて再検索ボタンを押してください。";
    }

    comment << "<br>";

    if( ! m_list_searchdata.empty() ){

        std::list< CORE::SEARCHDATA >::iterator it = m_list_searchdata.begin();
        for(; it != m_list_searchdata.end(); ++it ){

            // 板名表示
            if( m_searchmode == CORE::SEARCHMODE_ALLLOG || m_searchmode == CORE::SEARCHMODE_TITLE  )
                comment << "[ <a href=\"" << DBTREE::url_subject( (*it).url_readcgi ) << "\">" << (*it).boardname << "</a> ] ";

            comment << "<a href=\"" << (*it).url_readcgi << "\">" << (*it).subject << "</a>";

            if( (*it).num ) comment << " ( " << (*it).num << " )";

            // queryの抽出表示
            if( m_searchmode != CORE::SEARCHMODE_TITLE && has_query )
                comment << "<br><a href=\"" << PROTO_OR << (*it).url_readcgi + KEYWORD_SIGN + get_search_query() << "\">" << "抽出表示する" << "</a>";

            if( (*it).bookmarked ) comment << "<br>スレにしおりが付けられています";

            if( (*it).num_bookmarked ){
                comment << "<br>レスに付けられたしおり " << (*it).num_bookmarked << "件 <a href=\"" << PROTO_BM << (*it).url_readcgi << "\">" << "抽出表示する" << "</a>";
            }

            comment << "<br><br>";
        }
    }

    append_html( comment.str() );
    drawarea()->redraw_view();
}


//
// 検索終了
//
void ArticleViewSearch::slot_search_fin( const std::string& id )
{
    if( id != get_url() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::slot_search_fin " << get_url() << std::endl;
#endif

    m_list_searchdata =  CORE::get_search_manager()->get_list_data();
    relayout();

    m_loading = false;
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}


//
// 再読み込みボタンを押した
//
void ArticleViewSearch::reload()
{
    exec_reload();
}


//
// 再読み込み実行
//
void ArticleViewSearch::exec_reload()
{
    if( CORE::get_search_manager()->is_searching() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "他の検索スレッドが実行中です" );
        mdiag.run();
        return;
    }

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::exec_reload\n";
#endif

    // 検索が終わると ArticleViewSearch::slot_search_fin() が呼ばれる
    if( ! get_search_query().empty() || get_bm() ){

        update_url_query( true );
        const std::string id = get_url();

#ifdef _DEBUG
        std::cout << "id = " << id << std::endl;
#endif

        if( m_searchmode == CORE::SEARCHMODE_TITLE ){

            if( ! SESSION::is_online() ){
                SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
                mdiag.run();
                return;
            }

            CORE::get_search_manager()->search_title( id, get_search_query() );
            m_url_title = CORE::get_usrcmd_manager()->replace_cmd( CONFIG::get_url_search_title(), "", "", get_search_query(), 0 );
        }
        else{

            const bool calc_data = true;
            CORE::get_search_manager()->search( id, m_searchmode, m_url_board,
                                                get_search_query(), m_mode_or, get_bm(), calc_data );
        }
        
        m_search_executed = true;
        show_view();
        focus_view();
    }
}


//
// 検索停止
//
void ArticleViewSearch::stop()
{
    CORE::get_search_manager()->stop( get_url() );
}


//
// 検索entryでenterを押した
//
void ArticleViewSearch::exec_search()
{
    reload();
}

//
// 検索entryの操作
//
void ArticleViewSearch::operate_search( const std::string& controlid )
{
    const int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ){
        focus_view();
        ARTICLE::get_admin()->set_command( "close_searchbar" );
    }
    else if( id == CONTROL::DrawOutAnd ) reload();
}
