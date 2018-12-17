// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdregex.h"
#include "miscutil.h"

#ifdef HAVE_MIGEMO_H
#include "jdmigemo.h"
#endif

#include <cstring>

enum
{
    MAX_TARGET_SIZE = 64 * 1024,   // 全角半角変換のバッファサイズ
    REGEX_MAX_NMATCH = 32
};

using namespace JDLIB;

Regex::Regex()
    : m_compiled(false),
      m_target_asc( NULL ),
      m_table_pos( NULL )
{
    m_results.clear();
    m_pos.clear();
}


Regex::~Regex()
{
    dispose();

    if( m_target_asc ) free( m_target_asc );
    if( m_table_pos ) free( m_table_pos );
}


void Regex::dispose()
{
    if ( m_compiled ) {
        regfree( &m_reg );
        m_compiled = false;
    }
}


// icase : 大文字小文字区別しない
// newline :  . に改行をマッチさせない
// usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
// wchar : 全角半角の区別をしない
bool Regex::compile( const std::string reg, const bool icase, const bool newline, const bool use_migemo, const bool wchar )
{
#ifdef _DEBUG
    if( wchar ){
        std::cout << "Regex::compile\n";
        std::cout << reg << std::endl;
    }
#endif

    dispose();
    
    if( reg.empty() ) return false;
    
#ifdef USE_PCRE
    int cflags = REG_UTF8;
    if( ! newline ) cflags |= REG_DOTALL; // . を改行にマッチさせる
#else
    int cflags = REG_EXTENDED;
#endif
    if( newline ) cflags |= REG_NEWLINE;
    if( icase ) cflags |= REG_ICASE;

    m_newline = newline;
    m_wchar = wchar;

    const char* asc_reg = reg.c_str();

    // 全角英数字 → 半角英数字、半角カナ → 全角カナ
    if( m_wchar && MISC::has_widechar( asc_reg ) ){

        if( ! m_target_asc ) m_target_asc = ( char* )malloc( MAX_TARGET_SIZE );
        if( ! m_table_pos ) m_table_pos = ( int* )malloc( MAX_TARGET_SIZE );

        MISC::asc( asc_reg, m_target_asc, m_table_pos, MAX_TARGET_SIZE );
        asc_reg = m_target_asc;

#ifdef _DEBUG
        std::cout << m_target_asc << std::endl;
#endif
    }

#ifdef HAVE_MIGEMO_H

    if( use_migemo ){

        if( jd_migemo_regcomp( &m_reg, asc_reg, cflags ) != 0 ){

            if( regcomp( &m_reg, asc_reg, cflags ) != 0 ){
                regfree( &m_reg );
                return false;
            }
        }
    }
    else{
#endif

    if( regcomp( &m_reg, asc_reg, cflags ) != 0 ){
        regfree( &m_reg );
        return false;
    }

#ifdef HAVE_MIGEMO_H
    }
#endif

    m_compiled = true;
    return true;
}


bool Regex::exec( const std::string& target, const size_t offset )
{
    regmatch_t pmatch[ REGEX_MAX_NMATCH ];

    memset(pmatch, 0, sizeof(pmatch));

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

        if( ! m_target_asc ) m_target_asc = ( char* )malloc( MAX_TARGET_SIZE );
        if( ! m_table_pos ) m_table_pos = ( int* )malloc( MAX_TARGET_SIZE );

        MISC::asc( asc_target, m_target_asc, m_table_pos, MAX_TARGET_SIZE );
        exec_asc = true;
        asc_target = m_target_asc;

#ifdef _DEBUG
        std::cout << m_target_asc << std::endl;
#endif
    }

#ifdef USE_ONIG    

    // 鬼車はnewlineを無視するようなので、文字列のコピーを取って
    // 改行をスペースにしてから実行する
    if( ! m_newline ){

        std::string target_copy = asc_target;
        for( size_t i = 0; i < target_copy.size(); ++i ) if( target_copy[ i ] == '\n' ) target_copy[ i ] = ' ';
        if( regexec( &m_reg, target_copy.c_str(), REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
            return false;
        }
    }
    else

#endif

    if( regexec( &m_reg, asc_target, REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
        return false;
    }

    for( int i = 0; i < REGEX_MAX_NMATCH; ++i ){

        int so = pmatch[ i ].rm_so;
        int eo = pmatch[ i ].rm_eo;
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
    
    return true;
}


// icase : 大文字小文字区別しない
// newline :  . に改行をマッチさせない
// usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
// wchar : 全角半角の区別をしない
bool Regex::exec( const std::string reg, const std::string& target,
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
