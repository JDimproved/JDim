// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdregex.h"
#include "miscutil.h"

#ifdef HAVE_MIGEMO_H
#include "jdmigemo.h"
#endif


constexpr std::size_t MAX_TARGET_SIZE = 64 * 1024;  // 全角半角変換のバッファサイズ
#ifdef POSIX_STYLE_REGEX_API
constexpr std::size_t REGEX_MAX_NMATCH = 32;
#endif


using namespace JDLIB;

Regex::Regex()
    : m_compiled(false),
      m_newline(false),
      m_wchar(false)
{
    m_results.clear();
    m_pos.clear();
}


Regex::~Regex()
{
    dispose();
}


void Regex::dispose()
{
    if ( m_compiled ) {
#ifdef POSIX_STYLE_REGEX_API
        regfree( &m_reg );
#else
        g_regex_unref( m_reg );
        m_reg = nullptr;
#endif
        m_compiled = false;
    }
}


// icase : 大文字小文字区別しない
// newline :  . に改行をマッチさせない
// usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
// wchar : 全角半角の区別をしない
bool Regex::compile( const std::string& reg, const bool icase, const bool newline, const bool use_migemo, const bool wchar )
{
#ifdef _DEBUG
    if( wchar ){
        std::cout << "Regex::compile\n";
        std::cout << reg << std::endl;
    }
#endif

    dispose();

    if( reg.empty() ) return false;

#if POSIX_STYLE_REGEX_API
#ifdef HAVE_PCREPOSIX_H
    int cflags = REG_UTF8;
    if( ! newline ) cflags |= REG_DOTALL; // . を改行にマッチさせる
#else
    int cflags = REG_EXTENDED;
#endif
    if( newline ) cflags |= REG_NEWLINE;
    if( icase ) cflags |= REG_ICASE;
#else
    int cflags = G_REGEX_OPTIMIZE;
    if( newline ) cflags |= G_REGEX_MULTILINE;
    else cflags |= G_REGEX_DOTALL; // . を改行にマッチさせる
    if( icase ) cflags |= G_REGEX_CASELESS;
#endif // POSIX_STYLE_REGEX_API

    m_newline = newline;
    m_wchar = wchar;

    const char* asc_reg = reg.c_str();

    // 全角英数字 → 半角英数字、半角カナ → 全角カナ
    if( m_wchar && MISC::has_widechar( asc_reg ) ){

        m_target_asc.clear();
        m_table_pos.clear();
        if( m_target_asc.capacity() < MAX_TARGET_SIZE ) {
            m_target_asc.reserve( MAX_TARGET_SIZE );
            m_table_pos.reserve( MAX_TARGET_SIZE );
        }

        MISC::asc( asc_reg, m_target_asc, m_table_pos );
        asc_reg = m_target_asc.c_str();

#ifdef _DEBUG
        std::cout << m_target_asc << std::endl;
#endif
    }

#ifdef HAVE_MIGEMO_H
    std::string migemo_regex;

    if( use_migemo ){

        migemo_regex = jdmigemo::convert( asc_reg );
        if( ! migemo_regex.empty() ) {
            asc_reg = migemo_regex.c_str();
        }
    }
#endif

#ifdef POSIX_STYLE_REGEX_API
    if( regcomp( &m_reg, asc_reg, cflags ) != 0 ){
        regfree( &m_reg );
        return false;
    }
#else
    m_reg = g_regex_new( asc_reg, GRegexCompileFlags( cflags ), GRegexMatchFlags( 0 ), nullptr );
    if( m_reg == nullptr ){
        return false;
    }
#endif

    m_compiled = true;
    return true;
}


bool Regex::exec( const std::string& target, const size_t offset )
{
    if ( ! m_compiled ) return false;

    if( target.empty() ) return false;
    if( target.length() <= offset ) return false;

    m_pos.clear();
    m_results.clear();

    const char* asc_target = target.c_str() + offset;

    bool exec_asc = false;

    // 全角英数字 → 半角英数字、半角カナ → 全角カナ
    if( m_wchar && MISC::has_widechar( asc_target ) ){

#ifdef _DEBUG
        std::cout << "Regex::exec offset = " << offset << std::endl;
        std::cout << target << std::endl;
#endif

        m_target_asc.clear();
        m_table_pos.clear();
        if( m_target_asc.capacity() < MAX_TARGET_SIZE ) {
            m_target_asc.reserve( MAX_TARGET_SIZE );
            m_table_pos.reserve( MAX_TARGET_SIZE );
        }

        MISC::asc( asc_target, m_target_asc, m_table_pos );
        exec_asc = true;
        asc_target = m_target_asc.c_str();

#ifdef _DEBUG
        std::cout << m_target_asc << std::endl;
#endif
    }

#ifdef HAVE_ONIGPOSIX_H
    std::string target_copy;

    // 鬼車はnewlineを無視するようなので、文字列のコピーを取って
    // 改行をスペースにしてから実行する
    if( ! m_newline ){
        target_copy = asc_target;
        std::replace( target_copy.begin(), target_copy.end(), '\n', ' ' );
        asc_target = target_copy.c_str();
    }
#endif

#ifdef POSIX_STYLE_REGEX_API
    regmatch_t pmatch[ REGEX_MAX_NMATCH ]{};
#else
    GMatchInfo* pmatch{};
#endif

#ifdef POSIX_STYLE_REGEX_API
    if( regexec( &m_reg, asc_target, REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
        return false;
    }
    constexpr int match_count = REGEX_MAX_NMATCH;
#else
    if( ! g_regex_match( m_reg, asc_target, GRegexMatchFlags( 0 ), &pmatch ) ){
        g_match_info_free( pmatch );
        return false;
    }
    const int match_count = g_match_info_get_match_count( pmatch ) + 1;
#endif

    for( int i = 0; i < match_count; ++i ){

#ifdef POSIX_STYLE_REGEX_API
        int so = pmatch[ i ].rm_so;
        int eo = pmatch[ i ].rm_eo;
#else
        int so;
        int eo;
        if( ! g_match_info_fetch_pos( pmatch, i, &so, &eo ) ) so = eo = -1;
#endif

        if( exec_asc && so >= 0 && eo >= 0 ){
#ifdef _DEBUG
            std::cout << "so = " << so << " eo = " << eo;
#endif
            so = m_table_pos[ so ];
            eo = m_table_pos[ eo ];
#ifdef _DEBUG
            std::cout << " -> so = " << so << " eo = " << eo << std::endl;
#endif
        }
        so += offset;
        eo += offset;

        m_pos.push_back( so );

        if( so >= 0 && eo >= 0 ) m_results.push_back( target.substr( so, eo - so ) );
        else m_results.push_back( std::string() );
    }

#ifndef POSIX_STYLE_REGEX_API
    g_match_info_free( pmatch );
#endif

    return true;
}


// icase : 大文字小文字区別しない
// newline :  . に改行をマッチさせない
// usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
// wchar : 全角半角の区別をしない
bool Regex::exec( const std::string& reg, const std::string& target,
                  const size_t offset, const bool icase, const bool newline, const bool use_migemo, const bool wchar )
{
    if ( ! compile(reg, icase, newline, use_migemo, wchar ) ) return false;

    if ( ! exec(target, offset) ){
        dispose();
        return false;
    }
    
    dispose();
    
    return true;
}


std::string Regex::str( const size_t num )
{
    if( m_results.size() > num  ) return m_results[ num ];

    return std::string();
}


int Regex::pos( const size_t num )
{
    if( m_results.size() > num ) return m_pos[ num ];

    return -1;
}
