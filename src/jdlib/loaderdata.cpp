// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loader.h"

#include "config/globalconf.h"

#include "httpcode.h"

using namespace JDLIB;


LOADERDATA::LOADERDATA()
{
    init();
}


void LOADERDATA::init()
{
    head = false;
    length = 0;
    length_current = 0;
    size_data = 0;
            
    url.clear();
    protocol.clear();
    host.clear();
    path.clear();
    port = 0;
    use_ssl = false;
    async = true;
    use_ipv6 = CONFIG::get_use_ipv6();
            
    str_post.clear();

    host_proxy.clear();
    port_proxy = 0;
    basicauth_proxy.clear();

    agent.clear();
    origin.clear();
    referer.clear();
    ex_field.clear();
            
    str_header.clear();
    code = HTTP_INIT;
    str_code.clear();
    date.clear();
    modified.clear();
    byte_readfrom = 0;
    cookie_for_request.clear();
    list_cookies.clear();
    contenttype.clear();
    basicauth.clear();
    size_buf = 0;
    timeout = 0;
}



// 一般データのダウンロード用初期化
void LOADERDATA::init_for_data()
{
    agent = CONFIG::get_agent_for_data();
    if( CONFIG::get_use_proxy_for_data() ) host_proxy = CONFIG::get_proxy_for_data();
    else host_proxy = std::string();
    port_proxy = CONFIG::get_proxy_port_for_data();
    basicauth_proxy = CONFIG::get_proxy_basicauth_for_data();
    size_buf = CONFIG::get_loader_bufsize();
    timeout = CONFIG::get_loader_timeout_data();
}
