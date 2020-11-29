// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "cookiemanager.h"

#include "jdregex.h"
#include "miscutil.h"

#include <map>
#include <mutex>


namespace JDLIB
{
namespace detail
{

//
// シンプルなクッキー
//
struct SimpleCookie
{
    std::string name;
    std::string value;
    std::string domain;
};


//
// シンプルなクッキー解析
// 最低限の要素以外は無視する
//
struct SimpleCookieParser
{
public:

    // 解析に成功したらresultに結果を代入してtrueを返す
    bool parse( const std::string& input, SimpleCookie& result );
};


//
// シンプルなクッキー管理マネージャ (シングルトン用)
// ミューテックスを利用しているのでメンバー関数内で再帰的に呼び出さないように注意
//
class SimpleCookieManager : public JDLIB::CookieManager
{
public:

    SimpleCookieManager() = default;
    ~SimpleCookieManager() = default;

    // シングルトン オブジェクトが前提になっているためコピーとムーブ禁止
    SimpleCookieManager( const SimpleCookieManager& ) = delete;
    SimpleCookieManager( SimpleCookieManager&& ) = delete;

    SimpleCookieManager& operator=( const SimpleCookieManager& ) = delete;
    SimpleCookieManager& operator=( SimpleCookieManager&& ) = delete;

    void feed( const std::string& hostname, const std::string& input ) override;
    std::string get_cookie_by_host( const std::string& hostname ) const override;
    void delete_cookie_by_host( const std::string& hostname ) override;

private:

    SimpleCookieParser m_parser;
    std::map<std::string, std::map<std::string, std::string>> m_storage;
};


std::mutex s_simple_cookie_manager_mutex;

} // namespace detail
} // namespace JDLIB


namespace {

// シングルトン オブジェクト
JDLIB::CookieManager* singleton_manager{};
std::mutex s_singleton_manager_mutex;

} // namespace


JDLIB::CookieManager* JDLIB::get_cookie_manager()
{
    std::lock_guard<std::mutex> lock( s_singleton_manager_mutex );

    if( ! singleton_manager ) singleton_manager = new JDLIB::detail::SimpleCookieManager();
    return singleton_manager;
}


void JDLIB::delete_cookie_manager()
{
    std::lock_guard<std::mutex> lock( s_singleton_manager_mutex );

    if( singleton_manager ) delete singleton_manager;
    singleton_manager = nullptr;
}


using namespace JDLIB::detail;


bool SimpleCookieParser::parse( const std::string& input, SimpleCookie& result )
{
    // HTTPヘッダー名を考慮しないためinputに渡す前にヘッダー名と空白は取り除くこと
    // 実装をシンプルにするため規格より簡略化された解析になっている

    constexpr bool icase = true;
    constexpr bool newline = false;
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    JDLIB::Regex regex;
    std::size_t offset = 0;

    // クオート(")やエスケープ(\)が含まれているクッキーは解析失敗にする
    if( regex.exec( R"-(([^"\=;]+)=([^"\=;]*))-", input, offset, icase, newline, usemigemo, wchar ) ) {
        result.name = regex.str( 1 );
        result.value = regex.str( 0 );
        offset = regex.length( 0 );
    }
    else {
        return false;
    }

    if( regex.exec( R"-(path=/([^;]+))-", input, offset, icase, newline, usemigemo, wchar ) ) {
        // ルート(/)以外のpathは解析失敗にする
        if( regex.length( 1 ) ) return false;
    }
    if( regex.exec( R"-(domain=([^;]+))-", input, offset, icase, newline, usemigemo, wchar ) ) {
        result.domain = regex.str( 1 );
    }

    return true;
}


void SimpleCookieManager::feed( const std::string& hostname, const std::string& input )
{
#ifdef _DEBUG
    std::cout << "SimpleCookieManager::feed hostname = " << hostname << std::endl;
    std::cout << "SimpleCookieManager::feed input = " << input << std::endl;
#endif

    SimpleCookie result;
    if( ! m_parser.parse( input, result ) ) return;

    // ドメインの解析結果が空、またはホスト名と一致する場合はホスト名を使う
    if( result.domain.empty() || result.domain == "." + hostname ) {
        result.domain = hostname;
    }

#ifdef _DEBUG
    std::cout << "SimpleCookieManager::feed name = " << result.name << std::endl;
    std::cout << "SimpleCookieManager::feed value = " << result.value << std::endl;
    std::cout << "SimpleCookieManager::feed domain = " << result.domain << std::endl;
#endif

    std::lock_guard<std::mutex> lock( s_simple_cookie_manager_mutex );
    m_storage[result.domain][result.name] = result.value;
}


std::string SimpleCookieManager::get_cookie_by_host( const std::string& hostname ) const
{
    std::string output;

    std::lock_guard<std::mutex> lock( s_simple_cookie_manager_mutex );

    std::size_t i = 0;
    const char* sep = "";
    while( i != std::string::npos ) {
        auto it = m_storage.find( hostname.substr( i ) );
        if( it != m_storage.end() ) {
            output.append( sep );
            sep = "";
            for( auto& pair : it->second ) {
                output.append( sep );
                output.append( pair.second );
                sep = "; ";
            }
        }
        i = hostname.find( '.', i + 1 );
    }
    return output;
}


void SimpleCookieManager::delete_cookie_by_host( const std::string& hostname )
{
    std::lock_guard<std::mutex> lock( s_simple_cookie_manager_mutex );

    std::size_t i = 0;
    while( i != std::string::npos ) {
        auto it = m_storage.find( hostname.substr( i ) );
        if( it != m_storage.end() ) {
            m_storage.erase( it );
        }
        i = hostname.find( '.', i + 1 );
    }
}
