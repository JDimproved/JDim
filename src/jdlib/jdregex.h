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
#elif defined( USE_PCRE )
#include <pcreposix.h>
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
        bool m_wchar;

        char *m_target_asc;
        int *m_table_pos;

    public:

        Regex();
        ~Regex();

        void dispose();
        
        // icase : 大文字小文字区別しない
        // newline :  . に改行をマッチさせない
        // usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
        // wchar : 全角半角の区別をしない
        bool compile( const std::string reg, const bool icase, const bool newline, const bool usemigemo,
                      const bool wchar );

        bool exec( const std::string& target, const size_t offset );
        bool exec( const std::string reg, const std::string& target, const size_t offset, const bool icase,
                   const bool newline, const bool usemigemo, const bool wchar );

        int pos( const size_t num );
        const std::string str( const size_t num );
    };
}

#endif
