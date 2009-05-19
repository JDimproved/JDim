// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "textloader.h"

#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"

#include "httpcode.h"
#include "session.h"
#include "cache.h"

#include <cstring>

enum
{
    SIZE_OF_RAWDATA = 1024 * 1024
};

using namespace SKELETON;


TextLoader::TextLoader()
    : SKELETON::Loadable(),
      m_loaded( false ),
      m_rawdata( NULL ),
      m_lng_rawdata( 0 )
{
#ifdef _DEBUG
    std::cout << "TextLoader::TextLoader\n";
#endif
}


TextLoader::~TextLoader()
{
#ifdef _DEBUG
    std::cout << "TextLoader::~TextLoader\n";
#endif
    clear();
}


void TextLoader::init()
{
    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
}


void TextLoader::clear()
{
    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;
    m_lng_rawdata = 0;
}


void TextLoader::reset()
{
    m_loaded = false;
    m_data = std::string();
    clear();
}


//
// キャッシュからロード
//
void TextLoader::load_text()
{
    if( get_path().empty() ) return;

    init();
    set_code( HTTP_INIT );
    receive_finish();
}


//
// ダウンロード開始
//
void TextLoader::download_text()
{
#ifdef _DEBUG
    std::cout << "TextLoader::download_text url = " << get_url() << std::endl;
#endif
    if( get_url().empty() ) return;
    if( is_loading() ) return;
    if( m_loaded ) return; // 読み込み済み
    if( ! SESSION::is_online() ){
        load_text();
        return;
    }

#ifdef _DEBUG
    std::cout << "start loading...\n";
#endif

    JDLIB::LOADERDATA data;

    init();
    create_loaderdata( data );
    if( data.url.empty() ) return;
    if( ! start_load( data ) ) clear();
}


//
// ローダよりデータ受信
//
void TextLoader::receive_data( const char* data, size_t size )
{
    if( m_lng_rawdata + size < SIZE_OF_RAWDATA ){
        memcpy( m_rawdata + m_lng_rawdata , data, size );
        m_lng_rawdata += size;
    }
    else{
        MISC::ERRMSG( "TextLoader : received failed ( BOF )\n" );
    }
}



//
// ロード完了
//
void TextLoader::receive_finish()
{
#ifdef _DEBUG
    std::cout << "TextLoader::receive_finish code = " << get_str_code() << std::endl
              << "lng = " << m_lng_rawdata << " modified = " << get_date_modified() << std::endl;
#endif

    // 初期化時やnot modifiedの時はキャッシュから読み込み
    if( ! get_path().empty() && ( get_code() == HTTP_INIT || get_code() == HTTP_NOT_MODIFIED ) ){

        m_lng_rawdata = CACHE::load_rawdata( get_path(), m_rawdata, SIZE_OF_RAWDATA );

#ifdef _DEBUG
        std::cout << "read from " << get_path() << std::endl;
        if( ! m_lng_rawdata ) std::cout << "no data in cache!\n";
#endif        

        if( ! m_lng_rawdata && get_code() == HTTP_INIT ) set_date_modified( std::string() );
    }

    // 読み込みエラー
    if( get_code() != HTTP_OK && get_code() != HTTP_INIT && get_code() != HTTP_NOT_MODIFIED ){

        if( get_code() == HTTP_NOT_FOUND ) m_loaded = true;
        clear();
        set_date_modified( std::string() );
        parse_data();
        return;
    }

    // キャッシュに保存
    if( ! get_path().empty() && get_code() == HTTP_OK && m_lng_rawdata ){

        CACHE::save_rawdata( get_path(), m_rawdata, m_lng_rawdata );

#ifdef _DEBUG
        std::cout << "save to " << get_path() << std::endl;
#endif
    }

    if( get_code() != HTTP_INIT ) m_loaded = true;
    set_code( HTTP_OK );
    set_str_code( std::string() );

    // UTF-8に変換しておく
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( get_charset(), "UTF-8" );
    int byte_out;
    m_data = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );
    delete libiconv;
    clear();

#ifdef _DEBUG
//    std::cout << m_data << std::endl;
#endif

    parse_data();
}
