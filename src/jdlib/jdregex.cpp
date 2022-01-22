// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdregex.h"
#include "miscutil.h"

#ifdef HAVE_MIGEMO_H
#include "jdmigemo.h"
#endif


constexpr std::size_t MAX_TARGET_SIZE = 64 * 1024;  // 全角半角変換のバッファサイズ


using namespace JDLIB;

RegexPattern::RegexPattern( const std::string& reg, const bool icase, const bool newline,
                            const bool usemigemo, const bool wchar )
{
    set( reg, icase, newline, usemigemo, wchar );
}


RegexPattern::~RegexPattern() noexcept
{
    clear();
}


RegexPattern::RegexPattern( RegexPattern&& other ) noexcept
    : m_regex{ other.m_regex }
    , m_compiled{ other.m_compiled }
    , m_newline{ other.m_newline }
    , m_wchar{ other.m_wchar }
    , m_error{ other.m_error }
{
    other.m_compiled = false;
    other.m_error = decltype( m_error ){};
}


RegexPattern& RegexPattern::operator=( RegexPattern&& other ) noexcept
{
    if( this != &other ) {
        clear();

        m_regex = other.m_regex;
        m_compiled = other.m_compiled;
        m_newline = other.m_newline;
        m_wchar = other.m_wchar;
        m_error = other.m_error;

        other.m_compiled = false;
        other.m_error = decltype( m_error ){};
    }
    return *this;
}


void RegexPattern::clear()
{
    if ( m_compiled ) {
        g_regex_unref( m_regex );
        m_regex = nullptr;
    }
    m_compiled = false;
    g_clear_error( &m_error );
}

// icase : 大文字小文字区別しない
// newline :  . に改行をマッチさせない
// usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
// wchar : 全角半角の区別をしない
bool RegexPattern::set( const std::string& reg, const bool icase, const bool newline,
                        const bool usemigemo, const bool wchar )
{
#ifdef _DEBUG
    if( wchar ){
        std::cout << "RegexPattern::set " << reg << std::endl;
    }
#endif

    clear();

    if( reg.empty() ) return false;

    int cflags = G_REGEX_OPTIMIZE;
    if( newline ) cflags |= G_REGEX_MULTILINE;
    else cflags |= G_REGEX_DOTALL; // . を改行にマッチさせる
    if( icase ) cflags |= G_REGEX_CASELESS;

    m_newline = newline;
    m_wchar = wchar;

    const char* asc_reg = reg.c_str();
    std::string target_asc;

    // 全角英数字 → 半角英数字、半角カナ → 全角カナ
    if( m_wchar && MISC::has_widechar( asc_reg ) ){

        target_asc.reserve( MAX_TARGET_SIZE );
        std::vector<int> temp;
        MISC::asc( asc_reg, target_asc, temp );
        asc_reg = target_asc.c_str();

#ifdef _DEBUG
        std::cout << target_asc << std::endl;
#endif
    }

#ifdef HAVE_MIGEMO_H
    std::string migemo_regex;

    if( usemigemo ) {

        migemo_regex = jdmigemo::convert( asc_reg );
        if( ! migemo_regex.empty() ) {
            asc_reg = migemo_regex.c_str();
        }
    }
#endif

    m_regex = g_regex_new( asc_reg, GRegexCompileFlags( cflags ), GRegexMatchFlags( 0 ), &m_error );
    if( ! m_regex ) {
        return false;
    }

    m_compiled = true;
    return true;
}


std::string RegexPattern::errstr() const
{
    std::string errmsg;

    if( m_error ) {
        errmsg = m_error->message;
    }
    return errmsg;
}


///////////////////////////////////////////////


bool Regex::match( const RegexPattern& creg, const std::string& target,
                   const std::size_t offset, const bool notbol, const bool noteol )
{
    m_pos.clear();
    m_results.clear();
    m_target_asc.clear();
    m_table_pos.clear();

    if ( ! creg.m_compiled ) return false;

    if( target.empty() ) return false;
    if( target.size() <= offset ) return false;

    const char* asc_target = target.c_str() + offset;

    // 全角英数字 → 半角英数字、半角カナ → 全角カナ
    if( creg.m_wchar && MISC::has_widechar( asc_target ) ) {

#ifdef _DEBUG
        std::cout << "Regex::match offset = " << offset << std::endl;
        std::cout << target << std::endl;
#endif

        if( m_target_asc.capacity() < MAX_TARGET_SIZE ) {
            m_target_asc.reserve( MAX_TARGET_SIZE );
            m_table_pos.reserve( MAX_TARGET_SIZE );
        }

        MISC::asc( asc_target, m_target_asc, m_table_pos );
        asc_target = m_target_asc.c_str();

#ifdef _DEBUG
        std::cout << m_target_asc << std::endl;
#endif
    }

    GMatchInfo* pmatch{};

    int eflags = 0;
    if( notbol ) eflags |= G_REGEX_MATCH_NOTBOL;
    if( noteol ) eflags |= G_REGEX_MATCH_NOTEOL;

    if( ! g_regex_match( creg.m_regex, asc_target, GRegexMatchFlags( eflags ), &pmatch ) ) {
        g_match_info_free( pmatch );
        return false;
    }
    const int match_count = g_match_info_get_match_count( pmatch ) + 1;

    for( int i = 0; i < match_count; ++i ){

        int so;
        int eo;
        if( ! g_match_info_fetch_pos( pmatch, i, &so, &eo ) ) so = eo = -1;

        if( so < 0 || eo < 0 ) {
            m_pos.push_back( so );
            m_results.push_back( std::string() );
        }

        else {
            if( ! m_table_pos.empty() ) {
#ifdef _DEBUG
                std::cout << "so=" << so << " eo=" << eo;
#endif
                while( so > 0 && m_table_pos[so] < 0 ) so--;
                so = m_table_pos[so];
                auto it = std::find_if( m_table_pos.cbegin() + eo, m_table_pos.cend(), []( int p ) { return p >= 0; } );
                eo = ( it != m_table_pos.cend() ) ? *it : m_table_pos.size();
#ifdef _DEBUG
                std::cout << " -> so=" << so << " eo=" << eo << std::endl;
#endif
            }
            so += offset;
            eo += offset;

            m_pos.push_back( so );
            m_results.push_back( target.substr( so, eo - so ) );
        }
    }

    g_match_info_free( pmatch );

    return true;
}


//
// マッチした文字列と \0〜\9 を置換する
//
std::string Regex::replace( const std::string& repstr ) const
{
    if( repstr.empty() ) return repstr;

    const char* p0 = repstr.c_str();
    const char* p1;
    std::string str_out;

    while( ( p1 = strchr( p0, '\\' ) ) != nullptr ) {
        int n = p1[1] - '0';
        str_out.append( p0, p1 - p0 );
        p0 = p1 + 2;
        if( n < 0 || n > 9 ) {
            str_out.push_back( p1[1] );
        }
        else if( m_results.size() > static_cast<std::size_t>( n ) && m_pos[n] != -1 ){
            str_out.append( m_results[n] );
        }
    }

    str_out.append( repstr, p0 - repstr.c_str(), std::string::npos );

    return str_out;
}


int Regex::length( std::size_t num ) const noexcept
{
    if( m_results.size() > num ) return m_results[num].size();

    return 0;
}


int Regex::pos( std::size_t num ) const noexcept
{
    if( m_results.size() > num ) return m_pos[num];

    return -1;
}


std::string Regex::str( std::size_t num ) const
{
    if( m_results.size() > num ) return m_results[num];

    return {};
}
