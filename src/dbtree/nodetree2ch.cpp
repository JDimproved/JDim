// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"
#include "login2ch.h"

#include <sstream>

using namespace DBTREE;


NodeTree2ch::NodeTree2ch( const std::string& url, const std::string& org_url, const std::string& date_modified )
    : NodeTree2chCompati( url, date_modified )
    , m_org_url( org_url ), m_use_offlaw( false )
{
#ifdef _DEBUG
    std::cout << "NodeTree2ch::NodeTree2ch url = " << url << std::endl
              << "org_url = " << m_org_url << " modified = " << date_modified << std::endl;
#endif
}



NodeTree2ch::~NodeTree2ch()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::~NodeTree2ch : " << get_url() << std::endl;
#endif
}


//
// ロード用データ作成
//
void NodeTree2ch::create_loaderdata( JDLIB::LOADERDATA& data )
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::create_loaderdata : " << get_url() << std::endl;
#endif

    data.url = std::string();

    // dat 落ちしていたらofflaw経由でアクセス
    if( m_use_offlaw ){

        JDLIB::Regex regex;
        if( ! regex.exec( "(http://[^/]*)(/.*)/dat(/.*)\\.dat$", m_org_url ) ) return;

        std::ostringstream ss;
        ss << regex.str( 1 ) << "/test/offlaw.cgi" << regex.str( 2 ) << regex.str( 3 )
           << "/?raw=." << get_lng_dat();

        std::string sid = LOGIN::get_login2ch()->get_sessionid();
        ss << "&sid=" << MISC::url_encode( sid.c_str(), sid.length() );

        // レジュームは無し
        set_resume( false );
        data.byte_readfrom = 0;

        data.url = ss.str();

#ifdef _DEBUG    
        std::cout << "use offlaw url = " << data.url << std::endl;
#endif
    }

    // 普通の読み込み
    else{

        data.url = get_url();

        // レジューム設定
        // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
        if( get_lng_dat() ) {
            data.byte_readfrom = get_lng_dat() -1;
            set_resume( true );
        }
        else set_resume( false );
    }

    data.agent = DBTREE::get_agent( get_url() );

    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

    if( ! date_modified().empty() ) data.modified = date_modified();
}


//
// ロード完了
//
void NodeTree2ch::receive_finish()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::receive_finish : " << get_url() << std::endl;
    std::cout << "code = " << get_code() << std::endl;
#endif

    // オンライン、ログインしている、かつdat落ちの場合はofflaw.cgi経由で旧URLで再取得
    if( LOGIN::get_login2ch()->login_now() && SESSION::is_online() && ! m_use_offlaw
        && ( get_code() == HTTP_REDIRECT || get_code() == HTTP_NOT_FOUND )
        ){
#ifdef _DEBUG    
    std::cout << "reload by offlaw\n";
#endif
        m_use_offlaw = true;
        download_dat();
        return;
    }

    NodeTreeBase::receive_finish();
}
