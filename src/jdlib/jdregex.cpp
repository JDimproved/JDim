// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdregex.h"

#ifdef HAVE_MIGEMO_H
#include "jdmigemo.h"
#endif

#include <cstring>

enum
{
    REGEX_MAX_NMATCH = 32
};

using namespace JDLIB;

Regex::Regex()
    : m_compiled(false)
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
        regfree( &m_reg );
        m_compiled = false;
    }
}

// icase : true なら大小無視
// newline : true なら . に改行をマッチさせない
const bool Regex::compile( const std::string reg, const bool icase, const bool newline, const bool use_migemo )
{
    dispose();
    
    if( reg.empty() ) return false;
    
    int cflags = REG_EXTENDED;
    if( newline ) cflags |= REG_NEWLINE;
    if( icase ) cflags |= REG_ICASE;

    m_newline = newline;

#ifdef HAVE_MIGEMO_H

    if( use_migemo ){

        if( jd_migemo_regcomp( &m_reg, reg.c_str(),cflags ) != 0 ){
            if( regcomp( &m_reg, reg.c_str(), cflags ) != 0 ){
                regfree( &m_reg );
                return false;
            }
        }
    }
    else{
#endif

    if( regcomp( &m_reg, reg.c_str(), cflags ) != 0 ){
        regfree( &m_reg );
        return false;
    }

#ifdef HAVE_MIGEMO_H
    }
#endif

    m_compiled = true;
    return true;
}

const bool Regex::exec( const std::string& target, const unsigned int offset )
{
    regmatch_t pmatch[ REGEX_MAX_NMATCH ];

    memset(pmatch, 0, sizeof(pmatch));

    if ( ! m_compiled ) return false;
	
    if( target.empty() ) return false;
    if( target.length() <= offset ) return false;

    m_pos.clear();
    m_results.clear();

#ifdef USE_ONIG    

    // 鬼車はnewlineを無視するようなので、文字列のコピーを取って
    // 改行をスペースにしてから実行する
    if( ! m_newline ){

        std::string target_copy = target;
        for( size_t i = 0; i < target.size(); ++i ) if( target_copy[ i ] == '\n' ) target_copy[ i ] = ' ';
        if( regexec( &m_reg, target_copy.c_str() + offset, REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
            return false;
        }
    }
    else

#endif

    if( regexec( &m_reg, target.c_str() + offset, REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
        return false;
    }

    for( int i = 0; i < REGEX_MAX_NMATCH; ++i ){

        int so = offset + pmatch[ i ].rm_so;
        int eo = offset + pmatch[ i ].rm_eo;

        m_pos.push_back( so );

        if( so >= 0 && eo >= 0 ) m_results.push_back( target.substr( so, eo - so ) );
        else m_results.push_back( std::string() );
    }
    
    return true;
}

// icase : true なら大小無視
// newline : true なら . に改行をマッチさせない
const bool Regex::exec( const std::string reg, const std::string& target,
                        const unsigned int offset, const bool icase, const bool newline, const bool use_migemo )
{
    if ( ! compile(reg, icase, newline, use_migemo) ) return false;

    if ( ! exec(target, offset) ){
        dispose();
        return false;
    }
    
    dispose();
    
    return true;
}


const std::string Regex::str( const unsigned int num )
{
    if( m_results.size() > num  ) return m_results[ num ];

    return std::string();
}


const int Regex::pos( const unsigned int num )
{
    if( m_results.size() > num ) return m_pos[ num ];

    return -1;
}
