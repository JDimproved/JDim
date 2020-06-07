// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "frontloader.h"
#include "interface.h"

#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "cache.h"


using namespace DBTREE;


FrontLoader::FrontLoader( const std::string& url_boadbase )
    : SKELETON::TextLoader()
    , m_url_boadbase( url_boadbase )
{
}


std::string FrontLoader::get_charset()
{
    return DBTREE::board_charset( m_url_boadbase );
}


// ロード用データ作成
void FrontLoader::create_loaderdata( JDLIB::LOADERDATA& data )
{
    // 移転処理
    std::string url_boardbase = DBTREE::url_boardbase( m_url_boadbase );
    if( m_url_boadbase != url_boardbase ) m_url_boadbase = url_boardbase;

    data.url = get_url();
    data.agent = DBTREE::get_agent( m_url_boadbase );
    data.host_proxy = DBTREE::get_proxy_host( m_url_boadbase );
    data.port_proxy = DBTREE::get_proxy_port( m_url_boadbase );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( m_url_boadbase );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
    data.basicauth = DBTREE::board_basicauth( m_url_boadbase );
    data.cookie_for_request = DBTREE::board_cookie_for_request( m_url_boadbase );
}


// ロード後に呼び出される
void FrontLoader::parse_data()
{
    // フロントページからキーワードを解析して登録する
    if( ! get_data().empty() ) {
        DBTREE::board_analyze_keyword_for_newarticle( m_url_boadbase, get_data() );
    }
}


void FrontLoader::receive_cookies()
{
    DBTREE::board_set_list_cookies( m_url_boadbase, SKELETON::Loadable::cookies() );
}
