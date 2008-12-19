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

    public:

        Regex();
        ~Regex();

        void dispose();
        
        // icase : true なら大小無視
        // newline : true なら . に改行をマッチさせない
        bool compile( const std::string reg
                   , bool icase = false, bool newline = true, bool usemigemo = false );
        bool exec( const std::string& target, unsigned int offset = 0 );
        bool exec( const std::string reg, const std::string& target, unsigned int offset = 0
                   , bool icase = false, bool newline = true, bool usemigemo = false );
        const int pos( unsigned int num );
        const std::string str( unsigned int num );
    };
}

#endif
