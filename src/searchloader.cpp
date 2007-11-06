// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "searchloader.h"
#include "usrcmdmanager.h"

#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"


using namespace CORE;

SearchLoader::SearchLoader()
    : SKELETON::TextLoader()
{
    std::string url = CONFIG::get_url_search_title();

    m_charset = "UTF-8";
    if( url.find( "$TEXTX" ) != std::string::npos ) m_charset = "EUC-JP";
    else if( url.find( "$TEXTE" ) != std::string::npos ) m_charset = "MS932";

#ifdef _DEBUG
    std::cout << "SearchLoader::SearchLoader charset = " << m_charset << std::endl;;
#endif
}


SearchLoader::~SearchLoader()
{
#ifdef _DEBUG
    std::cout << "SearchLoader::~SearchLoader\n";
#endif
}


const std::string SearchLoader::get_url()
{
    std::string url = get_usrcmd_manager()->replace_cmd( CONFIG::get_url_search_title(), "", "", m_query );

#ifdef _DEBUG
    std::cout << "SearchLoader::get_url url = " << url << std::endl;
#endif

    return url;
}

void SearchLoader::search( const std::string& query )
{
#ifdef _DEBUG
    std::cout << "SearchLoader::search query = " << query << std::endl;
#endif

    m_query = query;

    reset();
    download_text();
}


// ロード用データ作成
void SearchLoader::create_loaderdata( JDLIB::LOADERDATA& data )
{
#ifdef _DEBUG
    std::cout << "SearchLoader::create_loaderdata\n";
#endif

    data.url = get_url();
    data.agent = CONFIG::get_agent_for_data();
    if( CONFIG::get_use_proxy_for_data() ) data.host_proxy = CONFIG::get_proxy_for_data();
    else data.host_proxy = std::string();
    data.port_proxy = CONFIG::get_proxy_port_for_data();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout_img();
}


// ロード後に呼び出される
void SearchLoader::parse_data()
{
#ifdef _DEBUG
    std::cout << "SearchLoader::parse_data\n";
#endif

    m_sig_search_fin.emit();
}
