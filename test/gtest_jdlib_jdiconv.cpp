// License: GPL2

#include "jdencoding.h"
#include "jdlib/jdiconv.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

// エンコーディング変換は無数の組み合わせがあるためテストケースを網羅できない
// JDim側で特別な処理をするパターンについてテストする

class Iconv_ToAsciiFromUtf8 : public ::testing::Test {};

TEST_F(Iconv_ToAsciiFromUtf8, empty)
{
    char input[] = "";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::ascii, Encoding::utf8, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "", result );
    EXPECT_EQ( 0, result.size() );
}

TEST_F(Iconv_ToAsciiFromUtf8, helloworld)
{
    char input[] = "hello world!\n";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::ascii, Encoding::utf8, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "hello world!\n", result );
    EXPECT_EQ( 13, result.size() );
}

TEST_F(Iconv_ToAsciiFromUtf8, hiragana)
{
    char input[] = "あいうえお";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::ascii, Encoding::utf8, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "&#12354;&#12356;&#12358;&#12360;&#12362;", result );
    EXPECT_EQ( 40, result.size() );
}

TEST_F(Iconv_ToAsciiFromUtf8, subdivision_flag)
{
    // :england: JDLIB::Iconv::convert()のコメントを参照
    char input[] = "\U0001F3F4\U000E0067\U000E0062\U000E0065\U000E006E\U000E0067\U000E007F";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::ascii, Encoding::utf8, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "&#127988;&#917607;&#917602;&#917605;&#917614;&#917607;&#917631;", result );
    EXPECT_EQ( 63, result.size() );
}


class Iconv_ToUtf8FromAscii : public ::testing::Test {};

TEST_F(Iconv_ToUtf8FromAscii, replacement_character_to_utf8_is_uFFFD)
{
    // テストは網羅してない
    char input[] = "\x80\x91\xA2\xB3\xC4\xD5\xE6\xF7";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::ascii, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    // UTF-8へ変換するとき入力エンコーディングで無効なバイト列は U+FFFD に置き換える
    EXPECT_EQ( "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"
               "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD", result );
    EXPECT_EQ( 24, result.size() );
}


class Iconv_ToUtf8FromMs932 : public ::testing::Test {};

TEST_F(Iconv_ToUtf8FromMs932, empty)
{
    char input[] = "";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "", result );
    EXPECT_EQ( 0, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, helloworld)
{
    char input[] = "hello world!\n";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "hello world!\n", result );
    EXPECT_EQ( 13, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, hiragana)
{
    char input[] = "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "あいうえお", result );
    EXPECT_EQ( 15, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, hex_a0)
{
    char input[] = "hello\xa0world!\n";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "hello world!\n", result );
    EXPECT_EQ( 13, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, mojibake_fix_inequality_sign_pattern1)
{
    // DATのデータ区切り <> が文字化けするとスレが壊れるため変換を修正する
    // エンコーディングがMS932のスレにUTF-8で書き込み文字化けした場合をテスト
    char input[] = "<>test テスト<>";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "<>test \xE7\xB9\x9D\xE2\x96\xA1\xE3\x81\x9B\xE7\xB9\x9D?<>", result );
    EXPECT_EQ( 22, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, mojibake_fix_inequality_sign_pattern2)
{
    // DATのデータ区切り <> が文字化けするとスレが壊れるため変換を修正する
    // エンコーディングがMS932のスレにUTF-8で書き込み文字化けした場合をテスト
    char input[] = "<> test テスト <>";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "<> test \xE7\xB9\x9D\xE2\x96\xA1\xE3\x81\x9B\xE7\xB9\x9D\xE2\x96\xA1<>", result );
    EXPECT_EQ( 25, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, mapping_error)
{
    // MS932の符号として不正な2バイトコードは白四角(\x81\A0 == U+25A1)として処理する
    // テストは網羅してない
    char input[] = "\x81\xAD\x82\x40\x88\x90\x98\x90";
    constexpr bool broken_sjis_be_utf8 = false;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "\u25A1\u25A1\u25A1\u25A1", result );
    EXPECT_EQ( 12, result.size() );
}

TEST_F(Iconv_ToUtf8FromMs932, broken_sjis_be_utf8)
{
    char input[] = "\x82\xa0\x82\xa2\x82\xa4 "
                   "\xe5\x85\xa5\xe3\x82\x8c\xe6\x9b\xbf\xe3\x82\x8f"
                   "\xe3\x81\xa3\xe3\x81\xa6\xe3\x82\x8b\xe3\x80\x9c?"
                   " \x82\xa6\x82\xa8";
    constexpr bool broken_sjis_be_utf8 = true;

    JDLIB::Iconv icv( Encoding::utf8, Encoding::sjis, broken_sjis_be_utf8 );
    const std::string& result = icv.convert( input, std::strlen(input) );

    EXPECT_EQ( "あいう <span class=\"BROKEN_SJIS\">入れ替わってる〜</span>? えお", result );
    EXPECT_EQ( 28 - 2 + 7 + 9 * 3 + 5 * 3, result.size() );
}

} // namespace
