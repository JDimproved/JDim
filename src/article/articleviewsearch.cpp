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

#include "global.h"
#include "session.h"
#include "controlid.h"

#include <sstream>
#include <sys/time.h>

using namespace ARTICLE;

// ログやスレタイ検索抽出ビュー
ArticleViewSearch::ArticleViewSearch( const std::string& url_board,
                                      const std::string& query, const int searchmode, const bool exec_search, const bool mode_or )
    : ArticleViewBase( url_board )
    , m_url_board( url_board ), m_searchmode( searchmode ), m_mode_or( mode_or ), 
      m_loading( false ), m_search_executed( false )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    m_time_str = MISC::timevaltostr( tv );

    set_id_toolbar( TOOLBAR_SEARCH );

    set_search_query( query );
    update_url_query( false );

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
    if( m_searchmode == SEARCHMODE_TITLE ) url_tmp += TITLE_SIGN;
    else url_tmp += BOARD_SIGN;
    url_tmp += KEYWORD_SIGN + get_search_query() + ORMODE_SIGN;
    if( m_mode_or ) url_tmp += "1";
    else url_tmp += "0";
    url_tmp += TIME_SIGN + m_time_str;
    set_url( url_tmp, update_history );

    // ツールバーのラベルも更新しておく
    std::string label;
    if( m_searchmode == SEARCHMODE_TITLE ) label =  "スレタイ検索";
    else label = "ログ検索";
    if( ! get_search_query().empty() ) label += " ： " + get_search_query();
    set_label( label );
    if( update_history ) ARTICLE::get_admin()->set_command( "redraw_toolbar" );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}


//
// フォーカスイン 
//
void ArticleViewSearch::focus_view()
{
    if( ! m_loading && m_list_searchdata.empty() ) ARTICLE::get_admin()->set_command( "open_searchbar", get_url() );
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

    if( m_searchmode == SEARCHMODE_ALLLOG ) append_html( "検索対象：キャッシュ内の全ログ<br>" );
    else if( m_searchmode == SEARCHMODE_TITLE ) append_html( "検索サイト : "
                                                             + MISC::get_hostname( CONFIG::get_url_search_title() ) + "<br>" );
    else append_html( "検索対象：" + DBTREE::board_name( m_url_board ) + "<br>" );

    if( CORE::get_search_manager()->get_id() == get_url() ){

        if( CORE::get_search_manager()->is_searching() ){
            append_html( "検索中・・・" );
            m_loading = true;
            ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
        }
    }
    else if( ! m_search_executed ) append_html( "検索語を入れて再検索ボタンを押してください。" );
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

    if( m_searchmode == SEARCHMODE_ALLLOG ) comment <<  "検索対象：キャッシュ内の全ログ<br>";
    else if( m_searchmode == SEARCHMODE_TITLE ) comment << "検索サイト : "
                                                + MISC::get_hostname( CONFIG::get_url_search_title() ) + "<br>";
    else comment <<  "検索対象：" << DBTREE::board_name( m_url_board ) << "<br>";

    if( m_search_executed ) comment << get_search_query() << " " << m_list_searchdata.size() << " 件<br>";
    else comment << "<br><br>検索語を入れて再検索ボタンを押してください。";

    comment << "<br>";

    if( ! m_list_searchdata.empty() ){

        std::list< CORE::SEARCHDATA >::iterator it = m_list_searchdata.begin();
        for(; it != m_list_searchdata.end(); ++it ){

            // 板名表示
            if( m_searchmode == SEARCHMODE_ALLLOG || m_searchmode == SEARCHMODE_TITLE )
                comment << "[ <a href=\"" << DBTREE::url_subject( (*it).url_readcgi ) << "\">" << (*it).boardname << "</a> ] ";

            comment << "<a href=\"" << (*it).url_readcgi << "\">" << (*it).subject << "</a>";

            if( (*it).num ) comment << " ( " << (*it).num << " )";

            // queryの抽出表示
            if( m_searchmode == SEARCHMODE_ALLLOG || m_searchmode == SEARCHMODE_LOG )
                comment << "<br><a href=\"" << PROTO_OR << (*it).url_readcgi + KEYWORD_SIGN + get_search_query() << "\">" << "抽出表示する" << "</a>";

            comment << "<br><br>";
        }
    }

    append_html( comment.str() );
    drawarea()->redraw_view();
}


//
// 検索終了
//
void ArticleViewSearch::slot_search_fin()
{
    if( CORE::get_search_manager()->get_id() != get_url() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::slot_search_fin " << get_url() << std::endl;
#endif

    m_list_searchdata =  CORE::get_search_manager()->get_list_data();
    relayout();

    m_loading = false;
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
}


//
// 再検索
//
void ArticleViewSearch::reload()
{
    if( CORE::get_search_manager()->is_searching() ){
        SKELETON::MsgDiag mdiag( NULL, "他の検索スレッドが実行中です" );
        mdiag.run();
        return;
    }

    // 検索が終わると ArticleViewSearch::slot_search_fin() が呼ばれる
    if( ! get_search_query().empty() ){

        update_url_query( true );

        if( m_searchmode == SEARCHMODE_TITLE ){

            if( ! SESSION::is_online() ){
                SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
                mdiag.run();
                return;
            }

            CORE::get_search_manager()->search_title( get_url(), get_search_query() );
        }
        else CORE::get_search_manager()->search( get_url(), m_url_board, get_search_query(), m_mode_or, ( m_searchmode == SEARCHMODE_ALLLOG ) );
        
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
    if( CORE::get_search_manager()->is_searching()
        && CORE::get_search_manager()->get_id() == get_url() ) CORE::get_search_manager()->stop();
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
    int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ){
        focus_view();
        ARTICLE::get_admin()->set_command( "close_searchbar" );
    }
    else if( id == CONTROL::DrawOutAnd ) reload();
}
