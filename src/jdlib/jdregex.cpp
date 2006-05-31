// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "jdregex.h"

#include <regex.h>

#define REGEX_MAX_NMATCH 32

using namespace JDLIB;

Regex::Regex()
{
    m_results.clear();
    m_pos.clear();
}


Regex::~Regex()
{}



// icase : true なら大小無視
// newline : true なら . に改行をマッチさせない
bool Regex::exec( const std::string reg, const std::string& target, unsigned int offset, bool icase, bool newline )
{
    regex_t preg;
    regmatch_t pmatch[ REGEX_MAX_NMATCH ];

    if( reg.empty() ) return false;
    if( target.empty() ) return false;
    if( target.length() <= offset ) return false;

    m_pos.clear();
    m_results.clear();

    int cflags = REG_EXTENDED;
    if( newline ) cflags |= REG_NEWLINE;
    if( icase ) cflags |= REG_ICASE;

    if( regcomp( &preg, reg.c_str(), cflags ) != 0 ){
        regfree( &preg );
        return false;
    }

    if( regexec( &preg, target.c_str() + offset, REGEX_MAX_NMATCH, pmatch, 0 ) != 0 ){
        regfree( &preg );
        return false;
    }

    for( int i = 0; i < REGEX_MAX_NMATCH; ++i ){

        int so = offset + pmatch[ i ].rm_so;
        int eo = offset + pmatch[ i ].rm_eo;

        m_pos.push_back( so );

        if( so >= 0 && eo >= 0 ) m_results.push_back( target.substr( so, eo - so ) );
        else m_results.push_back( std::string() );
    }

    regfree( &preg );
    return true;
}


const std::string Regex::str( unsigned int num )
{
    if( m_results.size() > num  ) return m_results[ num ];

    return std::string();
}


const int Regex::pos( unsigned int num )
{
    if( m_results.size() > num ) return m_pos[ num ];

    return -1;
}
