// License: GPL2

// 日本語文字コードの判定

#ifndef _MISCCHARCODE_H
#define _MISCCHARCODE_H

#include <string>

namespace MISC
{
    enum CodeSet
    {
        CHARCODE_UNKNOWN = -1,
        CHARCODE_ASCII = 0,
        CHARCODE_EUC_JP,
        CHARCODE_JIS,
        CHARCODE_SJIS,
        CHARCODE_UTF
    };

    bool is_euc( const char* input, size_t& read_byte );
    bool is_jis( const char* input, size_t& read_byte );
    bool is_sjis( const char* input, size_t& read_byte );
    bool is_utf( const char* input, size_t& read_byte );
    int judge_char_code( const std::string& str );
}

#endif
