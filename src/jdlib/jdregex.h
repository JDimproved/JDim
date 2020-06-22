// ライセンス: GPL2

#ifndef _JDREGEX_H
#define _JDREGEX_H

#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_ONIGPOSIX_H)
#include <onigposix.h>
#elif defined(HAVE_PCREPOSIX_H)
#include <pcreposix.h>
#elif defined(HAVE_REGEX_H)
#include <regex.h>
#else
#include <glib.h>
#endif

#if defined(HAVE_ONIGPOSIX_H) || defined(HAVE_PCREPOSIX_H) || defined(HAVE_REGEX_H)
#define POSIX_STYLE_REGEX_API 1
#endif


namespace JDLIB
{
    class Regex
    {
        std::vector< int > m_pos;
        std::vector< std::string > m_results;

#ifdef POSIX_STYLE_REGEX_API
        regex_t m_reg;
#else
        GRegex* m_reg{};
#endif

        bool m_compiled;
        bool m_newline;
        bool m_wchar;

        // 全角半角を区別しないときに使う変換用バッファ
        // 処理可能なバッファ長は regoff_t (= int) のサイズに制限される
        std::string m_target_asc;
        std::vector< int > m_table_pos;

    public:

        Regex();
        ~Regex();

        void dispose();
        
        // icase : 大文字小文字区別しない
        // newline :  . に改行をマッチさせない
        // usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
        // wchar : 全角半角の区別をしない
        bool compile( const std::string& reg, const bool icase, const bool newline, const bool usemigemo,
                      const bool wchar );

        bool exec( const std::string& target, const size_t offset );
        bool exec( const std::string& reg, const std::string& target, const size_t offset, const bool icase,
                   const bool newline, const bool usemigemo, const bool wchar );

        int pos( const size_t num );
        std::string str( const size_t num );
    };
}

#endif
