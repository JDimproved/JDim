// SPDX-License-Identifier: GPL-2.0-only

#include "jdlib/misccharcode.h"

#include "gtest/gtest.h"


namespace {

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

} // namespace
