// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "ruleloader.h"
#include "interface.h"

#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "cache.h"

#define HEAD_TXT "head.txt"

using namespace DBTREE;

RuleLoader::RuleLoader( const std::string& url_boadbase )
    : SKELETON::TextLoader()
    , m_url_boadbase( url_boadbase )
{
#ifdef _DEBUG
    std::cout << "RuleLoader::RuleLoader : " << RuleLoader::get_url() << std::endl;
#endif

    set_date_modified( DBTREE::board_get_modified_localrule( m_url_boadbase ) );
}


RuleLoader::~RuleLoader()
{
#ifdef _DEBUG
    std::cout << "RuleLoader::~RuleLoader : " << RuleLoader::get_url() << std::endl;
#endif
}


std::string RuleLoader::get_url() const
{
    return m_url_boadbase + HEAD_TXT;
}


std::string RuleLoader::get_path() const
{
    return CACHE::path_board_root( m_url_boadbase ) + HEAD_TXT;
}


// ロード用データ作成
void RuleLoader::create_loaderdata( JDLIB::LOADERDATA& data )
{
    if( !CACHE::mkdir_boardroot( m_url_boadbase ) ) data.url = std::string();
    else{

        // 移転処理
        m_url_boadbase = DBTREE::url_boardbase( m_url_boadbase );

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
}


// ロード後に呼び出される
void RuleLoader::parse_data()
{
    DBTREE::board_set_modified_localrule( m_url_boadbase, get_date_modified() );
}


void RuleLoader::receive_cookies()
{
    DBTREE::board_set_list_cookies( m_url_boadbase, SKELETON::Loadable::cookies() );
}
