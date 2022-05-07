// License: GPL2

// 日本語文字コードの判定

#ifndef _MISCCHARCODE_H
#define _MISCCHARCODE_H

#include <string>
#include <string_view>


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

    /// @brief get_unicodeblock() の戻り値
    enum class UnicodeBlock
    {
        BasicLatin, ///< 基本ラテン文字 [U+0000, U+007F]
        Hira, ///< 平仮名 [U+3040, U+309F]
        Kata, ///< 片仮名 [U+30A0, U+30FF]

        Other, ///< 上記以外
    };

    /// utf8_fix_wavedash() の変換モード
    enum class WaveDashFix
    {
        UnixToWin, ///< Unix から Windows へ
        WinToUnix, ///< Windows から Unix へ
    };

    bool is_eucjp( std::string_view input, std::size_t read_byte );
    bool is_jis( std::string_view input, std::size_t& read_byte );
    bool is_sjis( const char* input, size_t read_byte );
    bool is_utf8( const char* input, size_t read_byte );
    int judge_char_code( const std::string& str );

    /// utf-8文字のbyte数を返す
    int utf8bytes( const char* utf8str );

    // UTF-8 -> UTF-32 変換
    // 入力 : utf8str 入力文字 (UTF-8)
    // 出力 :  byte  長さ(バイト) utf8str が ASCII なら 1, UTF-8 なら 2 or 3 or 4, それ以外は 0 を入れて返す
    // 戻り値 : unicode code point
    char32_t utf8toutf32( const char* utf8str, int& byte );

    // UTF-32 から UTF-8 に変換する
    // 出力 : utf8str 変換後の文字
    // 戻り値 : バイト数
    int utf32toutf8( const char32_t uch, char* utf8str );

    /// 特定のUnicodeブロックかコードポイントを調べる
    UnicodeBlock get_unicodeblock( const char32_t unich );

    /// WAVE DASH(U+301C)などのWindows系UTF-8文字をUnix系文字と相互変換
    std::string utf8_fix_wavedash( const std::string& str, const WaveDashFix mode );
}

#endif
