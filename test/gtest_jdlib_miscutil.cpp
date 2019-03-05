// License: GPL2

#include "jdlib/miscutil.h"

#include "gtest/gtest.h"


namespace {

class SplitLineTest : public ::testing::Test {};

TEST_F(SplitLineTest, split_empty)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"" ) );
}

TEST_F(SplitLineTest, split_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"    " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8" the quick  brown   fox  " ) );
}

TEST_F(SplitLineTest, split_U_3000)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000 \u3000 " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000the\u3000quick \u3000brown\u3000 \u3000fox\u3000 " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"  \"\"  " ) );

    expect.assign( { u8"the quick", u8" ", u8" brown   fox " } );
    EXPECT_EQ( expect, MISC::split_line( u8" \"the quick\" \" \" \" brown   fox \" " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_3000)
{
    std::list< std::string > expect = { u8"the\u3000quick", u8"\u3000", u8"\u3000brown \u3000fox\u3000" };
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000\"the\u3000quick\" \"\u3000\" \"\u3000brown \u3000fox\u3000\"" ) );
}


class RemoveSpaceTest : public ::testing::Test {};

TEST_F(RemoveSpaceTest, remove_empty)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"" ) );
}

TEST_F(RemoveSpaceTest, remove_U_0020)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"    " ) );

    expect.assign( u8"the quick  brown   fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8" the quick  brown   fox  " ) );
}

TEST_F(RemoveSpaceTest, remove_U_3000)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \u3000 " ) );

    expect.assign( u8"the quick\u3000brown\u3000 fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000the quick\u3000brown\u3000 fox\u3000 " ) );
}

TEST_F(RemoveSpaceTest, remove_doublequote)
{
    std::string expect = u8"\"\"";
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \"\"\u3000 " ) );
}


class IsUrlSchemeTest : public ::testing::Test {};

TEST_F(IsUrlSchemeTest, url_none)
{
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "foo" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "http:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ttp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "tp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ftp:/" ) );
    // "sssp:/" はバッファオーバーランを起こす
}

TEST_F(IsUrlSchemeTest, url_http)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "http://", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "http://foobar", &length ) );
    EXPECT_EQ( 7, length );
}

TEST_F(IsUrlSchemeTest, url_ttp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TTP, MISC::is_url_scheme( "ttp://", &length ) );
    EXPECT_EQ( 6, length );

    EXPECT_EQ( MISC::SCHEME_TTP, MISC::is_url_scheme( "ttp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_tp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TP, MISC::is_url_scheme( "tp://", &length ) );
    EXPECT_EQ( 5, length );

    EXPECT_EQ( MISC::SCHEME_TP, MISC::is_url_scheme( "tp://foobar", &length ) );
    EXPECT_EQ( 5, length );
}

TEST_F(IsUrlSchemeTest, url_ftp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_FTP, MISC::is_url_scheme( "ftp://", &length ) );
    EXPECT_EQ( 6, length );

    EXPECT_EQ( MISC::SCHEME_FTP, MISC::is_url_scheme( "ftp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_sssp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://foobar", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_SSSP, MISC::is_url_scheme( "sssp://img.2ch", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://img.5ch", &length ) );
    EXPECT_EQ( 7, length );
}



class MISC_AscTest : public ::testing::Test {};

TEST_F(MISC_AscTest, empty_input)
{
    std::string output;
    std::vector<int> table;

    // 入力はヌル終端文字列
    MISC::asc( u8"", output, table );

    EXPECT_EQ( u8"", output );
    EXPECT_EQ( 0, output.size() );
    // 文字列の終端（ヌル文字）の位置が追加されるためtableのサイズが+1大きくなる
    EXPECT_EQ( 1, table.size() );
    EXPECT_EQ( 0, table.at( 0 ) );
}

TEST_F(MISC_AscTest, halfwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output, table );

    EXPECT_EQ( u8"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"the quick brown fox jumps over the lazy dog.", output, table );

    EXPECT_EQ( u8"the quick brown fox jumps over the lazy dog.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"1234567890+-*/", output, table );

    EXPECT_EQ( u8"1234567890+-*/", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_append_data)
{
    std::string output = "123";
    std::vector<int> table = { 0, 1, 2 };

    // アウトプット引数は初期化せずデータを追加する
    MISC::asc( u8"hello", output, table );

    EXPECT_EQ( u8"123hello", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = { 0, 1, 2, 0, 1, 2, 3, 4, 5 };
    EXPECT_EQ( expected_table, table );
}


std::vector<int> expected_table_fullwidth_quick_brown_fox()
{
    // 全角英数字から半角英数字に変換したときの文字列の位置を保存しておくテーブルのテストデータ
    return {
        // TH E  U+3000     Q   U   I   C   K   U+3000      B   R   O   W   N   U+3000      F   O   X   U+3000
        0, 3, 6, 9, 10, 11, 12, 15, 18, 21, 24, 27, 28, 29, 30, 33, 36, 39, 42, 45, 46, 47, 48, 51, 54, 57, 58, 59,
        // JU   M   P   S   U+3000      O   V   E   R   U+3000      T   H   E   U+3000         L    A    Z    Y
        60, 63, 66, 69, 72, 75, 76, 77, 78, 81, 84, 87, 90, 91, 92, 93, 96, 99, 102, 103, 104, 105, 108, 111, 114,
        // U+3000      D    O    G    U+FF0E         U+0000
        117, 118, 119, 120, 123, 126, 129, 130, 131, 132,
    };
}

TEST_F(MISC_AscTest, fullwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"ＴＨＥ　ＱＵＩＣＫ　ＢＲＯＷＮ　ＦＯＸ　ＪＵＭＰＳ　ＯＶＥＲ　ＴＨＥ　ＬＡＺＹ　ＤＯＧ．", output,
               table );

    // 和字間隔(U+3000)は半角スペースに変換されない
    EXPECT_EQ( u8"THE　QUICK　BROWN　FOX　JUMPS　OVER　THE　LAZY　DOG．", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_AscTest, fullwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ．", output,
               table );

    EXPECT_EQ( u8"the　quick　brown　fox　jumps　over　the　lazy　dog．", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_AscTest, fullwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( u8"１２３４５６７８９０＋−＊／", output, table );

    // 全角数字は半角に変換されるが、全角記号は半角に変換されない
    EXPECT_EQ( u8"1234567890＋−＊／", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
        30, 31, 32,  33, 34, 35,  36, 37, 38,  39, 40, 41,  42
    };
    EXPECT_EQ( expected_table, table );
}


TEST_F(MISC_AscTest, halfwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] {
        u8"\uFF61\uFF62\uFF63\uFF64\uFF65" u8"\uFF66" u8"\uFF67\uFF68\uFF69\uFF6A\uFF6B"
        u8"\uFF6C\uFF6D\uFF6E\uFF6F\uFF70" u8"\uFF71\uFF72\uFF73\uFF74\uFF75" u8"\uFF76\uFF77\uFF78\uFF79\uFF7A"
        u8"\uFF7B\uFF7C\uFF7D\uFF7E\uFF7F" u8"\uFF80\uFF81\uFF82\uFF83\uFF84" u8"\uFF85\uFF86\uFF87\uFF88\uFF89"
        u8"\uFF8A\uFF8B\uFF8C\uFF8D\uFF8E" u8"\uFF8F\uFF90\uFF91\uFF92\uFF93" u8"\uFF94\uFF95\uFF96"
        u8"\uFF97\uFF98\uFF99\uFF9A\uFF9B" u8"\uFF9C\uFF9D"
    };
    constexpr const char fullwidth[] {
        u8"。「」、・" u8"ヲ" u8"ァィゥェォ"
        u8"ャュョッー" u8"アイウエオ" u8"カキクケコ"
        u8"サシスセソ" u8"タチツテト" u8"ナニヌネノ"
        u8"ハヒフヘホ" u8"マミムメモ" u8"ヤユヨ"
        u8"ラリルレロ" u8"ワン"
    };

    // 半角片仮名から全角片仮名へ一対一の変換
    MISC::asc( halfwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_katakana_only_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;

    // 半角の濁点と半濁点は単独では全角に変換されない
    MISC::asc( u8"\uFF9E\uFF9F", output, table );

    EXPECT_EQ( u8"\uFF9E\uFF9F", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_to_precomposed)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] = {
        u8"\uFF76\uFF9E\uFF77\uFF9E\uFF78\uFF9E\uFF79\uFF9E\uFF7A\uFF9E"
        u8"\uFF7B\uFF9E\uFF7C\uFF9E\uFF7D\uFF9E\uFF7E\uFF9E\uFF7F\uFF9E"
        u8"\uFF80\uFF9E\uFF81\uFF9E\uFF82\uFF9E\uFF83\uFF9E\uFF84\uFF9E"
        u8"\uFF8A\uFF9E\uFF8B\uFF9E\uFF8C\uFF9E\uFF8D\uFF9E\uFF8E\uFF9E"
        u8"\uFF8A\uFF9F\uFF8B\uFF9F\uFF8C\uFF9F\uFF8D\uFF9F\uFF8E\uFF9F"
    };
    constexpr const char fullwidth[] {
        u8"\u30AC\u30AE\u30B0\u30B2\u30B4" // ガギグゲゴ
        u8"\u30B6\u30B8\u30BA\u30BC\u30BE" // ザジズゼゾ
        u8"\u30C0\u30C2\u30C5\u30C7\u30C9" // ダヂヅデド
        u8"\u30D0\u30D3\u30D6\u30D9\u30DC" // バビブベボ
        u8"\u30D1\u30D4\u30D7\u30DA\u30DD" // パピプペポ
    };

    // 合成済み文字が存在する(半)濁点付き半角片仮名は全角へ変換される
    MISC::asc( halfwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26,
        30, 31, 32, 36, 37, 38, 42, 43, 44, 48, 49, 50, 54, 55, 56,
        60, 61, 62, 66, 67, 68, 72, 73, 74, 78, 79, 80, 84, 85, 86,
        90, 91, 92, 96, 97, 98, 102, 103, 104, 108, 109, 110, 114, 115, 116,
        120, 121, 122, 126, 127, 128, 132, 133, 134, 138, 139, 140, 144, 145, 146,
        150,
    };
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_wagyo)
{
    std::string output;
    std::vector<int> table;

    // 濁点付き半角片仮名のウは全角の合成済み文字に変換される：ヴ
    // ワ行の濁点付き半角片仮名は全角に変換されない(変換表が未実装) : ヷヺ
    MISC::asc( u8"\uFF73\uFF9E\uFF9C\uFF9E\uFF66\uFF9E", output, table );

    EXPECT_EQ( u8"\u30F4\uFF9C\uFF9E\uFF66\uFF9E", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 1, 2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_through)
{
    std::string output;
    std::vector<int> table;

    // 合成済み文字が存在しない(半)濁点付き半角片仮名は全角に変換されない : カ゚キ゚ク゚ケ゚コ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( u8"\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", output, table );

    EXPECT_EQ( u8"\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

} // namespace
