// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loadable.h"

#include "jdlib/loader.h"
#include "jdlib/misccharcode.h"
#include "jdlib/misctime.h"
#include "jdlib/miscutil.h"

#include "httpcode.h"

using namespace SKELETON;


/** @brief HTTPやHTMLからテキストの文字エンコーディングを検出する処理の状態 */
enum class Loadable::CharsetDetection
{
    parse_header, ///< HTTP header を解析する
    parse_body,   ///< HTML body 要素を解析する
    finished,     ///< 検出を終えた
};


Loadable::Loadable()
    : m_charset_det{ CharsetDetection::parse_header }
    , m_encoding{ Encoding::unknown }
{
    clear_load_data();
}


Loadable::~Loadable()
{
    terminate_load(); // 一応デストラクタ内部でも実行しておく
}


// ロード強制停止
void Loadable::terminate_load()
{
#ifdef _DEBUG
    std::cout << "Loadable::terminate_load\n";
#endif

    // loadableを delete する前に terminate_load() を呼び出さないと
    // スレッド実行中にメモリなどが初期化されてしまうため落ちる時がある
    // デストラクタ内から terminate_load() しても落ちる時があるので
    // デストラクタの外から呼び出すこと
    set_dispatchable( false );

    m_loader.reset();
}


// 完全クリア
void Loadable::clear_load_data()
{
    m_charset_det = CharsetDetection::parse_header;
    m_code = HTTP_INIT;
    m_str_code = std::string();
    m_date_modified = std::string();
    m_cookies.clear();
    m_location = std::string();
    m_total_length = 0;
    m_current_length = 0;
}



bool Loadable::is_loading() const
{
    // m_loader != nullptr ならローダ起動中ってこと
    return static_cast<bool>( m_loader );
}


//
// 更新時刻
//
time_t Loadable::get_time_modified()
{
    time_t time_out;
    time_out = MISC::datetotime( m_date_modified );
    if( time_out == 0 ) time_out = time( nullptr ) - 600;

#ifdef _DEBUG
    std::cout << "Loadable::get_time_modified " << m_date_modified << std::endl
              << " -> " << time_out << std::endl;
#endif

    return time_out; 
}


//
// ロード開始
//
bool Loadable::start_load( const JDLIB::LOADERDATA& data )
{
    if( is_loading() ) return false;

#ifdef _DEBUG
    std::cout << "Loadable::start_load url = " << data.url << std::endl;
#endif

    assert( m_loader == nullptr );
    m_loader = std::make_unique<JDLIB::Loader>( m_low_priority );

    // 情報初期化
    // m_date_modified, m_cookie は初期化しない
    m_charset_det = CharsetDetection::parse_header;
    m_code = HTTP_INIT;
    m_str_code = std::string();
    m_location = std::string();
    m_total_length = 0;
    m_current_length = 0;

    if( !m_loader->run( this, data ) ){
        m_code = get_loader_code();
        m_str_code = get_loader_str_code();
        m_loader.reset();
        return false;
    }

    return true;
}



//
// ロード中止
//
void Loadable::stop_load()
{
    if( m_loader ) m_loader->stop();
}



// ローダーからコールバックされてコードなどを取得してから
// receive_data() を呼び出す
void Loadable::receive( const char* data, size_t size )
{
    m_code = get_loader_code();
    if( ! m_total_length && m_code != HTTP_INIT ) m_total_length = get_loader_length();
    m_current_length += size;

    if( m_charset_det == CharsetDetection::parse_header ) {
        const Encoding enc = get_loader_content_charset();
        if( enc != Encoding::unknown ) {
            set_encoding( enc );
            // The HTTP header has a higher precedence than the in-HTML <meta> elements.
            m_charset_det = CharsetDetection::finished;
        }
        else if( m_loader &&
                 MISC::ascii_ignore_case_find( m_loader->data().contenttype, "html" ) != std::string::npos ) {
            // If the content MIME type is text/html or application/xhtml+xml,
            // find charset from <meta> elements.
            m_charset_det = CharsetDetection::parse_body;
        }
        else {
            m_charset_det = CharsetDetection::finished;
        }
    }
    if( m_charset_det == CharsetDetection::parse_body ) {
        std::string buf( data, size );
        const std::string charset = MISC::parse_charset_from_html_meta( buf );
        if( ! charset.empty() ) {
            const Encoding enc = MISC::encoding_from_web_charset( charset );
            if( enc != Encoding::unknown ) set_encoding( enc );
            m_charset_det = CharsetDetection::finished;
        }
        else if( m_current_length >= 1024 ) {
            // <meta> elements which declare a character encoding must be located
            // entirely within the first 1024 bytes of the document.
            m_charset_det = CharsetDetection::finished;
        }
    }

    receive_data( data, size );
}


// 別スレッドで動いているローダからfinish()を呼ばれたらディスパッチして
// メインスレッドに制御を戻してから callback_dispatch()を呼び出す。
// そうしないと色々不具合が生じる
void Loadable::finish()
{
#ifdef _DEBUG
    std::cout << "Loadable::finish\n";
#endif

    dispatch();
}


//
// ローダを削除してreceive_finish()をコール
//
void Loadable::callback_dispatch()
{
#ifdef _DEBUG
    std::cout << "Loadable::callback_dispatch\n";
#endif

    // ローダを削除する前に情報保存
    m_code = get_loader_code();
    if( ! get_loader_str_code().empty() ) m_str_code = get_loader_str_code();
    if( ! get_loader_contenttype().empty() ) m_contenttype = get_loader_contenttype();
    if( ! get_loader_modified().empty() ) m_date_modified = get_loader_modified();
    if( ! get_loader_cookies().empty() ) m_cookies = get_loader_cookies();
    if( ! get_loader_location().empty() ) m_location = get_loader_location();

    if( m_charset_det == CharsetDetection::parse_header ) {
        const Encoding enc = get_loader_content_charset();
        if( enc != Encoding::unknown ) set_encoding( enc );
        m_charset_det = CharsetDetection::finished;
    }

#ifdef _DEBUG
    std::cout << "delete loader\n";
#endif

    m_loader.reset();

#ifdef _DEBUG
    std::cout << "code = " << m_code << std::endl;
    std::cout << "str_code = " << m_str_code << std::endl;
    std::cout << "contenttype = " << m_contenttype << std::endl;
    std::cout << "modified = " << m_date_modified << std::endl;
    std::cout << "location = " << m_location << std::endl;
    std::cout << "total_length = " << m_total_length << std::endl;
    std::cout << "current length = " << m_current_length << std::endl;
    std::cout << "charset = " << MISC::encoding_to_cstr( get_encoding() ) << std::endl;
#endif

    receive_finish();
}



// ローダから各種情報の取得

int Loadable::get_loader_code() const
{
    if( ! m_loader ) return HTTP_INIT;

    return m_loader->data().code;
}


std::string Loadable::get_loader_str_code() const
{
    if( ! m_loader ) return std::string();

    return m_loader->data().str_code;
}


std::string Loadable::get_loader_contenttype() const
{
    if( ! m_loader ) return std::string();

    return m_loader->data().contenttype;
}


std::string Loadable::get_loader_modified() const
{
    if( ! m_loader ) return std::string();

    return m_loader->data().modified;
}


std::list< std::string > Loadable::get_loader_cookies() const
{
    if( ! m_loader ) return std::list< std::string >();

    return m_loader->data().list_cookies;
}


std::string Loadable::get_loader_location() const
{
    if( ! m_loader ) return std::string();

    return m_loader->data().location;
}


size_t Loadable::get_loader_length() const
{
    if( ! m_loader ) return 0;

    return m_loader->data().length;
}


/** @brief HTTP header Content-Type から文字エンコーディング情報を取得する
 *
 * @return 文字エンコーディングの列挙型
 * @retval Encoding::unknown エンコーディング情報が見つからない、またはJDimでは処理しないエンコーディングだった
 */
Encoding Loadable::get_loader_content_charset() const
{
    if( m_loader ) {
        const std::string& contenttype = m_loader->data().contenttype;
        const std::size_t pos = contenttype.find( "charset=" );
        if( pos != std::string::npos ){
            const std::string raw_charset = MISC::utf8_trim( std::string_view{ contenttype }.substr( pos + 8 ) );
            return MISC::encoding_from_web_charset( raw_charset );
        }
    }
    return Encoding::unknown;
}
