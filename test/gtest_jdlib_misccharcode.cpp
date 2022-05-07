// SPDX-License-Identifier: GPL-2.0-only

#include "jdlib/misccharcode.h"

#include "gtest/gtest.h"


namespace {

class IsEucjpTest : public ::testing::Test {};

TEST_F(IsEucjpTest, null_data)
{
    EXPECT_TRUE( MISC::is_eucjp( "", 0 ) );
}

TEST_F(IsEucjpTest, ascii_only)
{
    EXPECT_TRUE( MISC::is_eucjp( "!\"#$%&'()*+,-./ :;<=>?@ [\\]^_` {|}~", 0 ) );
    EXPECT_TRUE( MISC::is_eucjp( "0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0 ) );
}

TEST_F(IsEucjpTest, hiragana_katakana)
{
    EXPECT_TRUE( MISC::is_eucjp( "\xA4\xA2\xA4\xA4\xA4\xA6\xA4\xA8\xA4\xAA", 0 ) ); // あいうえお
    EXPECT_TRUE( MISC::is_eucjp( "\xA5\xA2\xA5\xA4\xA5\xA6\xA5\xA8\xA5\xAA", 0 ) ); // アイウエオ
}

TEST_F(IsEucjpTest, fullwidth_alnum)
{
    EXPECT_TRUE( MISC::is_eucjp( "\xA3\xB1\xA3\xB2\xA3\xB3\xA3\xB4\xA3\xB5", 0 ) ); // １２３４５
    EXPECT_TRUE( MISC::is_eucjp( "\xA3\xC1\xA3\xC2\xA3\xC3\xA3\xC4\xA3\xC5", 0 ) ); // ＡＢＣＤＥ
    EXPECT_TRUE( MISC::is_eucjp( "\xA3\xE1\xA3\xE2\xA3\xE3\xA3\xE4\xA3\xE5", 0 ) ); // ａｂｃｄｅ
}

TEST_F(IsEucjpTest, halfwidth_katakana)
{
    EXPECT_TRUE( MISC::is_eucjp( "\x8E\xA7\x8E\xA8\x8E\xA9\x8E\xAA\x8E\xAB", 0 ) ); // ｱｲｳｴｵ
}

TEST_F(IsEucjpTest, three_byte_sub_kanji)
{
    EXPECT_TRUE( MISC::is_eucjp( "\x8F\xB0\xD0\x8F\xB0\xD1\x8F\xB0\xD2\x8F\xB0\xD2\x8F\xB0\xD3", 0 ) ); // 仾仿伀
}

TEST_F(IsEucjpTest, jis)
{
    EXPECT_TRUE( MISC::is_eucjp( "\x1B$B$\"$$$&$($*\x1B(B", 0 ) );
}

TEST_F(IsEucjpTest, sjis)
{
    EXPECT_FALSE( MISC::is_eucjp( "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8", 0 ) );
}

TEST_F(IsEucjpTest, utf8)
{
    EXPECT_FALSE( MISC::is_eucjp( "\u3042", 0 ) );
}

TEST_F(IsEucjpTest, skip_data)
{
    EXPECT_TRUE( MISC::is_eucjp( "\u3042\xA4\xA2\xA3\xB1", 3 ) );
}

TEST_F(IsEucjpTest, lack_head_byte)
{
    // hiragana
    EXPECT_FALSE( MISC::is_eucjp( "\xA2", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xA2\xA4\xA4", 0 ) );

    // fullwidth
    EXPECT_FALSE( MISC::is_eucjp( "\xB1\xA3\xB2", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xC1\xA3\xC2", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xE1\xA3\xE2", 0 ) );

    // halfwidth katakana
    EXPECT_FALSE( MISC::is_eucjp( "\xA7\x8E\xA8", 0 ) );

    // three byte sub kanji
    EXPECT_FALSE( MISC::is_eucjp( "\xB0", 0 ) );
    EXPECT_TRUE( MISC::is_eucjp( "\xB0\xD0", 0 ) );
    EXPECT_TRUE( MISC::is_eucjp( "\xB0\xD0\x8F\xB0\xD1", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xD0", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xD0\x8F\xB0\xD1", 0 ) );
}

TEST_F(IsEucjpTest, lack_following_bytes)
{
    // hiragana
    EXPECT_FALSE( MISC::is_eucjp( "\xA4", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xA4\xA2\xA4", 0 ) );

    // fullwidth
    EXPECT_FALSE( MISC::is_eucjp( "\xA3\xB1\xA3", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xA3\xC1\xA3", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\xA3\xE1\xA3", 0 ) );

    // halfwidth katakana
    EXPECT_FALSE( MISC::is_eucjp( "\x8E", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\x8E\xA7\x8E", 0 ) );

    // three byte sub kanji
    EXPECT_FALSE( MISC::is_eucjp( "\x8F", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\x8F\xB0", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\x8F\xB0\xD0\x8F", 0 ) );
    EXPECT_FALSE( MISC::is_eucjp( "\x8F\xB0\xD0\x8F\xB0", 0 ) );
}


class IsJisTest : public ::testing::Test {};

TEST_F(IsJisTest, null_data)
{
    std::size_t byte = 0;
    EXPECT_FALSE( MISC::is_jis( "", byte ) );
}

TEST_F(IsJisTest, ascii_only)
{
    std::size_t byte = 0;
    EXPECT_FALSE( MISC::is_jis( "!\"#$%&'()*+,-./ :;<=>?@ [\\]^_` {|}~", byte ) );
    EXPECT_EQ( 35, byte );
    byte = 0;
    EXPECT_FALSE( MISC::is_jis( "0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ", byte ) );
    EXPECT_EQ( 64, byte );
}

TEST_F(IsJisTest, hiragana_katakana)
{
    std::size_t byte = 0;
    EXPECT_TRUE( MISC::is_jis( "\x1B$B$\"$$$&$($*\x1B(B", byte ) ); // あいうえお
    EXPECT_EQ( 0, byte );
    EXPECT_TRUE( MISC::is_jis( "\x1B$B%\"%$%&%(%*\x1B(B", byte ) ); // アイウエオ
    EXPECT_EQ( 0, byte );
}

TEST_F(IsJisTest, fullwidth_alnum)
{
    std::size_t byte = 0;
    EXPECT_TRUE( MISC::is_jis( "\x1B$B!*!I!t!p!s\x1B(B", byte ) ); // ！”＃＄％
    EXPECT_EQ( 0, byte );
    EXPECT_TRUE( MISC::is_jis( "\x1B$B#1#2#3#4#5\x1B(B", byte ) ); // １２３４５
    EXPECT_EQ( 0, byte );
    EXPECT_TRUE( MISC::is_jis( "\x1B$B#A#B#C#D#E\x1B(B", byte ) ); // ＡＢＣＤＥ
    EXPECT_EQ( 0, byte );
    EXPECT_TRUE( MISC::is_jis( "\x1B$B#a#b#c#d#e\x1B(B", byte ) ); // ａｂｃｄｅ
    EXPECT_EQ( 0, byte );
}

TEST_F(IsJisTest, eucjp)
{
    std::size_t byte = 0;
    EXPECT_FALSE( MISC::is_jis( "\xA4\xA2\xA4\xA4\xA4\xA6\xA4\xA8\xA4\xAA", byte ) );
    EXPECT_EQ( 0, byte );
}

TEST_F(IsJisTest, sjis)
{
    std::size_t byte = 0;
    EXPECT_FALSE( MISC::is_jis( "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8", byte ) );
    EXPECT_EQ( 0, byte );
}

TEST_F(IsJisTest, utf8)
{
    std::size_t byte = 0;
    EXPECT_FALSE( MISC::is_jis( "\u3042", byte ) ); // U+3042
    EXPECT_EQ( 0, byte );
}

TEST_F(IsJisTest, skip_data)
{
    std::size_t byte = 3;
    EXPECT_TRUE( MISC::is_jis( "\u3042\x1B$B#A#B#C#D#E\x1B(B", byte ) ); // U+3042ＡＢＣＤＥ
    EXPECT_EQ( 3, byte );
}


class Utf8BytesTest : public ::testing::Test {};

TEST_F(Utf8BytesTest, null_data)
{
    EXPECT_EQ( 0, MISC::utf8bytes( nullptr ) );
    EXPECT_EQ( 0, MISC::utf8bytes( "" ) );
}

TEST_F(Utf8BytesTest, ascii)
{
    char ascii_seq[2]{};
    for( int i = 1; i < 0x80; ++i ) {
        ascii_seq[0] = i;
        EXPECT_EQ( 1, MISC::utf8bytes( ascii_seq ) );
    }
}

TEST_F(Utf8BytesTest, two_bytes)
{
    EXPECT_EQ( 2, MISC::utf8bytes( "\xC2\x80" ) ); // U+0080
    EXPECT_EQ( 2, MISC::utf8bytes( "\xDF\xBF" ) ); // U+07FF
}

TEST_F(Utf8BytesTest, three_bytes)
{
    EXPECT_EQ( 3, MISC::utf8bytes( "\xE0\xA0\x80" ) ); // U+0800
    EXPECT_EQ( 3, MISC::utf8bytes( "\xEF\xBF\xBF" ) ); // U+FFFF
}

TEST_F(Utf8BytesTest, four_bytes)
{
    EXPECT_EQ( 4, MISC::utf8bytes( "\xF0\x90\x80\x80" ) ); // U+10000
    EXPECT_EQ( 4, MISC::utf8bytes( "\xF4\x8F\xBF\xBF" ) ); // U+10FFFF
}

TEST_F(Utf8BytesTest, obsolete_four_bytes)
{
    // 廃止された RFC2279
    // 簡易的なチェックのため先頭が F4 のシーケンスは4(バイト)が返る
    EXPECT_EQ( 4, MISC::utf8bytes( "\xF4\x90\x80\x80" ) ); // U+110000
    EXPECT_EQ( 4, MISC::utf8bytes( "\xF4\xBF\xBF\xBF" ) ); // U+13FFFF

    EXPECT_EQ( 0, MISC::utf8bytes( "\xF5\x80\x80\x80" ) ); // U+140000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xF6\x80\x80\x80" ) ); // U+180000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xF7\x80\x80\x80" ) ); // U+1C0000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xF7\xBF\xBF\xBF" ) ); // U+1FFFFF
}

TEST_F(Utf8BytesTest, obsolete_five_bytes)
{
    // 廃止された RFC2279
    EXPECT_EQ( 0, MISC::utf8bytes( "\xF8\x88\x80\x80\x80" ) ); // U+200000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xF9\x80\x80\x80\x80" ) ); // U+1000000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFA\x80\x80\x80\x80" ) ); // U+2000000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFB\x80\x80\x80\x80" ) ); // U+3000000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFB\xBF\xBF\xBF\xBF" ) ); // U+3FFFFFF
}

TEST_F(Utf8BytesTest, obsolete_six_bytes)
{
    // 廃止された RFC2279
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFC\x84\x80\x80\x80\x80" ) ); // U+4000000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFD\x80\x80\x80\x80\x80" ) ); // U+40000000
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFD\xBF\xBF\xBF\xBF\xBF" ) ); // U+7FFFFFFF
}

TEST_F(Utf8BytesTest, invalid_byte)
{
    // マルチバイトの後続部分
    char invalid_seq[2]{};
    for( int i = 0x80; i < 0xC0; ++i ) {
        invalid_seq[0] = i;
        EXPECT_EQ( 0, MISC::utf8bytes( invalid_seq ) );
    }

    // 最少のバイト数による表現以外は不正
    EXPECT_EQ( 0, MISC::utf8bytes( "\xC0" ) );
    EXPECT_EQ( 0, MISC::utf8bytes( "\xC1" ) );

    // UTF-16, UTF-32 で使われるバイトオーダーマーク
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFE" ) );
    EXPECT_EQ( 0, MISC::utf8bytes( "\xFF" ) );
}

TEST_F(Utf8BytesTest, invalid_seq)
{
    EXPECT_EQ( 0, MISC::utf8bytes( "\xC2\x7F" ) );
    EXPECT_EQ( 0, MISC::utf8bytes( "\xC2\xC0" ) );

    // 簡易的なチェックのため非ゼロが返る (右コメントは正しい範囲)
    EXPECT_EQ( 3, MISC::utf8bytes( "\xE0\x9F\x80" ) ); // E0 A0-BF 80-BF
    EXPECT_EQ( 3, MISC::utf8bytes( "\xED\xA0\x80" ) ); // ED 80-9F 80-BF
    EXPECT_EQ( 4, MISC::utf8bytes( "\xF0\x8F\x80\x80" ) ); // F0 90-BF 80-BF 80-BF
}


class Utf8ToUtf32Test : public ::testing::Test {};

TEST_F(Utf8ToUtf32Test, null_data)
{
    int byte;
    EXPECT_EQ( 0, MISC::utf8toutf32( nullptr, byte ) );
    EXPECT_EQ( 0, byte );
    EXPECT_EQ( 0, MISC::utf8toutf32( "", byte ) );
    EXPECT_EQ( 0, byte );
}

TEST_F(Utf8ToUtf32Test, ascii)
{
    int byte;
    EXPECT_EQ( 0x0001, MISC::utf8toutf32( "\x01", byte ) );
    EXPECT_EQ( 1, byte );
    EXPECT_EQ( 0x007F, MISC::utf8toutf32( "\x7F", byte ) );
    EXPECT_EQ( 1, byte );
}

TEST_F(Utf8ToUtf32Test, two_bytes)
{
    int byte;
    EXPECT_EQ( 0x0080, MISC::utf8toutf32( "\xC2\x80", byte ) );
    EXPECT_EQ( 2, byte );
    EXPECT_EQ( 0x07FF, MISC::utf8toutf32( "\xDF\xBF", byte ) );
    EXPECT_EQ( 2, byte );
}

TEST_F(Utf8ToUtf32Test, three_bytes)
{
    int byte;
    EXPECT_EQ( 0x0800, MISC::utf8toutf32( "\xE0\xA0\x80", byte ) );
    EXPECT_EQ( 3, byte );
    EXPECT_EQ( 0xFFFF, MISC::utf8toutf32( "\xEF\xBF\xBF", byte ) );
    EXPECT_EQ( 3, byte );
}

TEST_F(Utf8ToUtf32Test, four_bytes)
{
    int byte;
    EXPECT_EQ( 0x00010000, MISC::utf8toutf32( "\xF0\x90\x80\x80", byte ) );
    EXPECT_EQ( 4, byte );
    EXPECT_EQ( 0x0010FFFF, MISC::utf8toutf32( "\xF4\x8F\xBF\xBF", byte ) );
    EXPECT_EQ( 4, byte );
}

TEST_F(Utf8ToUtf32Test, invalid_bytes)
{
    // 範囲外のうち一部の4バイトシーケンスは 0 にならない
    // Utf8BytesTest::obsolete_four_bytes を参照
    int byte;
    EXPECT_EQ( 0x00110000, MISC::utf8toutf32( "\xF4\x90\x80\x80", byte ) );
    EXPECT_EQ( 4, byte );
    EXPECT_EQ( 0x0013FFFF, MISC::utf8toutf32( "\xF4\xBF\xBF\xBF", byte ) );
    EXPECT_EQ( 4, byte );

    EXPECT_EQ( 0, MISC::utf8toutf32( "\xF5\x80\x80\x80", byte ) ); // U+140000
    EXPECT_EQ( 0, byte );
}


class Utf32ToUtf8Test : public ::testing::Test {};

TEST_F(Utf32ToUtf8Test, null_data)
{
    EXPECT_EQ( 0, MISC::utf32toutf8( 0x1000, nullptr ) );
}

TEST_F(Utf32ToUtf8Test, ascii)
{
    char out_char[8];
    EXPECT_EQ( 1, MISC::utf32toutf8( 0x0000, out_char ) );
    EXPECT_STREQ( "\u0000", out_char );
    EXPECT_EQ( 1, MISC::utf32toutf8( 0x007F, out_char ) );
    EXPECT_STREQ( "\u007F", out_char );
}

TEST_F(Utf32ToUtf8Test, two_bytes)
{
    char out_char[8];
    EXPECT_EQ( 2, MISC::utf32toutf8( 0x0080, out_char ) );
    EXPECT_STREQ( "\u0080", out_char );
    EXPECT_EQ( 2, MISC::utf32toutf8( 0x07FF, out_char ) );
    EXPECT_STREQ( "\u07FF", out_char );
}

TEST_F(Utf32ToUtf8Test, three_bytes)
{
    char out_char[8];
    EXPECT_EQ( 3, MISC::utf32toutf8( 0x0800, out_char ) );
    EXPECT_STREQ( "\u0800", out_char );
    EXPECT_EQ( 3, MISC::utf32toutf8( 0xFFFF, out_char ) );
    EXPECT_STREQ( "\uFFFF", out_char );
}

TEST_F(Utf32ToUtf8Test, four_bytes)
{
    char out_char[8];
    EXPECT_EQ( 4, MISC::utf32toutf8( 0x00010000, out_char ) );
    EXPECT_STREQ( "\U00010000", out_char );
    EXPECT_EQ( 4, MISC::utf32toutf8( 0x0010FFFF, out_char ) );
    EXPECT_STREQ( "\U0010FFFF", out_char );
}

TEST_F(Utf32ToUtf8Test, out_of_range)
{
    char out_char[8];
    EXPECT_EQ( 0, MISC::utf32toutf8( 0x00110000, out_char ) );
    EXPECT_STREQ( "", out_char );
}


class GetUnicodeBlockTest : public ::testing::Test {};

TEST_F(GetUnicodeBlockTest, basic_latin)
{
    EXPECT_EQ( MISC::UnicodeBlock::BasicLatin, MISC::get_unicodeblock( 0x0000 ) );
    EXPECT_EQ( MISC::UnicodeBlock::BasicLatin, MISC::get_unicodeblock( 0x007F ) );
}

TEST_F(GetUnicodeBlockTest, hiragana)
{
    EXPECT_EQ( MISC::UnicodeBlock::Hira, MISC::get_unicodeblock( 0x3040 ) );
    EXPECT_EQ( MISC::UnicodeBlock::Hira, MISC::get_unicodeblock( 0x309F ) );
}

TEST_F(GetUnicodeBlockTest, katanaka)
{
    EXPECT_EQ( MISC::UnicodeBlock::Kata, MISC::get_unicodeblock( 0x30A0 ) );
    EXPECT_EQ( MISC::UnicodeBlock::Kata, MISC::get_unicodeblock( 0x30FF ) );
}

TEST_F(GetUnicodeBlockTest, other)
{
    EXPECT_EQ( MISC::UnicodeBlock::Other, MISC::get_unicodeblock( 0x0080 ) );

    EXPECT_EQ( MISC::UnicodeBlock::Other, MISC::get_unicodeblock( 0x303F ) );
    EXPECT_EQ( MISC::UnicodeBlock::Other, MISC::get_unicodeblock( 0x3100 ) );

    EXPECT_EQ( MISC::UnicodeBlock::Other, MISC::get_unicodeblock( 0x10FFFF ) );
    EXPECT_EQ( MISC::UnicodeBlock::Other, MISC::get_unicodeblock( 0x110000 ) );
}


class Utf8FixWaveDashTest : public ::testing::Test {};

TEST_F(Utf8FixWaveDashTest, empty_data)
{
    EXPECT_EQ( "", MISC::utf8_fix_wavedash( "", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "", MISC::utf8_fix_wavedash( "", MISC::WaveDashFix::WinToUnix ) );
}

TEST_F(Utf8FixWaveDashTest, not_fix)
{
    EXPECT_EQ( "Hello World", MISC::utf8_fix_wavedash( "Hello World", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "いろはにほへ", MISC::utf8_fix_wavedash( "いろはにほへ", MISC::WaveDashFix::WinToUnix ) );
}

TEST_F(Utf8FixWaveDashTest, fix_unix_to_win)
{
    EXPECT_EQ( "\uFF5E+a", MISC::utf8_fix_wavedash( "\u301C+a", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "\u2015-b", MISC::utf8_fix_wavedash( "\u2014-b", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "\u2225*c", MISC::utf8_fix_wavedash( "\u2016*c", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "\uFF0D/d", MISC::utf8_fix_wavedash( "\u2212/d", MISC::WaveDashFix::UnixToWin ) );
}

TEST_F(Utf8FixWaveDashTest, fix_win_to_unix)
{
    EXPECT_EQ( "a+\u301C", MISC::utf8_fix_wavedash( "a+\uFF5E", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "b-\u2014", MISC::utf8_fix_wavedash( "b-\u2015", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "c*\u2016", MISC::utf8_fix_wavedash( "c*\u2225", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "d/\u2212", MISC::utf8_fix_wavedash( "d/\uFF0D", MISC::WaveDashFix::WinToUnix ) );
}

TEST_F(Utf8FixWaveDashTest, not_fix_unix_to_win)
{
    EXPECT_EQ( "a+\uFF5E", MISC::utf8_fix_wavedash( "a+\uFF5E", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "b-\u2015", MISC::utf8_fix_wavedash( "b-\u2015", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "c*\u2225", MISC::utf8_fix_wavedash( "c*\u2225", MISC::WaveDashFix::UnixToWin ) );
    EXPECT_EQ( "d/\uFF0D", MISC::utf8_fix_wavedash( "d/\uFF0D", MISC::WaveDashFix::UnixToWin ) );
}

TEST_F(Utf8FixWaveDashTest, not_fix_win_to_unix)
{
    EXPECT_EQ( "\u301C+a", MISC::utf8_fix_wavedash( "\u301C+a", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "\u2014-b", MISC::utf8_fix_wavedash( "\u2014-b", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "\u2016*c", MISC::utf8_fix_wavedash( "\u2016*c", MISC::WaveDashFix::WinToUnix ) );
    EXPECT_EQ( "\u2212/d", MISC::utf8_fix_wavedash( "\u2212/d", MISC::WaveDashFix::WinToUnix ) );
}

} // namespace
