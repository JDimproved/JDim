// ライセンス: GPL2

#ifndef _JDREGEX_H
#define _JDREGEX_H

#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ONIG
#include <onigposix.h>
#else
#include <regex.h>
#endif	/** USE_ONIG **/


namespace JDLIB
{
    class Regex
    {
        std::vector< int > m_pos;
        std::vector< std::string > m_results;
        regex_t m_reg;
        bool m_compiled;
        bool m_newline;

    public:

        Regex();
        ~Regex();

        void dispose();
        
        // icase : true なら大小無視
        // newline : true なら . に改行をマッチさせない
        const bool compile( const std::string reg
                   , const bool icase = false, const bool newline = true, const bool usemigemo = false );
        const bool exec( const std::string& target, const unsigned int offset = 0 );
        const bool exec( const std::string reg, const std::string& target, const unsigned int offset = 0
                         , const bool icase = false, const bool newline = true, const bool usemigemo = false );
        const int pos( const unsigned int num );
        const std::string str( const unsigned int num );
    };
}

#endif
