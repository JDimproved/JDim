// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "searchmanager.h"
#include "searchloader.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "config/globalconf.h"

CORE::Search_Manager* instance_search_manager = NULL;

CORE::Search_Manager* CORE::get_search_manager()
{
    if( ! instance_search_manager ) instance_search_manager = new Search_Manager();
    assert( instance_search_manager );

    return instance_search_manager;
}


void CORE::delete_search_manager()
{
    if( instance_search_manager ) delete instance_search_manager;
    instance_search_manager = NULL;
}

///////////////////////////////////////////////

using namespace CORE;

Search_Manager::Search_Manager()
    : m_searching( false ),
      m_searchloader( NULL )
{}


Search_Manager::~Search_Manager()
{
    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop( std::string() );
    wait();

    if( m_searchloader ) delete m_searchloader;
}


//
// ログ検索
//
// 結果は m_list_article と m_list_data に入る。
//
// id : 呼び出し元の ID。 検索終了時に SIG_SEARCH_FIN シグナルで送る
// searchmode : 検索モード
// url: ログ検索先の板のアドレス
// query : 検索文字列、空文字ならキャッシュにあるスレを全て選択
// mode_or : false なら AND、true なら OR で検索する
// bm  : trueの時、しおりが付いている(スレ一覧でしおりを付けた or レスに一つでもしおりが付いている)スレのみを対象に検索する
// calc_data : 検索終了時に m_list_data を求める
//
bool Search_Manager::search( const std::string& id, const int searchmode, const std::string& url,
                             const std::string& query, const bool mode_or, const bool bm, const bool calc_data )
{
#ifdef _DEBUG
    std::cout << "Search_Manager::search url = " << url << " query = " << query << std::endl;
#endif

    if( m_searching ) return false;
    if( m_thread.is_running() ) return false;

    m_searchmode = searchmode;

    m_id = id;
    m_url = url;
    m_query = query;
    m_mode_or = mode_or;
    m_bm = bm;
    m_calc_data = calc_data;

    m_list_article.clear();
    m_list_data.clear();

    // 全ログ検索のとき、スレッドを起動する前にメインスレッドで板情報ファイルを
    // 読み込んでおかないと大量の warning が出る
    if( m_searchmode == SEARCHMODE_ALLLOG ) DBTREE::read_boardinfo_all();

    if( ! m_thread.create( ( STARTFUNC ) launcher, ( void * ) this, JDLIB::NODETACH ) ){
        MISC::ERRMSG( "Search_Manager::search : could not start thread" );
        return FALSE;
    }

    m_searching = true;
    return true;
}


//
// スレッドのランチャ (static)
//
void* Search_Manager::launcher( void* dat )
{
    Search_Manager* sm = ( Search_Manager * ) dat;
    sm->thread_search();
    return 0;
}


//
// ログ検索実行スレッド
//
void Search_Manager::thread_search()
{
#ifdef _DEBUG
    std::cout << "Search_Manager::thread_search\n";
#endif

    m_stop = false;

    if( m_searchmode == SEARCHMODE_LOG ) DBTREE::search_cache( m_url, m_list_article, m_query, m_mode_or, m_bm,  m_stop );
    else if( m_searchmode == SEARCHMODE_ALLLOG ) DBTREE::search_cache_all( m_list_article, m_query, m_mode_or, m_bm, m_stop );

    if( m_calc_data ){

        std::vector< DBTREE::ArticleBase* >::iterator it = m_list_article.begin();
        for( ; it != m_list_article.end(); ++it ){

            DBTREE::ArticleBase* article = ( *it );

            SEARCHDATA data;
            data.url_readcgi = DBTREE::url_readcgi( article->get_url(), 0, 0 );
            data.subject = article->get_subject();
            data.num = article->get_number_load();
            data.bookmarked = article->is_bookmarked_thread();
            data.num_bookmarked = article->get_num_bookmark();
            data.boardname = DBTREE::board_name( data.url_readcgi );

#ifdef _DEBUG
            std::cout << "url = " << data.url_readcgi << std::endl
                      << "board = " << data.boardname << std::endl
                      << "subject = " << data.subject << std::endl
                      << "num = " << data.num << std::endl << std::endl;
#endif
            m_list_data.push_back( data );
        }

    }

    dispatch();
}


//
// ディスパッチャのコールバック関数
//
void Search_Manager::callback_dispatch()
{
    wait();
    search_fin();
}


//
// 検索終了
//
void Search_Manager::search_fin()
{
#ifdef _DEBUG
    std::cout << "Search_Manager::search_fin\n";
#endif

    m_sig_search_fin.emit( m_id );
    m_searching = false;
}


//
// 検索中止
//
void Search_Manager::stop( const std::string& id )
{
    if( ! m_searching ) return;
    if( ! id.empty() && id != m_id ) return;

#ifdef _DEBUG
    std::cout << "Search_Manager::stop\n";
#endif

    m_stop = true;

    // スレタイ検索停止
    if( m_searchmode == SEARCHMODE_TITLE && m_searchloader ) m_searchloader->stop_load();
}


void Search_Manager::wait()
{
    m_thread.join();
}


/////////////////////////////////////////////////////////////////////////


//
// スレタイ検索
//
bool Search_Manager::search_title( const std::string& id, const std::string& query )
{
    if( m_searching ) return false;
    if( m_searchloader && m_searchloader->is_loading() ) return false;

#ifdef _DEBUG
    std::cout << "Search_Manager::search_title query = " << query << std::endl;
#endif

    m_searchmode = SEARCHMODE_TITLE;

    m_id = id;
    m_query = query;

    m_list_data.clear();

    // スレタイの検索が終わったら search_fin_title が呼び出される
    if( ! m_searchloader ){
        m_searchloader = new SearchLoader();
        m_searchloader->sig_search_fin().connect( sigc::mem_fun( *this, &Search_Manager::search_fin_title ) );
    }

    m_searching = true;
    m_searchloader->search( m_query );

    return true;
}


//
// スレタイ検索ロード終了
//
void Search_Manager::search_fin_title()
{
#ifdef _DEBUG
    std::cout << "Search_Manager::search_fin_title\n";
#endif

    if( ! m_searchloader ) return;

    // 行ごとに分割し、regexを使ってurl取得
    if( ! m_searchloader->get_data().empty() ){

        JDLIB::Regex regex;
        size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;
        regex.compile( CONFIG::get_regex_search_title(), icase, newline, usemigemo, wchar );

        std::list< std::string > lines = MISC::get_lines( m_searchloader->get_data() );
        std::list< std::string >::iterator it;
        for( it = lines.begin(); it != lines.end(); ++it ){

            std::string line = MISC::remove_space( *it );

            if( line.empty() ) continue;

            // & が &amp; に置き換わっているので直す
            if( line.find( "&" ) != std::string::npos ) line = MISC::replace_str( line, "&amp;", "&" );

            offset = 0;
            while( regex.exec( line, offset ) ){

                SEARCHDATA data;
                data.url_readcgi = DBTREE::url_readcgi( regex.str( 1 ), 0, 0 );
                data.subject = MISC::html_unescape( regex.str( 2 ) );
                data.num = atoi( regex.str( 3 ).c_str() );
                data.bookmarked = false;
                data.num_bookmarked = 0;

                if( ! data.url_readcgi.empty() ){

                    data.boardname = DBTREE::board_name( data.url_readcgi );

#ifdef _DEBUG
                    std::cout << "url = " << data.url_readcgi << std::endl
                              << "board = " << data.boardname << std::endl
                              << "subject = " << data.subject << std::endl
                              << "num = " << data.num << std::endl << std::endl;
#endif

                    m_list_data.push_back( data );
                }

                // 1行に複数の検索結果が返ってくる場合があるので、オフセットを設定して再検索する
                offset = regex.pos( 0 ) + regex.str( 0 ).length();
            }
        }
    }

    m_searchloader->reset();

    search_fin();
}
