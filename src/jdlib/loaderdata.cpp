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

    agent.clear();
    referer.clear();
    ex_field.clear();
            
    str_header.clear();
    code = HTTP_INIT;
    str_code.clear();
    date.clear();
    modified.clear();
    byte_readfrom = 0;
    cookie_for_write.clear();
    list_cookies.clear();
    contenttype.clear();
    basicauth.clear();
    size_buf = 0;
    timeout = 0;
}
