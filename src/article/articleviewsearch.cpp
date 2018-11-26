// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewsearch.h"
#include "drawareamain.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "config/globalconf.h"

#include "control/controlid.h"

#include "global.h"
#include "session.h"
#include "usrcmdmanager.h"

#include <sstream>

using namespace ARTICLE;

// ログやスレタイ検索抽出ビュー
ArticleViewSearch::ArticleViewSearch( const std::string& url, const bool exec_search )
    : ArticleViewBase( url, "dummy" ),
      m_mode_or( false ),
      m_enable_bm( false ),
      m_bm( false ),
      m_loading( false ),
      m_search_executed( false ),
      m_escaped( false )
{
    set_id_toolbar( TOOLBAR_SEARCH );
    set_writeable( false );

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::ArticleViewSearch " << get_url() << std::endl;
#endif

    setup_view();

    CORE::get_search_manager()->sig_search_fin().connect( sigc::mem_fun( *this, &ArticleViewSearch::slot_search_fin ) );

    m_cancel_reload = ! exec_search;
}



ArticleViewSearch::~ArticleViewSearch()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewSearch::~ArticleViewSearch : " << get_url() << std::endl;
#endif

    stop();
}


//
// url から query などを取得してツールバーの状態をセット
//
void ArticleViewSearch::set_toolbar_from_url()
{
#ifdef _DEBUG
        std::cout << "ArticleViewSearch::set_toolbar_from_url url = " << get_url() << std::endl;
#endif 

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( std::string( "(.*)" ) + BOARD_SIGN + KEYWORD_SIGN + "(.*)" + ORMODE_SIGN + "(.*)" + BOOKMK_SIGN + "(.*)",
                    get_url(), offset, icase, newline, usemigemo, wchar )){

        m_url_board = regex.str( 1 );
        if( m_url_board == URL_SEARCH_ALLBOARD ) m_searchmode = CORE::SEARCHMODE_ALLLOG;
        else m_searchmode = CORE::SEARCHMODE_LOG;

        set_search_query( regex.str( 2 ) );
        if( regex.str( 3 ) == "1" ) m_mode_or = true;
        else m_mode_or = false;
        if( regex.str( 4 ) == "1" ) m_bm = true;
        else m_bm = false;

        m_enable_bm = true;
    }
    else if( regex.exec( std::string( "(.*)" ) + TITLE_SIGN + KEYWORD_SIGN + "(.*)",
                         get_url(), offset, icase, newline, usemigemo, wchar )){

        m_url_board = regex.str( 1 );
        m_searchmode = CORE::SEARCHMODE_TITLE;

        set_search_query( regex.str( 2 ) );
        m_mode_or = false;
        m_bm = false;

        m_enable_bm = false;
    }

    update_label();
}


//
// queryなどを変更した時の新しいURL
//
// queryが変化したときにurlを更新しないと再起動したときのrestoreで
// 古いqueryのままになる
//
const std::string ArticleViewSearch::get_new_url()
{
    std::string url_tmp = m_url_board;

    if( m_searchmode == CORE::SEARCHMODE_TITLE ) url_tmp += TITLE_SIGN;
    else url_tmp += BOARD_SIGN;
    
    url_tmp += KEYWORD_SIGN + get_search_query();

    if( m_searchmode != CORE::SEARCHMODE_TITLE ){

        url_tmp += ORMODE_SIGN;
        if( m_mode_or ) url_tmp += "1";
        else url_tmp += "0";

        url_tmp += BOOKMK_SIGN;
        if( get_bm() ) url_tmp += "1";
        else url_tmp += "0";
    }

#ifdef _DEBUG
    std::cout << "ArticleViewSearch::get_new_url " << url_tmp << std::endl;
#endif

    return url_tmp;
}


//
// ラベルやタブを更新
//
void ArticleViewSearch::update_label()
{
    std::string label;
    if( m_searchmode == CORE::SEARCHMODE_TITLE ) label =  "スレタイ検索";
    else if( m_searchmode == CORE::SEARCHMODE_ALLLOG ) label =  "全ログ検索";
    else label = "ログ検索";
    if( ! get_search_query().empty() ) label += " ： " + get_search_query();
    set_label( label );
    ARTICLE::get_admin()->set_command( "redraw_toolbar" );

    // タブ更新
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), get_label() );
}


//
// 正規表現メタ文字をエスケープ
//
void ArticleViewSearch::regex_escape()
{
    m_escaped = false;
    if( m_searchmode == CORE::SEARCHMODE_LOG || m_searchmode == CORE::SEARCHMODE_ALLLOG ){

        const bool escape = false;  // \ を エスケープ文字として考慮しない
        if( MISC::has_regex_metachar( get_search_query(), escape ) ){

            m_escaped = true;
            set_search_query( MISC::regex_escape( get_search_query(), escape ) );
        }
    }
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
// コマンド
//
bool ArticleViewSearch::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    // URL とツールバーの状態を一致させる
    if( command == "set_toolbar_from_url" ){

        set_toolbar_from_url();
        return true;
    }

    return ArticleViewBase::set_command( command, arg1, arg2 );
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

    set_toolbar_from_url();

    // コンストラクタのパラメータの exec_search が false の時にロードをキャンセル
    if( ! m_cancel_reload ) reload();
    m_cancel_reload = false;

    relayout();
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

    if( m_loading ){
        comment <<  "<br><br>検索中・・・";
        ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
    }
    else{

        if( m_search_executed ){
            if( has_query ) comment << get_search_query() << " ";
            comment << m_list_searchdata.size() << " 件<br>";
            if( m_escaped ) comment << "ログ検索では正規表現は使用出来ません。メタ文字はエスケープされました。<br>";
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

                comment << "<a href=\"" << (*it).url_readcgi << "\">" << MISC::html_escape( (*it).subject ) << "</a>";

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

    m_loading = false;
    m_list_searchdata =  CORE::get_search_manager()->get_list_data();
    relayout();

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

        // url 変更
        const std::string new_url = get_new_url();
        if( new_url != get_url() ){

            // 他のタブで既に開いていたら切り替える
            if( ARTICLE::get_admin()->exist_tab( new_url ) ){

#ifdef _DEBUG
                std::cout << "switch -> " << new_url << std::endl;
#endif

                // 切り替える前に URL とツールバーの状態を合わせておかないと無限ループになる時がある
                ARTICLE::get_admin()->set_command_immediately( "set_toolbar_from_url", new_url );

                ARTICLE::get_admin()->set_command( "switch_view", new_url );
                ARTICLE::get_admin()->set_command( "reload_view", new_url );
                return;
            }

            set_url( new_url );
            update_label();
        }

        regex_escape();
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
                                                MISC::regex_unescape( get_search_query() ), m_mode_or, get_bm(), calc_data );
        }
        
        m_search_executed = true;
        m_loading = true;
        relayout();
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
