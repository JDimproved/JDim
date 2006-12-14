// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "settingloader.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/confloader.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"
#include "cache.h"

#define SIZE_OF_RAWDATA ( 2 * 1024 * 1024 )

#define SETTING_TXT "SETTING.TXT"


using namespace DBTREE;


SettingLoader::SettingLoader( const std::string& url_boadbase )
    : SKELETON::Loadable()
    , m_loaded( 0 )
    , m_url_boadbase( url_boadbase )
    , m_rawdata( 0 )
    , m_lng_rawdata( 0 )
    , m_line_number( 0 )
    , m_message_count( 0 )
{
#ifdef _DEBUG
    std::cout << "SettingLoader::SettingLoader : " << m_url_boadbase << std::endl;
#endif

    clear();
}



SettingLoader::~SettingLoader()
{
#ifdef _DEBUG
    std::cout << "SettingLoader::~SettingLoader : " << m_url_boadbase << std::endl;
#endif

    clear();
}


void SettingLoader::clear()
{
    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;
    m_lng_rawdata = 0;
}


//
// パース
//
void SettingLoader::parse( const std::string& setting )
{
    JDLIB::ConfLoader cf( "", setting );

    m_default_noname = cf.get_option( "BBS_NONAME_NAME", "No Name" );
    m_line_number = cf.get_option( "BBS_LINE_NUMBER", 0 );
    m_message_count = cf.get_option( "BBS_MESSAGE_COUNT", 0 );
}


//
// キャッシュからSETTING.TXTをロード
//
void SettingLoader::load_setting()
{
#ifdef _DEBUG
    std::cout << "SettingLoader::load_setting " << m_url_boadbase << std::endl;
#endif
    if( is_loading() ) return;
    if( m_loaded ) return; // 読み込み済み

    std::string path = CACHE::path_board_root( m_url_boadbase ) + SETTING_TXT;

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    m_lng_rawdata = CACHE::load_rawdata( path, m_rawdata, SIZE_OF_RAWDATA );

    // UTF-8に変換しておく
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( DBTREE::board_charset( m_url_boadbase ), "UTF-8" );
    int byte_out;
    m_settingtxt = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    delete libiconv;
    clear();

#ifdef _DEBUG
    std::cout << m_settingtxt << std::endl;
#endif

    parse( m_settingtxt );
    set_code( HTTP_OK );
}


//
// SETTING.TXT ダウンロード
//
void SettingLoader::download_setting()
{
#ifdef _DEBUG
    std::cout << "SettingLoader::download_setting " << m_url_boadbase << std::endl;
#endif
    if( is_loading() ) return;
    if( m_loaded ) return; // 読み込み済み

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );

    // オフライン
    if( ! SESSION::is_online() ){

        set_str_code( "" );

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();
        return;
    }

    JDLIB::LOADERDATA data;    
    data.url = m_url_boadbase + SETTING_TXT;
    data.agent = DBTREE::get_agent( m_url_boadbase );
    data.host_proxy = DBTREE::get_proxy_host( m_url_boadbase );
    data.port_proxy = DBTREE::get_proxy_port( m_url_boadbase );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

    if( ! start_load( data ) ){
        clear();
    }
}


//
// ローダよりsubject.txt受信
//
void SettingLoader::receive_data( const char* data, size_t size )
{
    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
    assert( m_lng_rawdata < SIZE_OF_RAWDATA );
}



//
// ロード完了
//
void SettingLoader::receive_finish()
{
    bool read_from_cache = false;

#ifdef _DEBUG
    std::cout << "SettingLoader::receive_finish code = " << get_str_code() << std::endl;
#endif

    std::string path = CACHE::path_board_root( m_url_boadbase ) + SETTING_TXT;

    // 失敗時、オフライン時はキャッシュから読み込み
    if( get_code() != HTTP_OK || !SESSION::is_online() ){

        m_lng_rawdata = CACHE::load_rawdata( path, m_rawdata, SIZE_OF_RAWDATA );
        read_from_cache = true;

#ifdef _DEBUG
        std::cout << "read from " << path << std::endl;
#endif        
    }

    if( m_lng_rawdata == 0 ){
        clear();
        return;
    }

    set_code( HTTP_OK );
    set_str_code( std::string() );
    m_loaded = true;

    // キャッシュに保存
    if( SESSION::is_online() && ! read_from_cache ){

        if( CACHE::mkdir_boardroot( m_url_boadbase ) ){

            CACHE::save_rawdata( path, m_rawdata, m_lng_rawdata );

#ifdef _DEBUG
            std::cout << "save to " << path << std::endl;
#endif
        }
    }

    // UTF-8に変換しておく
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( DBTREE::board_charset( m_url_boadbase ), "UTF-8" );
    int byte_out;
    m_settingtxt = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    delete libiconv;
    clear();

#ifdef _DEBUG
    std::cout << m_settingtxt << std::endl;
#endif
    parse( m_settingtxt );
}
