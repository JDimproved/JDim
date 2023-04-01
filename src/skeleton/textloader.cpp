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


namespace SKELETON::tl {
constexpr std::size_t kSizeOfRawData = 256 * 1024;
}


using namespace SKELETON;


TextLoader::TextLoader()
    : SKELETON::Loadable()
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
    if( m_rawdata.capacity() < tl::kSizeOfRawData ) {
        m_rawdata.reserve( tl::kSizeOfRawData );
    }
}


void TextLoader::clear()
{
    m_rawdata.clear();
    m_rawdata.shrink_to_fit();
}


void TextLoader::reset()
{
    m_loaded = false;
    m_data = std::string();
    clear();
}


/** @brief キャッシュからロード
 *
 * @details 読み込んだキャッシュはUTF-8に変換する。
 * @param[in] encoding キャッシュの文字エンコーディング
 */
void TextLoader::load_text( const Encoding encoding )
{
    if( get_path().empty() ) return;

    init();
    set_code( HTTP_INIT );
    set_encoding( encoding );
    receive_finish();
}


/** @brief ダウンロードを開始する
 *
 * @details ダウンロードしたテキストはUTF-8に変換する。
 * HTTP 304 Not Modified の時はキャッシュから読み込む。
 * @param[in] encoding ダウンロードしたテキストの文字エンコーディング
 */
void TextLoader::download_text( const Encoding encoding )
{
#ifdef _DEBUG
    std::cout << "TextLoader::download_text url = " << get_url() << std::endl;
#endif
    if( get_url().empty() ) return;
    if( is_loading() ) return;
    if( m_loaded ) return; // 読み込み済み
    if( ! SESSION::is_online() ){
        load_text( encoding );
        return;
    }

#ifdef _DEBUG
    std::cout << "start loading...\n";
#endif

    JDLIB::LOADERDATA data;

    init();
    set_encoding( encoding );
    create_loaderdata( data );
    if( data.url.empty() ) return;
    if( ! start_load( data ) ) clear();
}


//
// ローダよりデータ受信
//
void TextLoader::receive_data( std::string_view buf )
{
    if( m_rawdata.size() + buf.size() < tl::kSizeOfRawData ){
        m_rawdata.append( buf );
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
              << "lng = " << m_rawdata.size() << " modified = " << get_date_modified() << std::endl;
#endif

    // 初期化時やnot modifiedの時はキャッシュから読み込み
    if( ! get_path().empty() && ( get_code() == HTTP_INIT || get_code() == HTTP_NOT_MODIFIED ) ){

        m_rawdata.resize( tl::kSizeOfRawData );
        const std::size_t read_size = CACHE::load_rawdata( get_path(), m_rawdata.data(), tl::kSizeOfRawData );
        m_rawdata.resize( read_size );

#ifdef _DEBUG
        std::cout << "read from " << get_path() << std::endl;
        if( ! read_size ) std::cout << "no data in cache!\n";
#endif        

        if( ! read_size && get_code() == HTTP_INIT ) set_date_modified( std::string() );
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
    if( ! get_path().empty() && get_code() == HTTP_OK && ! m_rawdata.empty() ){

        CACHE::save_rawdata( get_path(), m_rawdata );

#ifdef _DEBUG
        std::cout << "save to " << get_path() << std::endl;
#endif
    }

    if( get_code() != HTTP_INIT ) m_loaded = true;
    set_code( HTTP_OK );
    set_str_code( std::string() );

    // UTF-8に変換しておく
    JDLIB::Iconv libiconv( Encoding::utf8, get_encoding() );
    libiconv.convert( m_rawdata.data(), m_rawdata.size(), m_data );
    clear();

    receive_cookies();

    parse_data();
}
