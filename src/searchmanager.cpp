// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "searchmanager.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscthread.h"

#include "dbtree/interface.h"

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
    : m_searching( false )
{}

Search_Manager::~Search_Manager()
{
    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    stop();
}


bool Search_Manager::search( const std::string& id,
                             const std::string& url, const std::string& query,
                             bool mode_or, bool searchall )
{
#ifdef _DEBUG
    std::cout << "Search_Manager::search " << query << std::endl;
#endif

    if( m_searching ) return false;

    m_id = id;
    m_url = url;
    m_query = query;
    m_mode_or = mode_or;
    m_searchall = searchall;

    // 全ログ検索のとき、スレッドを起動する前にメインスレッドで板情報ファイルを
    // 読み込んでおかないと大量の warning が出る
    if( m_searchall ) DBTREE::read_boardinfo_all();

    const int stacksize = 8;
    int status;
    if( ( status = MISC::thread_create( &m_thread, ( STARTFUNC ) launcher, ( void * ) this, stacksize ) )){
        MISC::ERRMSG( std::string( "Search_Manager::search : could not start thread " ) + strerror( status ) );
    }
    else{
        m_searching = true;
        pthread_detach( m_thread );
    }

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
// 検索実行スレッド
//
void Search_Manager::thread_search()
{
#ifdef _DEBUG
    std::cout << "Search_Manager::thread_search\n";
#endif

    m_stop = false;

    if( m_searchall ) m_urllist = DBTREE::search_cache_all( m_url, m_query, m_mode_or, m_stop );
    else m_urllist = DBTREE::search_cache( m_url, m_query, m_mode_or, m_stop );

    dispatch();
}


//
// ディスパッチャのコールバック関数
//
void Search_Manager::callback_dispatch()
{
    search_fin();
}


//
// 検索終了
//
void Search_Manager::search_fin()
{
    m_sig_search_fin.emit();
    m_searching = false;
}


//
// 検索中止
//
void Search_Manager::stop()
{
    if( ! m_searching ) return;

    m_stop = true;
}
