// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "settingloader.h"
#include "interface.h"

#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "cache.h"


using namespace DBTREE;

SettingLoader::SettingLoader( const std::string& url_boadbase )
    : SKELETON::TextLoader()
    , m_url_boadbase( url_boadbase )
{
#ifdef _DEBUG
    std::cout << "SettingLoader::SettingLoader : " << m_url_boadbase << std::endl;
#endif

    set_date_modified( DBTREE::board_get_modified_setting( m_url_boadbase ) );
}


SettingLoader::~SettingLoader()
{
#ifdef _DEBUG
    std::cout << "SettingLoader::~SettingLoader : " << m_url_boadbase << std::endl;
#endif
}


std::string SettingLoader::get_url() const
{
    return DBTREE::url_settingtxt( m_url_boadbase );
}


std::string SettingLoader::get_path() const
{
    return CACHE::path_board_root( m_url_boadbase ) + DBTREE::kSettingTxt;
}


// ロード用データ作成
void SettingLoader::create_loaderdata( JDLIB::LOADERDATA& data )
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
void SettingLoader::parse_data()
{
    JDLIB::ConfLoader cf( "", get_data() );

    m_default_noname = cf.get_option_str( "BBS_NONAME_NAME", "No Name" );
    m_line_number = cf.get_option_int( "BBS_LINE_NUMBER", 0, 0, 8192 );
    m_message_count = cf.get_option_int( "BBS_MESSAGE_COUNT", 0, 0, 81920 );
    m_unicode = cf.get_option_str( "BBS_UNICODE", "" );
    const int num_stop = cf.get_option_int( "BBS_THREAD_STOP", 0, 0, CONFIG::get_max_resnumber() );
    if( num_stop ) {
        // 板設定の最大レス数がデフォルト(0:未設定)でないときは BBS_THREAD_STOP の値に上書きする
        const int max_res = DBTREE::board_get_number_max_res( m_url_boadbase );
        if( !max_res && max_res != num_stop ) {
            DBTREE::board_set_number_max_res( m_url_boadbase, num_stop );
        }
    }
    DBTREE::board_set_modified_setting( m_url_boadbase, get_date_modified() );

#ifdef _DEBUG
    std::cout << "SettingLoader::parse url = " << get_url() << std::endl
              << "default_noname = " << m_default_noname << std::endl
              << "line_number = " << m_line_number << std::endl
              << "message_count = " << m_message_count << std::endl
              << "unicode = " << m_unicode << std::endl;
#endif
}


void SettingLoader::receive_cookies()
{
    DBTREE::board_set_list_cookies( m_url_boadbase, SKELETON::Loadable::cookies() );
}
