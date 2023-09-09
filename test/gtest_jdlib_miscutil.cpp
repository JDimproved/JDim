// License: GPL2

#include "jdlib/miscutil.h"

#include "gtest/gtest.h"

#include <numeric> // std::iota()


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


class ConcatWithSuffixTest : public ::testing::Test {};

TEST_F(ConcatWithSuffixTest, empty_list)
{
    std::list<std::string> list_in;
    EXPECT_EQ( "", MISC::concat_with_suffix( list_in, '!' ) );
}

TEST_F(ConcatWithSuffixTest, one_element)
{
    std::list<std::string> list_in = { "hello" };
    EXPECT_EQ( "hello!", MISC::concat_with_suffix( list_in, '!' ) );
}

TEST_F(ConcatWithSuffixTest, hello_world)
{
    std::list<std::string> list_in = { "hello", "world" };
    EXPECT_EQ( "hello!world!", MISC::concat_with_suffix( list_in, '!' ) );
}

TEST_F(ConcatWithSuffixTest, ignore_empty_string)
{
    std::list<std::string> list_in = { "", "hello", "", "", "world", "" };
    EXPECT_EQ( "hello!world!", MISC::concat_with_suffix( list_in, '!' ) );
}


class Utf8TrimTest : public ::testing::Test {};

TEST_F(Utf8TrimTest, remove_empty)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::utf8_trim( "" ) );
}

TEST_F(Utf8TrimTest, remove_U_0020)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::utf8_trim( "    " ) );

    expect.assign( "the quick  brown   fox" );
    EXPECT_EQ( expect, MISC::utf8_trim( " the quick  brown   fox  " ) );
}

TEST_F(Utf8TrimTest, remove_mixed_U_2000_U_3000)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::utf8_trim( "\u3000 \u3000 " ) );

    expect.assign( "the quick\u3000brown\u3000 fox" );
    EXPECT_EQ( expect, MISC::utf8_trim( "\u3000the quick\u3000brown\u3000 fox\u3000 " ) );
}

TEST_F(Utf8TrimTest, not_remove_U_3000_only)
{
    // 半角スペースが含まれてないときはU+3000が先頭末尾にあってもトリミングしない
    std::string expect = "\u3000\u3000";
    EXPECT_EQ( expect, MISC::utf8_trim( "\u3000\u3000" ) );

    expect.assign( "\u3000the\u3000quick\u3000brown\u3000fox\u3000" );
    EXPECT_EQ( expect, MISC::utf8_trim( expect ) );
}

TEST_F(Utf8TrimTest, remove_doublequote)
{
    std::string expect = "\"\"";
    EXPECT_EQ( expect, MISC::utf8_trim( "\u3000 \"\"\u3000 " ) );
}

TEST_F(Utf8TrimTest, input_length_is_shorter_than_u3000)
{
    std::string expect = "a";
    EXPECT_EQ( expect, MISC::utf8_trim( " a" ) );

    expect = "b";
    EXPECT_EQ( expect, MISC::utf8_trim( "b " ) );
}


class AsciiTrimTest : public ::testing::Test {};

TEST_F(AsciiTrimTest, empty_data)
{
    EXPECT_EQ( "", MISC::ascii_trim( "" ) );
}

TEST_F(AsciiTrimTest, multiple_whitespace_only_data)
{
	EXPECT_EQ( "", MISC::ascii_trim("  ") );
}

TEST_F(AsciiTrimTest, no_space_chars_at_start_and_end)
{
    EXPECT_EQ( "Hello \n \r \t World", MISC::ascii_trim( "Hello \n \r \t World" ) );
    EXPECT_EQ( "あいうえお", MISC::ascii_trim( "あいうえお" ) );
}

TEST_F(AsciiTrimTest, trim_front)
{
    EXPECT_EQ( "Hello", MISC::ascii_trim( " Hello" ) );
    EXPECT_EQ( "Hello", MISC::ascii_trim( "\nHello" ) );
    EXPECT_EQ( "Hello", MISC::ascii_trim( "\rHello" ) );
    EXPECT_EQ( "Hello", MISC::ascii_trim( "\tHello" ) );
    EXPECT_EQ( "Hello", MISC::ascii_trim( "\n \r \t Hello" ) );
}

TEST_F(AsciiTrimTest, trim_back)
{
    EXPECT_EQ( "World", MISC::ascii_trim( "World " ) );
    EXPECT_EQ( "World", MISC::ascii_trim( "World\n" ) );
    EXPECT_EQ( "World", MISC::ascii_trim( "World\r" ) );
    EXPECT_EQ( "World", MISC::ascii_trim( "World\t" ) );
    EXPECT_EQ( "World", MISC::ascii_trim( "World\n \r \t" ) );
}

TEST_F(AsciiTrimTest, trim_both_side)
{
    EXPECT_EQ( "Hello\t \n \rWorld", MISC::ascii_trim( "\n \r \t Hello\t \n \rWorld \n \r \t" ) );
}

TEST_F(AsciiTrimTest, not_trim_ascii)
{
    EXPECT_EQ( "\vHello\v", MISC::ascii_trim( "\vHello\v" ) ); // VERTICAL TAB
    EXPECT_EQ( "\fHello\f", MISC::ascii_trim( "\fHello\f" ) ); // FORM FEED
}

TEST_F(AsciiTrimTest, not_trim_unicode)
{
    EXPECT_EQ( "\u00A0Hello\u00A0", MISC::ascii_trim( "\u00A0Hello\u00A0" ) ); // NO-BREAK SPACE
    EXPECT_EQ( "\u3000Hello\u3000", MISC::ascii_trim( "\u3000Hello\u3000" ) ); // IDEOGRAPHIC SPACE
}


class RemoveStrStartEndTest : public ::testing::Test {};

TEST_F(RemoveStrStartEndTest, empty_data)
{
    EXPECT_EQ( "", MISC::remove_str( "", "", "" ) );
    EXPECT_EQ( "", MISC::remove_str( "", "<<", "" ) );
    EXPECT_EQ( "", MISC::remove_str( "", "<<", ">>" ) );
    EXPECT_EQ( "", MISC::remove_str( "", "", ">>" ) );
}

TEST_F(RemoveStrStartEndTest, empty_start)
{
    EXPECT_EQ( "Quick<<Brown>>Fox", MISC::remove_str( "Quick<<Brown>>Fox", "", ">>" ) );
}

TEST_F(RemoveStrStartEndTest, empty_end)
{
    EXPECT_EQ( "Quick<<Brown>>Fox", MISC::remove_str( "Quick<<Brown>>Fox", "<<", "" ) );
}

TEST_F(RemoveStrStartEndTest, different_marks)
{
    EXPECT_EQ( "QuickFox", MISC::remove_str( "Quick<<Brown>>Fox", "<<", ">>" ) );
}

TEST_F(RemoveStrStartEndTest, same_marks)
{
    EXPECT_EQ( "QuickFox", MISC::remove_str( "Quick!!Brown!!Fox", "!!", "!!" ) );
}

TEST_F(RemoveStrStartEndTest, much_start_marks)
{
    EXPECT_EQ( "TheFox", MISC::remove_str( "The(Quick(Brown)Fox", "(", ")" ) );
}

TEST_F(RemoveStrStartEndTest, much_end_marks)
{
    EXPECT_EQ( "TheBrown)Fox", MISC::remove_str( "The(Quick)Brown)Fox", "(", ")" ) );
}


class CutStrFrontBackTest : public ::testing::Test {};

TEST_F(CutStrFrontBackTest, empty_data)
{
    EXPECT_EQ( "", MISC::cut_str( "", "", "" ) );
    EXPECT_EQ( "", MISC::cut_str( "", "AA", "" ) );
    EXPECT_EQ( "", MISC::cut_str( "", "AA", "BB" ) );
    EXPECT_EQ( "", MISC::cut_str( "", "", "BB" ) );
}

TEST_F(CutStrFrontBackTest, empty_front_separator)
{
    EXPECT_EQ( "", MISC::cut_str( "Quick<<Brown>>Fox", "", ">>" ) );
}

TEST_F(CutStrFrontBackTest, empty_back_separator)
{
    EXPECT_EQ( "", MISC::cut_str( "Quick<<Brown>>Fox", "<<", "" ) );
}

TEST_F(CutStrFrontBackTest, different_separators)
{
    EXPECT_EQ( "Brown", MISC::cut_str( "Quick<<Brown>>Fox", "<<", ">>" ) );
}

TEST_F(CutStrFrontBackTest, same_separators)
{
    EXPECT_EQ( "Brown", MISC::cut_str( "Quick!!Brown!!Fox", "!!", "!!" ) );
}

TEST_F(CutStrFrontBackTest, much_front_separators)
{
    EXPECT_EQ( "Quick(Brown", MISC::cut_str( "The(Quick(Brown)Fox", "(", ")" ) );
}

TEST_F(CutStrFrontBackTest, much_back_separators)
{
    EXPECT_EQ( "Quick", MISC::cut_str( "The(Quick)Brown)Fox", "(", ")" ) );
}


class ReplaceStrTest : public ::testing::Test {};

TEST_F(ReplaceStrTest, empty_data)
{
    EXPECT_EQ( "", MISC::replace_str( "", "", "" ) );
    EXPECT_EQ( "", MISC::replace_str( "", "AA", "" ) );
    EXPECT_EQ( "", MISC::replace_str( "", "AA", "BB" ) );
    EXPECT_EQ( "", MISC::replace_str( "", "", "BB" ) );
}

TEST_F(ReplaceStrTest, empty_match)
{
    EXPECT_EQ( "Quick Brown Fox", MISC::replace_str( "Quick Brown Fox", "", "Red" ) );
}

TEST_F(ReplaceStrTest, replace_with_empty)
{
    EXPECT_EQ( "Quick//Fox", MISC::replace_str( "Quick/Brown/Fox", "Brown", "" ) );
}

TEST_F(ReplaceStrTest, not_match)
{
    EXPECT_EQ( "Quick Brown Fox", MISC::replace_str( "Quick Brown Fox", "Red", "Blue" ) );
}

TEST_F(ReplaceStrTest, multi_match)
{
    EXPECT_EQ( "Quick Red Red Fox", MISC::replace_str( "Quick Brown Brown Fox", "Brown", "Red" ) );
}


class ReplaceCaseStrTest : public ::testing::Test {};

TEST_F(ReplaceCaseStrTest, empty_data)
{
    EXPECT_EQ( "", MISC::replace_casestr( "", "", "" ) );
    EXPECT_EQ( "", MISC::replace_casestr( "", "AA", "" ) );
    EXPECT_EQ( "", MISC::replace_casestr( "", "AA", "BB" ) );
    EXPECT_EQ( "", MISC::replace_casestr( "", "", "BB" ) );
}

TEST_F(ReplaceCaseStrTest, empty_match)
{
    EXPECT_EQ( "Quick Brown Fox", MISC::replace_casestr( "Quick Brown Fox", "", "Red" ) );
}

TEST_F(ReplaceCaseStrTest, replace_with_empty)
{
    EXPECT_EQ( "Quick//Fox", MISC::replace_casestr( "Quick/Brown/Fox", "Brown", "" ) );
}

TEST_F(ReplaceCaseStrTest, replace_with_empty_ignore_case)
{
    EXPECT_EQ( "Quick//Fox", MISC::replace_casestr( "Quick/BrOwN/Fox", "bRoWn", "" ) );
}

TEST_F(ReplaceCaseStrTest, not_match)
{
    EXPECT_EQ( "Quick Brown Fox", MISC::replace_casestr( "Quick Brown Fox", "Red", "Blue" ) );
}

TEST_F(ReplaceCaseStrTest, multi_match)
{
    EXPECT_EQ( "Quick Red Red Fox", MISC::replace_casestr( "Quick Brown Brown Fox", "Brown", "Red" ) );
}

TEST_F(ReplaceCaseStrTest, multi_match_ignore_case)
{
    EXPECT_EQ( "Quick Red Red Fox", MISC::replace_casestr( "Quick BrOwN bRoWn Fox", "BRowN", "Red" ) );
}


class ReplaceStrListTest : public ::testing::Test {};

TEST_F(ReplaceStrListTest, empty_data)
{
    std::list<std::string> empty;
    std::list<std::string> expect;
    EXPECT_EQ( expect, MISC::replace_str_list( empty, "AA", "BB" ) );
}

TEST_F(ReplaceStrListTest, sample_match)
{
    std::list<std::string> input = { "hello", "world", "sample" };
    std::list<std::string> expect = { "hell123", "w123rld", "sample" };
    EXPECT_EQ( expect, MISC::replace_str_list( input, "o", "123" ) );
}


class ReplaceNewlinesToStrTest : public ::testing::Test {};

TEST_F(ReplaceNewlinesToStrTest, empty_data)
{
    EXPECT_EQ( "", MISC::replace_newlines_to_str( "", "" ) );
    EXPECT_EQ( "", MISC::replace_newlines_to_str( "", "A\nA" ) );
}

TEST_F(ReplaceNewlinesToStrTest, empty_replacement)
{
    EXPECT_EQ( "\nBrown\nFox\n", MISC::replace_newlines_to_str( "\nBrown\nFox\n", "" ) );
    EXPECT_EQ( "\rBrown\rFox\r", MISC::replace_newlines_to_str( "\rBrown\rFox\r", "" ) );
    EXPECT_EQ( "\r\nBrown\r\nFox\r\n", MISC::replace_newlines_to_str( "\r\nBrown\r\nFox\r\n", "" ) );
}

TEST_F(ReplaceNewlinesToStrTest, replace_cr)
{
    EXPECT_EQ( "!!Brown!!Fox!!", MISC::replace_newlines_to_str( "\rBrown\rFox\r", "!!" ) );
}

TEST_F(ReplaceNewlinesToStrTest, replace_lf)
{
    EXPECT_EQ( "!!Brown!!Fox!!", MISC::replace_newlines_to_str( "\nBrown\nFox\n", "!!" ) );
}

TEST_F(ReplaceNewlinesToStrTest, replace_crlf)
{
    EXPECT_EQ( "!!Brown!!Fox!!", MISC::replace_newlines_to_str( "\r\nBrown\r\nFox\r\n", "!!" ) );
}


class ChrToBinTest : public ::testing::Test {};

TEST_F(ChrToBinTest, empty_input)
{
    char output[4]{};
    EXPECT_EQ( 0, MISC::chrtobin( "", output ) );
}

TEST_F(ChrToBinTest, fullwidth_input)
{
    char output[4]{};
    EXPECT_EQ( 0, MISC::chrtobin( "ＡＢ", output ) );
    EXPECT_EQ( 0, MISC::chrtobin( "１２", output ) );
}

TEST_F(ChrToBinTest, non_ascii_input)
{
    char output[4]{};
    EXPECT_EQ( 0, MISC::chrtobin( "あい", output ) );
    EXPECT_EQ( 0, MISC::chrtobin( "アイ", output ) );
}

TEST_F(ChrToBinTest, non_hexadecimal_input)
{
    char output[4]{};
    EXPECT_EQ( 0, MISC::chrtobin( "GH", output ) );
    EXPECT_EQ( 0, MISC::chrtobin( "gh", output ) );
    EXPECT_EQ( 0, MISC::chrtobin( " !", output ) );
}

TEST_F(ChrToBinTest, hexadecimal_input)
{
    char output[16];

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 10, MISC::chrtobin( "0123456789", output ) );
    EXPECT_STREQ( "\x01\x23\x45\x67\x89", output );

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 6, MISC::chrtobin( "ABCDEF", output ) );
    EXPECT_STREQ( "\xAB\xCD\xEF", output );
}

TEST_F(ChrToBinTest, hexadecimal_incomplete_input)
{
    char output[16];

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 3, MISC::chrtobin( "123", output ) );
    EXPECT_STREQ( "\x12\x03", output );

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 3, MISC::chrtobin( "ABC", output ) );
    EXPECT_STREQ( "\xAB\x0C", output );
}

TEST_F(ChrToBinTest, break_at_non_hexadecimal)
{
    char output[16];

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 4, MISC::chrtobin( "1234あFFFF", output ) );
    EXPECT_STREQ( "\x12\x34", output );

    std::memset( output, '\0', 16 );
    EXPECT_EQ( 3, MISC::chrtobin( "ABC FFFF", output ) );
    EXPECT_STREQ( "\xAB\xC0", output );
}


class HtmlEscapeTest : public ::testing::Test {};

TEST_F(HtmlEscapeTest, empty_data)
{
    EXPECT_EQ( "", MISC::html_escape( "", false ) );
    EXPECT_EQ( "", MISC::html_escape( "", true ) );
}

TEST_F(HtmlEscapeTest, not_escape)
{
    EXPECT_EQ( "hello world", MISC::html_escape( "hello world", false ) );
    EXPECT_EQ( "hello world", MISC::html_escape( "hello world", true ) );
}

TEST_F(HtmlEscapeTest, escape_amp)
{
    EXPECT_EQ( "hello&amp;world", MISC::html_escape( "hello&world", false ) );
    EXPECT_EQ( "hello&amp;world", MISC::html_escape( "hello&world", true ) );
}

TEST_F(HtmlEscapeTest, escape_quot)
{
    EXPECT_EQ( "hello&quot;world", MISC::html_escape( "hello\"world", false ) );
    EXPECT_EQ( "hello&quot;world", MISC::html_escape( "hello\"world", true ) );
}

TEST_F(HtmlEscapeTest, escape_lt)
{
    EXPECT_EQ( "hello&lt;world", MISC::html_escape( "hello<world", false ) );
    EXPECT_EQ( "hello&lt;world", MISC::html_escape( "hello<world", true ) );
}

TEST_F(HtmlEscapeTest, escape_gt)
{
    EXPECT_EQ( "hello&gt;world", MISC::html_escape( "hello>world", false ) );
    EXPECT_EQ( "hello&gt;world", MISC::html_escape( "hello>world", true ) );
}

TEST_F(HtmlEscapeTest, completely)
{
    // URLを含むテキスト
    const std::string input = "& https://foobar.test/?a=b&c=d& &";
    EXPECT_EQ( "&amp; https://foobar.test/?a=b&amp;c=d&amp; &amp;", MISC::html_escape( input, true ) );
    EXPECT_EQ( "&amp; https://foobar.test/?a=b&c=d& &amp;", MISC::html_escape( input, false ) );
}


class HtmlUnescapeTest : public ::testing::Test {};

TEST_F(HtmlUnescapeTest, empty_data)
{
    EXPECT_EQ( "", MISC::html_unescape( "" ) );
    EXPECT_EQ( "", MISC::html_unescape( "" ) );
}

TEST_F(HtmlUnescapeTest, not_escape)
{
    EXPECT_EQ( "quick brown fox", MISC::html_unescape( "quick brown fox" ) );
    EXPECT_EQ( "quick brown fox", MISC::html_unescape( "quick brown fox" ) );
}

TEST_F(HtmlUnescapeTest, escape_amp)
{
    EXPECT_EQ( "quick&brown&fox", MISC::html_unescape( "quick&amp;brown&amp;fox" ) );
}

TEST_F(HtmlUnescapeTest, escape_quot)
{
    EXPECT_EQ( "quick\"brown\"fox", MISC::html_unescape( "quick&quot;brown&quot;fox" ) );
}

TEST_F(HtmlUnescapeTest, escape_lt)
{
    EXPECT_EQ( "quick<brown<fox", MISC::html_unescape( "quick&lt;brown&lt;fox" ) );
}

TEST_F(HtmlUnescapeTest, escape_gt)
{
    EXPECT_EQ( "quick>brown>fox", MISC::html_unescape( "quick&gt;brown&gt;fox" ) );
}

TEST_F(HtmlUnescapeTest, numeric_char_reference)
{
    EXPECT_EQ( "quick&#123;brown&#234;fox", MISC::html_unescape( "quick&#123;brown&#234;fox" ) );
    EXPECT_EQ( "quick&#xabcd;brown&#xEF01;fox", MISC::html_unescape( "quick&#xabcd;brown&#xEF01;fox" ) );
}

TEST_F(HtmlUnescapeTest, named_char_reference)
{
    EXPECT_EQ( "quick&auml;brown&Uuml;fox", MISC::html_unescape( "quick&auml;brown&Uuml;fox" ) );
    EXPECT_EQ( "quick&Rarr;brown&dArr;fox", MISC::html_unescape( "quick&Rarr;brown&dArr;fox" ) );
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

    EXPECT_EQ( MISC::SCHEME_SSSP, MISC::is_url_scheme( "sssp://img.5ch", &length ) );
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


TEST_F(MISC_AscTest, fullwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u30A1\u30A2\u30A3\u30A4\u30A5\u30A6\u30A7\u30A8\u30A9\u30AA" // ァ - オ
        u8"\u30AB\u30AD\u30AF\u30B1\u30B3\u30B5\u30B7\u30B9\u30BB\u30BD" // カ - ソ
        u8"\u30BF\u30C1\u30C3\u30C4\u30C6\u30C8\u30CA\u30CB\u30CC\u30CD\u30CE" // タ - ノ
        u8"\u30CF\u30D2\u30D5\u30D8\u30DB\u30DE\u30DF\u30E0\u30E1\u30E2" // ハ - モ
        u8"\u30E3\u30E4\u30E5\u30E6\u30E7\u30E8\u30E9\u30EA\u30EB\u30EC\u30ED" // ャ - ロ
        u8"\u30EE\u30EF\u30F0\u30F1\u30F2\u30F3\u30F5\u30F6" // ヮ - ヶ
    };

    // (半)濁点の付いていない全角片仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_katakana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u30AC\u30AE\u30B0\u30B2\u30B4\u30B6\u30B8\u30BA\u30BC\u30BE" // ガ - ゾ
        u8"\u30C0\u30C2\u30C5\u30C7\u30C9\u30D0\u30D3\u30D6\u30D9\u30DC" // ダ - ボ
        u8"\u30F4\u30F7\u30F8\u30F9\u30FA" // ヴ - ヺ
        u8"\u30D1\u30D4\u30D7\u30DA0x30DD" // パ - ポ
    };

    // 合成済み文字の(半)濁点付き全角片仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_katakana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u30AB\u3099\u30AD\u3099\u30AF\u3099\u30B1\u3099\u30B3\u3099" // ガギグゲゴ
        u8"\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A" // カ゚キ゚ク゚ケ゚コ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角片仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u3041\u3042\u3043\u3044\u3045\u3046\u3047\u3048\u3049\u304A" // ぁ - お
        u8"\u304B\u304D\u304F\u3051\u3053\u3055\u3057\u3059\u305B\u305D" // か - そ
        u8"\u305F\u3061\u3063\u3064\u3066\u3068\u306A\u306B\u306C\u306D\u306E" // た - の
        u8"\u306F\u3072\u3075\u3078\u307B\u307E\u307F\u3080\u3081\u3082" // は - も
        u8"\u3083\u3084\u3085\u3086\u3087\u3088\u3089\u308A\u308B\u308C\u308D" // ゃ - ろ
        u8"\u308E\u308F\u3090\u3091\u3092\u3093\u3095\u3096" // ゎ - ゖ
    };

    // (半)濁点の付いていない全角平仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u304C\u304E\u3050\u3052\u3054\u3056\u3058\u305A\u305C\u305E" // が - ぞ
        u8"\u3060\u3062\u3065\u3067\u3069\u3070\u3073\u3076\u3079\u307C" // だ - ぼ
        u8"\u3094" // ゔ
        u8"\u3071\u3074\u3077\u307A0x307D" // ぱ - ぽ
    };

    // 合成済み文字の(半)濁点付き全角平仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        u8"\u304B\u3099\u304D\u3099\u304F\u3099\u3051\u3099\u3053\u3099" // がぎぐげご
        u8"\u304B\u309A\u304D\u309A\u304F\u309A\u3051\u309A\u3053\u309A" // か゚き゚く゚け゚こ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角平仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}


class MISC_NormTest : public ::testing::Test {};

TEST_F(MISC_NormTest, empty_input)
{
    std::string output;
    std::vector<int> table;

    // 入力はヌル終端文字列
    MISC::norm( "", output, &table );

    EXPECT_EQ( "", output );
    EXPECT_EQ( 0, output.size() );
    // 文字列の終端（ヌル文字）の位置が追加されるためtableのサイズが+1大きくなる
    EXPECT_EQ( 1, table.size() );
    EXPECT_EQ( 0, table.at( 0 ) );
}

TEST_F(MISC_NormTest, halfwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output, &table );

    EXPECT_EQ( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, halfwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "the quick brown fox jumps over the lazy dog.", output, &table );

    EXPECT_EQ( "the quick brown fox jumps over the lazy dog.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, halfwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "1234567890+-*/", output, &table );

    EXPECT_EQ( "1234567890+-*/", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, halfwidth_append_data)
{
    std::string output = "123";
    std::vector<int> table = { 0, 1, 2 };

    // アウトプット引数は初期化せずデータを追加する
    MISC::norm( "hello", output, &table );

    EXPECT_EQ( "123hello", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = { 0, 1, 2, 0, 1, 2, 3, 4, 5 };
    EXPECT_EQ( expected_table, table );
}


std::vector<int> norm_expected_table_fullwidth_quick_brown_fox()
{
    // 全角英数字から半角英数字に変換したときの文字列の位置を保存しておくテーブルのテストデータ
    return {
        // TH E  _  Q   U   I   C   K   _0  B   R   O   W   N   _   F   O   X   _
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57,
        // JU   M   P   S   _   O   V   E   R   _   T   H   E   _    L    A    Z    Y
        60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 114,
        // _ D    O    G    .    U+0000
        117, 120, 123, 126, 129, 132,
    };
}

TEST_F(MISC_NormTest, fullwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "ＴＨＥ　ＱＵＩＣＫ　ＢＲＯＷＮ　ＦＯＸ　ＪＵＭＰＳ　ＯＶＥＲ　ＴＨＥ　ＬＡＺＹ　ＤＯＧ．", output,
                &table );

    // 和字間隔(U+3000)と全角ピリオド(U+FF0E)も半角スペースに変換される
    EXPECT_EQ( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( norm_expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_NormTest, fullwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ．", output,
                &table );

    EXPECT_EQ( "the quick brown fox jumps over the lazy dog.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( norm_expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_NormTest, fullwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::norm( "１２３４５６７８９０＋−＊／", output, &table );

    // 全角Minus Sign(U+2212)以外は半角に変換される
    EXPECT_EQ( "1234567890+−*/", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
        30, 33, 34, 35,  36, 39, 42
    };
    EXPECT_EQ( expected_table, table );
}


TEST_F(MISC_NormTest, halfwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] {
        "\uFF61\uFF62\uFF63\uFF64\uFF65" "\uFF66" "\uFF67\uFF68\uFF69\uFF6A\uFF6B"
        "\uFF6C\uFF6D\uFF6E\uFF6F\uFF70" "\uFF71\uFF72\uFF73\uFF74\uFF75" "\uFF76\uFF77\uFF78\uFF79\uFF7A"
        "\uFF7B\uFF7C\uFF7D\uFF7E\uFF7F" "\uFF80\uFF81\uFF82\uFF83\uFF84" "\uFF85\uFF86\uFF87\uFF88\uFF89"
        "\uFF8A\uFF8B\uFF8C\uFF8D\uFF8E" "\uFF8F\uFF90\uFF91\uFF92\uFF93" "\uFF94\uFF95\uFF96"
        "\uFF97\uFF98\uFF99\uFF9A\uFF9B" "\uFF9C\uFF9D"
    };
    constexpr const char fullwidth[] {
        "。「」、・" "ヲ" "ァィゥェォ"
        "ャュョッー" "アイウエオ" "カキクケコ"
        "サシスセソ" "タチツテト" "ナニヌネノ"
        "ハヒフヘホ" "マミムメモ" "ヤユヨ"
        "ラリルレロ" "ワン"
    };

    // 半角片仮名から全角片仮名へ一対一の変換
    MISC::norm( halfwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, halfwidth_katakana_only_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;

    // 半角の濁点と半濁点は全角結合文字に変換される
    MISC::norm( "\uFF9E\uFF9F", output, &table );

    EXPECT_EQ( "\u3099\u309A", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, halfwidth_katakana_combining_voiced_sound_mark_to_decomposed)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] = {
        "\uFF76\uFF9E\uFF77\uFF9E\uFF78\uFF9E\uFF79\uFF9E\uFF7A\uFF9E"
        "\uFF7B\uFF9E\uFF7C\uFF9E\uFF7D\uFF9E\uFF7E\uFF9E\uFF7F\uFF9E"
        "\uFF80\uFF9E\uFF81\uFF9E\uFF82\uFF9E\uFF83\uFF9E\uFF84\uFF9E"
        "\uFF8A\uFF9E\uFF8B\uFF9E\uFF8C\uFF9E\uFF8D\uFF9E\uFF8E\uFF9E"
        "\uFF8A\uFF9F\uFF8B\uFF9F\uFF8C\uFF9F\uFF8D\uFF9F\uFF8E\uFF9F"
    };
    constexpr const char fullwidth[] {
        "\u30AB\u3099\u30AD\u3099\u30AF\u3099\u30B1\u3099\u30B3\u3099" // ガギグゲゴ
        "\u30B5\u3099\u30B7\u3099\u30B9\u3099\u30BB\u3099\u30BD\u3099" // ザジズゼゾ
        "\u30BF\u3099\u30C1\u3099\u30C4\u3099\u30C6\u3099\u30C8\u3099" // ダヂヅデド
        "\u30CF\u3099\u30D2\u3099\u30D5\u3099\u30D8\u3099\u30DB\u3099" // バビブベボ
        "\u30CF\u309A\u30D2\u309A\u30D5\u309A\u30D8\u309A\u30DB\u309A" // パピプペポ
    };

    // 合成済み文字が存在する(半)濁点付き半角片仮名は全角片仮名と結合文字へ変換される
    MISC::norm( halfwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );

    std::vector<int> expected_table( table.size() );
    std::iota( expected_table.begin(), expected_table.end(), 0 );
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_NormTest, halfwidth_katakana_combining_voiced_sound_mark_wagyo)
{
    std::string output;
    std::vector<int> table;

    // 濁点付き半角片仮名のウやワ行は全角片仮名と結合文字へ変換される
    MISC::norm( "\uFF73\uFF9E\uFF9C\uFF9E\uFF66\uFF9E", output, &table );

    EXPECT_EQ( "\u30A6\u3099\u30EF\u3099\u30F2\u3099", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    std::vector<int> expected_table( table.size() );
    std::iota( expected_table.begin(), expected_table.end(), 0 );
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_NormTest, halfwidth_katakana_combining_voiced_sound_mark_through)
{
    std::string output;
    std::vector<int> table;

    // 合成済み文字が存在しない(半)濁点付き半角片仮名は全角片仮名と結合文字に変換される : カ゚キ゚ク゚ケ゚コ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::norm( "\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", output, &table );

    EXPECT_EQ( "\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}


TEST_F(MISC_NormTest, fullwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30A1\u30A2\u30A3\u30A4\u30A5\u30A6\u30A7\u30A8\u30A9\u30AA" // ァ - オ
        "\u30AB\u30AD\u30AF\u30B1\u30B3\u30B5\u30B7\u30B9\u30BB\u30BD" // カ - ソ
        "\u30BF\u30C1\u30C3\u30C4\u30C6\u30C8\u30CA\u30CB\u30CC\u30CD\u30CE" // タ - ノ
        "\u30CF\u30D2\u30D5\u30D8\u30DB\u30DE\u30DF\u30E0\u30E1\u30E2" // ハ - モ
        "\u30E3\u30E4\u30E5\u30E6\u30E7\u30E8\u30E9\u30EA\u30EB\u30EC\u30ED" // ャ - ロ
        "\u30EE\u30EF\u30F0\u30F1\u30F2\u30F3\u30F5\u30F6" // ヮ - ヶ
    };

    // (半)濁点の付いていない全角片仮名はそのまま
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, fullwidth_katakana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30AC\u30AE\u30B0\u30B2\u30B4\u30B6\u30B8\u30BA\u30BC\u30BE" // ガ - ゾ
        "\u30C0\u30C2\u30C5\u30C7\u30C9\u30D0\u30D3\u30D6\u30D9\u30DC" // ダ - ボ
        "\u30F4\u30F7\u30F8\u30F9\u30FA" // ヴ - ヺ
        "\u30D1\u30D4\u30D7\u30DA\u30DD" // パ - ポ
    };
    constexpr const char fullwidth_combining[] {
        "\u30AB\u3099\u30AD\u3099\u30AF\u3099\u30B1\u3099\u30B3\u3099" // ガギグゲゴ
        "\u30B5\u3099\u30B7\u3099\u30B9\u3099\u30BB\u3099\u30BD\u3099" // ザジズゼゾ
        "\u30BF\u3099\u30C1\u3099\u30C4\u3099\u30C6\u3099\u30C8\u3099" // ダヂヅデド
        "\u30CF\u3099\u30D2\u3099\u30D5\u3099\u30D8\u3099\u30DB\u3099" // バビブベボ
        "\u30A6\u3099\u30EF\u3099\u30F0\u3099\u30F1\u3099\u30F2\u3099" // ヴヷヸヹヺ
        "\u30CF\u309A\u30D2\u309A\u30D5\u309A\u30D8\u309A\u30DB\u309A" // パピプペポ
    };

    // 合成済み文字の(半)濁点付き全角片仮名は結合文字が分解される
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth_combining, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( std::size_t i = 0; i < table.size() - 6; i += 6 ) {
        EXPECT_EQ( i / 2 + 0, table.at(i + 0) );
        EXPECT_EQ( i / 2 + 1, table.at(i + 1) );
        EXPECT_EQ( i / 2 + 2, table.at(i + 2) );
        EXPECT_EQ( i / 2 + 3, table.at(i + 3) );
        EXPECT_EQ( i / 2 + 4, table.at(i + 4) );
        EXPECT_EQ( i / 2 + 5, table.at(i + 5) );
    }
}

TEST_F(MISC_NormTest, fullwidth_katakana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30AB\u3099\u30AD\u3099\u30AF\u3099\u30B1\u3099\u30B3\u3099" // ガギグゲゴ
        "\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A" // カ゚キ゚ク゚ケ゚コ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角片仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, fullwidth_hiragana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u3041\u3042\u3043\u3044\u3045\u3046\u3047\u3048\u3049\u304A" // ぁ - お
        "\u304B\u304D\u304F\u3051\u3053\u3055\u3057\u3059\u305B\u305D" // か - そ
        "\u305F\u3061\u3063\u3064\u3066\u3068\u306A\u306B\u306C\u306D\u306E" // た - の
        "\u306F\u3072\u3075\u3078\u307B\u307E\u307F\u3080\u3081\u3082" // は - も
        "\u3083\u3084\u3085\u3086\u3087\u3088\u3089\u308A\u308B\u308C\u308D" // ゃ - ろ
        "\u308E\u308F\u3090\u3091\u3092\u3093\u3095\u3096" // ゎ - ゖ
    };

    // (半)濁点の付いていない全角平仮名はそのまま
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_NormTest, fullwidth_hiragana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u304C\u304E\u3050\u3052\u3054\u3056\u3058\u305A\u305C\u305E" // が - ぞ
        "\u3060\u3062\u3065\u3067\u3069\u3070\u3073\u3076\u3079\u307C" // だ - ぼ
        "\u3094" // ゔ
        "\u3071\u3074\u3077\u307A\u307D" // ぱ - ぽ
    };
    constexpr const char fullwidth_combining[] {
        "\u304B\u3099\u304D\u3099\u304F\u3099\u3051\u3099\u3053\u3099"
        "\u3055\u3099\u3057\u3099\u3059\u3099\u305B\u3099\u305D\u3099"
        "\u305F\u3099\u3061\u3099\u3064\u3099\u3066\u3099\u3068\u3099"
        "\u306F\u3099\u3072\u3099\u3075\u3099\u3078\u3099\u307B\u3099"
        "\u3046\u3099"
        "\u306F\u309A\u3072\u309A\u3075\u309A\u3078\u309A\u307B\u309A"
    };

    // 合成済み文字の(半)濁点付き全角平仮名は結合文字が分解される
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth_combining, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( std::size_t i = 0; i < table.size() - 6; i += 6 ) {
        EXPECT_EQ( i / 2 + 0, table.at(i + 0) );
        EXPECT_EQ( i / 2 + 1, table.at(i + 1) );
        EXPECT_EQ( i / 2 + 2, table.at(i + 2) );
        EXPECT_EQ( i / 2 + 3, table.at(i + 3) );
        EXPECT_EQ( i / 2 + 4, table.at(i + 4) );
        EXPECT_EQ( i / 2 + 5, table.at(i + 5) );
    }
}

TEST_F(MISC_NormTest, fullwidth_hiragana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u304B\u3099\u304D\u3099\u304F\u3099\u3051\u3099\u3053\u3099" // がぎぐげご
        "\u304B\u309A\u304D\u309A\u304F\u309A\u3051\u309A\u3053\u309A" // か゚き゚く゚け゚こ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角平仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::norm( fullwidth, output, &table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}


class MISC_StartsWith : public ::testing::Test {};

TEST_F(MISC_StartsWith, null_terminated_string_with_zero_length)
{
    EXPECT_TRUE( MISC::starts_with( "", "" ) );
    EXPECT_TRUE( MISC::starts_with( "helloworld", "" ) );
    EXPECT_FALSE( MISC::starts_with( "", "helloworld" ) );
}

TEST_F(MISC_StartsWith, null_terminated_string)
{
    EXPECT_TRUE( MISC::starts_with( "hello", "hello" ) );
    EXPECT_TRUE( MISC::starts_with( "helloworld", "hello" ) );
    EXPECT_FALSE( MISC::starts_with( "hello", "helloworld" ) );
}


class MISC_EndsWith : public ::testing::Test {};

TEST_F(MISC_EndsWith, empty_haystack_and_needle)
{
    EXPECT_TRUE( MISC::ends_with( "", "" ) );
}

TEST_F(MISC_EndsWith, empty_haystack)
{
    EXPECT_FALSE( MISC::ends_with( "", "needle" ) );
}

TEST_F(MISC_EndsWith, empty_needle)
{
    EXPECT_TRUE( MISC::ends_with( "haystack", "" ) );
}

TEST_F(MISC_EndsWith, hello_world)
{
    EXPECT_TRUE( MISC::ends_with( "Hello World", "World" ) );
}

TEST_F(MISC_EndsWith, too_long_needle)
{
    EXPECT_FALSE( MISC::ends_with( "World", "Hello World" ) );
}

TEST_F(MISC_EndsWith, not_match)
{
    EXPECT_FALSE( MISC::ends_with( "quick brown fox", "dogs" ) );
}


class MISC_ParseHtmlFormData : public ::testing::Test {};

TEST_F(MISC_ParseHtmlFormData, empty_html)
{
    auto result = MISC::parse_html_form_data( std::string{} );
    EXPECT_TRUE( result.empty() );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum)
{
    auto result = MISC::parse_html_form_data( "<input type=hidden name=FOO value=100>" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum_uppercase)
{
    auto result = MISC::parse_html_form_data( "<INPUT TYPE=hidden NAME=FOO VALUE=100>" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="FOO" value="100">)" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_datum)
{
    auto result = MISC::parse_html_form_data( "<input type=submit value=書き込む name=submit>" );
    decltype(result) expect{ { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value="書き込む" name="submit">)" );
    decltype(result) expect{ { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum)
{
    auto result = MISC::parse_html_form_data( "<textarea name=MESSAGE>あかさたな</textarea>" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_have_tag)
{
    auto result = MISC::parse_html_form_data( "<textarea name=MESSAGE>あか<br>さたな</textarea>" );
    decltype(result) expect{ { "MESSAGE", "あか<br>さたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum_uppercase)
{
    auto result = MISC::parse_html_form_data( "<TEXTAREA NAME=MESSAGE>あかさたな</TEXTAREA>" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE">"あかさたな"</textarea>)" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_1)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE">"あかさたな"</textarea>)"
                                              "<input type=submit value=書き込む name=submit>"
                                              R"(<input type="hidden" name="FOO" value="100">)" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" }, { "submit", "書き込む" }, { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_2)
{
    auto result = MISC::parse_html_form_data( "<input type=submit value=書き込む name=submit>"
                                              R"(<input type="hidden" name="FOO" value="100">)"
                                              R"(<textarea name="MESSAGE">"あかさたな"</textarea>)" );
    decltype(result) expect{ { "submit", "書き込む" }, { "FOO", "100" }, { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_3)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="FOO" value="100">)"
                                              R"(<textarea name="MESSAGE">"あかさたな"</textarea>)"
                                              "<input type=submit value=書き込む name=submit>" );
    decltype(result) expect{ { "FOO", "100" }, { "MESSAGE", "あかさたな" }, { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="" value="100">)" );
    decltype(result) expect{ { std::string{}, "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name=FOO value="">)" );
    decltype(result) expect{ { "FOO", std::string{} } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value=書き込む name="">)" );
    decltype(result) expect{ { std::string{}, "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value="" name=submit>)" );
    decltype(result) expect{ { "submit", std::string{} } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="">あかさたな</textarea>)" );
    decltype(result) expect{ { std::string{}, "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE"></textarea>)" );
    decltype(result) expect{ { "MESSAGE", std::string{} } };
    EXPECT_EQ( result, expect );
}


class MISC_ParseHtmlFormActionTest : public ::testing::Test {};

TEST_F(MISC_ParseHtmlFormActionTest, empty_html)
{
    std::string result = MISC::parse_html_form_action( std::string{} );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, unquote_post)
{
    const std::string html = R"(<form method=POST action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, quote_post)
{
    const std::string html = R"(<form method="POST" action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, subbbscgi)
{
    const std::string html = R"(<form method="POST" action="/test/subbbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/subbbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, with_parameter)
{
    const std::string html = R"(<form method="POST" action="/test/bbs.cgi?foo=bar">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi?foo=bar" );
}

TEST_F(MISC_ParseHtmlFormActionTest, ignore_attributes)
{
    const std::string html = R"(<form method="POST" accept-charset="Shift_JIS" action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_an_upper_order)
{
    const std::string html = R"(<form method="POST" action="../test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_same_hierarchy)
{
    std::string html = R"(<form method="POST" action="./bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );

    html = R"(<form method="POST" action="./subbbs.cgi">")";
    result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/subbbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_double_upper_orders)
{
    const std::string html = R"(<form method="POST" action="../../test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_middle_upper_order)
{
    const std::string html = R"(<form method="POST" action="/test/../bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, absolute_path_without_scheme)
{
    const std::string html = R"(<form method="POST" action="//example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, http_url)
{
    const std::string html = R"(<form method="POST" action="http://example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, https_url)
{
    const std::string html = R"(<form method="POST" action="https://example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}


class MISC_ParseCharsetFromHtmlMetaTest : public ::testing::Test {};

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, empty_html)
{
    std::string result = MISC::parse_charset_from_html_meta( std::string{} );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, http_equiv)
{
    const std::string html = R"(<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "UTF-8" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, http_equiv_uppercase)
{
    const std::string html = R"(<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="TEXT/HTML; CHARSET=Shift_JIS"/>)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "Shift_JIS" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, http_equiv_trim)
{
    const std::string html = R"(<meta http-equiv="content-type" content="text/html; charset= EUC-JP "/>)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "EUC-JP" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, charset)
{
    const std::string html = R"(<meta charset="UTF-8">)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "UTF-8" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, charset_uppercase)
{
    const std::string html = R"(<META CHARSET="Shift_JIS">)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "Shift_JIS" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, charset_trim)
{
    const std::string html = R"(<meta charset=" EUC-JP ">)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "EUC-JP" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, charset_no_quote)
{
    const std::string html = R"(<meta charset=latin1>)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "latin1" );
}

TEST_F(MISC_ParseCharsetFromHtmlMetaTest, choose_first_meta_tag)
{
    const std::string html = R"(<meta http-equiv="Content-Type" content="text/html; charset=utf8">)"
                             R"(<meta charset="x-sjis">)";
    std::string result = MISC::parse_charset_from_html_meta( html );
    EXPECT_EQ( result, "utf8" );
}


class ToPlainTest : public ::testing::Test {};

TEST_F(ToPlainTest, empty)
{
    const std::string result = MISC::to_plain( std::string{} );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(ToPlainTest, no_conversion)
{
    const std::string result = MISC::to_plain( "hello 世界" );
    EXPECT_EQ( result, "hello 世界" );
}

TEST_F(ToPlainTest, decimal_hello)
{
    const std::string result = MISC::to_plain( "&#104;&#101;&#108;&#108;&#111;" );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ToPlainTest, hexadecimal_hello)
{
    const std::string result = MISC::to_plain( "&#x68;&#x65;&#X6c;&#x6C;&#x6f;" );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ToPlainTest, escape_html_char_completely)
{
    const std::string input = "&#60;&#62;&#38;&#34; &#x3c;&#x3e;&#x26;&#x22; &lt;&gt;&amp;&quot;";
    const std::string result = MISC::to_plain( input );
    EXPECT_EQ( result, R"(<>&" <>&" <>&")" );
}

TEST_F(ToPlainTest, flatten_tags)
{
    const std::string input = "Hello<foo>世界<bar>Quick</bar></foo>Brown Fox";
    const std::string result = MISC::to_plain( input );
    EXPECT_EQ( result, "Hello世界QuickBrown Fox" );
}

TEST_F(ToPlainTest, broken_tags)
{
    std::string input = "Hello<fo<o>世界</f>oo>Quick Brown Fox";
    std::string result = MISC::to_plain( input );
    EXPECT_EQ( result, "Hello世界oo>Quick Brown Fox" );

    input = "Hello<foo>世界>Quick</foo<Brown Fox";
    result = MISC::to_plain( input );
    EXPECT_EQ( result, "Hello世界>Quick" );
}


class ToMarkupTest : public ::testing::Test {};

TEST_F(ToMarkupTest, empty)
{
    const std::string result = MISC::to_markup( std::string{} );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(ToMarkupTest, no_conversion)
{
    const std::string result = MISC::to_markup( "hello 世界" );
    EXPECT_EQ( result, "hello 世界" );
}

TEST_F(ToMarkupTest, decimal_hello)
{
    const std::string result = MISC::to_markup( "&#104;&#101;&#108;&#108;&#111;" );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ToMarkupTest, hexadecimal_hello)
{
    const std::string result = MISC::to_markup( "&#x68;&#x65;&#X6c;&#x6C;&#x6f;" );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ToMarkupTest, escape_html_char_completely)
{
    // 動作の根拠がはっきりしていないが &quot; は " に変換される
    const std::string input = "&#60;&#62;&#38;&#34; &#x3c;&#x3e;&#x26;&#x22; &lt;&gt;&amp;&quot;";
    const std::string result = MISC::to_markup( input );
    EXPECT_EQ( result, R"(&lt;&gt;&amp;&quot; &lt;&gt;&amp;&quot; &lt;&gt;&amp;")" );
}

TEST_F(ToMarkupTest, flatten_tags)
{
    const std::string input = "Hello<foo>世界<bar>Quick</bar></foo>Brown Fox";
    const std::string result = MISC::to_markup( input );
    EXPECT_EQ( result, "Hello世界QuickBrown Fox" );
}

TEST_F(ToMarkupTest, broken_tags)
{
    std::string input = "Hello<fo<o>世界</f>oo>Quick Brown Fox";
    std::string result = MISC::to_markup( input );
    EXPECT_EQ( result, "Hello世界oo>Quick Brown Fox" );

    input = "Hello<foo>世界>Quick</foo<Brown Fox";
    result = MISC::to_markup( input );
    EXPECT_EQ( result, "Hello世界>Quick" );
}

TEST_F(ToMarkupTest, span_tags)
{
    std::string input = "Hello<span>世界</span>Quick Brown Fox";
    std::string result = MISC::to_markup( input );
    EXPECT_EQ( result, "Hello<span>世界</span>Quick Brown Fox" );

    input = R"(Hello 世界<span class="mark">Quick</span>Brown Fox)";
    result = MISC::to_markup( input );
    EXPECT_EQ( result, R"(Hello 世界<span color="#000000000000" background="#ffffffff0000">Quick</span>Brown Fox)" );
}

TEST_F(ToMarkupTest, mark_tags)
{
    std::string input = "Hello<mark>世界</mark>Quick Brown Fox";
    std::string result = MISC::to_markup( input );
    EXPECT_EQ( result, R"(Hello<span color="#000000000000" background="#ffffffff0000">世界</span>Quick Brown Fox)" );

    input = R"(Hello<mark class="jdim-unused-css-class">世界</mark>Quick Brown Fox)";
    result = MISC::to_markup( input );
    EXPECT_EQ( result, "Hello<span>世界</span>Quick Brown Fox" );
}


class ChrefDecodeTest : public ::testing::Test {};

TEST_F(ChrefDecodeTest, empty)
{
    const std::string result = MISC::chref_decode( std::string{}, true );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(ChrefDecodeTest, decimal_hello)
{
    const std::string result = MISC::chref_decode( std::string{ "&#104;&#101;&#108;&#108;&#111;" }, true );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ChrefDecodeTest, hexadecimal_hello)
{
    const std::string result = MISC::chref_decode( std::string{ "&#x68;&#x65;&#X6c;&#x6C;&#x6f;" }, true );
    EXPECT_EQ( result, "hello" );
}

TEST_F(ChrefDecodeTest, escape_html_char_completely)
{
    const std::string input = "&#60;&#62;&#38;&#34; &#x3c;&#x3e;&#x26;&#x22; &lt;&gt;&amp;&quot;";
    const std::string result = MISC::chref_decode( input, true );
    EXPECT_EQ( result, R"(<>&" <>&" <>&")" );
}

TEST_F(ChrefDecodeTest, escape_html_char_keeping)
{
    const std::string input = "&#60;&#62;&#38;&#34; &#x3c;&#x3e;&#x26;&#x22; &lt;&gt;&amp;&quot;";
    const std::string result = MISC::chref_decode( input, false );
    EXPECT_EQ( result, "&lt;&gt;&amp;&quot; &lt;&gt;&amp;&quot; &lt;&gt;&amp;&quot;" );
}


class UrlDecodeTest : public ::testing::Test {};

TEST_F(UrlDecodeTest, empty)
{
    EXPECT_EQ( "", MISC::url_decode( "" ) );
}

TEST_F(UrlDecodeTest, not_decode)
{
    EXPECT_EQ( "http://foobar.test?a=1&b=c", MISC::url_decode( "http://foobar.test?a=1&b=c" ) );
}

TEST_F(UrlDecodeTest, decode_hiragana)
{
    constexpr const char* url = "http://hira.test/%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
    EXPECT_EQ( "http://hira.test/あいうえお", MISC::url_decode( url ) );
}

TEST_F(UrlDecodeTest, decode_plus_sign)
{
    constexpr const char* url = "http://plus.test/Quick+Brown+Fox";
    EXPECT_EQ( "http://plus.test/Quick Brown Fox", MISC::url_decode( url ) );
}

TEST_F(UrlDecodeTest, out_of_range_segments)
{
    constexpr const char* url = "http://out.test/%41%4G%61%G1%%a";
    EXPECT_EQ( "http://out.test/A%4Ga%G1%%a", MISC::url_decode( url ) );
}


class MISC_UrlEncodeTest : public ::testing::Test {};

TEST_F(MISC_UrlEncodeTest, empty_string)
{
    std::string_view input = "";
    EXPECT_EQ( "", MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, unencoded_ascii_characters)
{
    std::string_view input = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz*-._";
    EXPECT_EQ( input, MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, u0020)
{
    std::string_view input = " ";
    EXPECT_EQ( "%20", MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, u000A)
{
    std::string_view input = "quick\nbrown\n\nfox";
    EXPECT_EQ( "quick%0Abrown%0A%0Afox", MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, u000D)
{
    std::string_view input = "quick\rbrown\r\rfox";
    EXPECT_EQ( "quick%0Dbrown%0D%0Dfox", MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, u000D_u000A)
{
    std::string_view input = "quick\r\nbrown\r\n\r\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, encoded_ascii_characters)
{
    std::string_view input = "!\"#$%&\'()_+,_/_:;<=>?@_[\\]^_`_{|}~";
    std::string_view result = "%21%22%23%24%25%26%27%28%29_%2B%2C_%2F_"
                              "%3A%3B%3C%3D%3E%3F%40_%5B%5C%5D%5E_%60_%7B%7C%7D%7E";
    EXPECT_EQ( result, MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, encoded_utf8)
{
    std::string_view input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%E3%81%82";
    EXPECT_EQ( result, MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, encoded_shift_jis)
{
    std::string_view input = "\x82\xA0"; // HIRAGANA A
    std::string_view result = "%82%A0";
    EXPECT_EQ( result, MISC::url_encode( input ) );
}

TEST_F(MISC_UrlEncodeTest, url_utf8)
{
    std::string_view input = "https://jdim.test/い ろ/は?に=ほ&へ=と z";
    std::string_view result = "https%3A%2F%2Fjdim.test%2F%E3%81%84%20%E3%82%8D%2F"
                              "%E3%81%AF%3F%E3%81%AB%3D%E3%81%BB%26%E3%81%B8%3D%E3%81%A8%20z";
    EXPECT_EQ( result, MISC::url_encode( input ) );
}


class MISC_UrlEncodeWithEncodingTest : public ::testing::Test {};

TEST_F(MISC_UrlEncodeWithEncodingTest, empty_string)
{
    std::string input = "";
    EXPECT_EQ( "", MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, unencoded_ascii_characters)
{
    std::string input = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz*-._";
    EXPECT_EQ( input, MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, u0020)
{
    std::string input = " ";
    EXPECT_EQ( "%20", MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, u000A)
{
    std::string input = "quick\nbrown\n\nfox";
    EXPECT_EQ( "quick%0Abrown%0A%0Afox", MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, u000D)
{
    std::string input = "quick\rbrown\r\rfox";
    EXPECT_EQ( "quick%0Dbrown%0D%0Dfox", MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, u000D_u000A)
{
    std::string input = "quick\r\nbrown\r\n\r\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, encoded_ascii_characters)
{
    std::string input = "!\"#$%&\'()_+,_/_:;<=>?@_[\\]^_`_{|}~";
    std::string_view result = "%21%22%23%24%25%26%27%28%29_%2B%2C_%2F_"
                              "%3A%3B%3C%3D%3E%3F%40_%5B%5C%5D%5E_%60_%7B%7C%7D%7E";
    EXPECT_EQ( result, MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, encoded_to_ms932)
{
    std::string input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%82%A0";
    EXPECT_EQ( result, MISC::url_encode( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, encoded_to_eucjp)
{
    std::string input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%A4%A2";
    EXPECT_EQ( result, MISC::url_encode( input, Encoding::eucjp ) );
}

TEST_F(MISC_UrlEncodeWithEncodingTest, url_to_ms932)
{
    std::string input = "https://jdim.test/い ろ/は?に=ほ&へ=と z";
    std::string_view result = "https%3A%2F%2Fjdim.test%2F%82%A2%20%82%EB%2F"
                              "%82%CD%3F%82%C9%3D%82%D9%26%82%D6%3D%82%C6%20z";
    EXPECT_EQ( result, MISC::url_encode( input, Encoding::sjis ) );
}


class MISC_UrlEncodePlusTest : public ::testing::Test {};

TEST_F(MISC_UrlEncodePlusTest, empty_string)
{
    std::string_view input = "";
    EXPECT_EQ( "", MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, unencoded_ascii_characters)
{
    std::string_view input = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz*-._";
    EXPECT_EQ( input, MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, u0020)
{
    std::string_view input = " ";
    EXPECT_EQ( "+", MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, u000A)
{
    std::string_view input = "quick\nbrown\n\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, u000D)
{
    std::string_view input = "quick\rbrown\r\rfox";
    EXPECT_EQ( "quickbrownfox", MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, u000D_u000A)
{
    std::string_view input = "quick\r\nbrown\r\n\r\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, encoded_ascii_characters)
{
    std::string_view input = "!\"#$%&\'()_+,_/_:;<=>?@_[\\]^_`_{|}~";
    std::string_view result = "%21%22%23%24%25%26%27%28%29_%2B%2C_%2F_"
                              "%3A%3B%3C%3D%3E%3F%40_%5B%5C%5D%5E_%60_%7B%7C%7D%7E";
    EXPECT_EQ( result, MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, encoded_utf8)
{
    std::string_view input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%E3%81%82";
    EXPECT_EQ( result, MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, encoded_shift_jis)
{
    std::string_view input = "\x82\xA0"; // HIRAGANA A
    std::string_view result = "%82%A0";
    EXPECT_EQ( result, MISC::url_encode_plus( input ) );
}

TEST_F(MISC_UrlEncodePlusTest, url_utf8)
{
    std::string_view input = "https://jdim.test/い ろ/は?に=ほ&へ=と z";
    std::string_view result = "https%3A%2F%2Fjdim.test%2F%E3%81%84+%E3%82%8D%2F"
                              "%E3%81%AF%3F%E3%81%AB%3D%E3%81%BB%26%E3%81%B8%3D%E3%81%A8+z";
    EXPECT_EQ( result, MISC::url_encode_plus( input ) );
}


class MISC_UrlEncodePlusWithEncodingTest : public ::testing::Test {};

TEST_F(MISC_UrlEncodePlusWithEncodingTest, empty_string)
{
    std::string input = "";
    EXPECT_EQ( "", MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, unencoded_ascii_characters)
{
    std::string input = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz*-._";
    EXPECT_EQ( input, MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, single_u0020)
{
    std::string input = " ";
    EXPECT_EQ( "+", MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, u000A)
{
    std::string input = "quick\nbrown\n\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, u000D)
{
    std::string input = "quick\rbrown\r\rfox";
    EXPECT_EQ( "quickbrownfox", MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, u000D_u000A)
{
    std::string input = "quick\r\nbrown\r\n\r\nfox";
    EXPECT_EQ( "quick%0D%0Abrown%0D%0A%0D%0Afox", MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, words_separated_by_u0020)
{
    // U+3000 won't be converted to '+'
    std::string input = "Quick Brown　Fox い ろ　は";
    std::string_view result = "Quick+Brown%81%40Fox+%82%A2+%82%EB%81%40%82%CD";
    EXPECT_EQ( result, MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, encoded_ascii_characters)
{
    std::string input = "!\"#$%&\'()_+,_/_:;<=>?@_[\\]^_`_{|}~";
    std::string_view result = "%21%22%23%24%25%26%27%28%29_%2B%2C_%2F_"
                              "%3A%3B%3C%3D%3E%3F%40_%5B%5C%5D%5E_%60_%7B%7C%7D%7E";
    EXPECT_EQ( result, MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, encoded_to_ms932)
{
    std::string input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%82%A0";
    EXPECT_EQ( result, MISC::url_encode_plus( input, Encoding::sjis ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, encoded_to_eucjp)
{
    std::string input = "\xE3\x81\x82"; // U+3042
    std::string_view result = "%A4%A2";
    EXPECT_EQ( result, MISC::url_encode_plus( input, Encoding::eucjp ) );
}

TEST_F(MISC_UrlEncodePlusWithEncodingTest, url_to_ms932)
{
    std::string input = "https://jdim.test/い ろ/は?に=ほ&へ=と z";
    std::string_view result = "https%3A%2F%2Fjdim.test%2F%82%A2+%82%EB%2F"
                              "%82%CD%3F%82%C9%3D%82%D9%26%82%D6%3D%82%C6+z";
    EXPECT_EQ( result, MISC::url_encode_plus( input, Encoding::sjis ) );
}


class MISC_DecodeSpcharNumberRawTest : public ::testing::Test {};

TEST_F(MISC_DecodeSpcharNumberRawTest, empty_string)
{
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "", 0, 0 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, padding_zeros)
{
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#0000;", 2, 4 ) );
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#x0000;", 3, 4 ) );
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#X0000;", 3, 4 ) );
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#0013;", 2, 4 ) );
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#x000D;", 3, 4 ) );
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#X000d;", 3, 4 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, null_character)
{
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#0;", 2, 1 ) );
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#x0;", 3, 1 ) );
    EXPECT_EQ( 0, MISC::decode_spchar_number_raw( "&#X0;", 3, 1 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, carriage_return)
{
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#13;", 2, 2 ) );
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#xD;", 3, 1 ) );
    EXPECT_EQ( 0x000D, MISC::decode_spchar_number_raw( "&#Xd;", 3, 1 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, ascii_whitespace)
{
    EXPECT_EQ( 0x0009, MISC::decode_spchar_number_raw( "&#9;", 2, 1 ) );
    EXPECT_EQ( 0x0009, MISC::decode_spchar_number_raw( "&#x9;", 3, 1 ) );

    EXPECT_EQ( 0x000A, MISC::decode_spchar_number_raw( "&#10;", 2, 2 ) );
    EXPECT_EQ( 0x000A, MISC::decode_spchar_number_raw( "&#xA;", 3, 1 ) );

    EXPECT_EQ( 0x000C, MISC::decode_spchar_number_raw( "&#12;", 2, 2 ) );
    EXPECT_EQ( 0x000C, MISC::decode_spchar_number_raw( "&#xC;", 3, 1 ) );

    EXPECT_EQ( 0x0020, MISC::decode_spchar_number_raw( "&#32;", 2, 2 ) );
    EXPECT_EQ( 0x0020, MISC::decode_spchar_number_raw( "&#x20;", 3, 2 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, out_of_range)
{
    EXPECT_EQ( 0x110000, MISC::decode_spchar_number_raw( "&#1114112;", 2, 7 ) );
    EXPECT_EQ( 0x110000, MISC::decode_spchar_number_raw( "&#x110000;", 3, 6 ) );

    EXPECT_EQ( 0x1000000, MISC::decode_spchar_number_raw( "&#16777216;", 2, 8 ) );
    EXPECT_EQ( 0x1000000, MISC::decode_spchar_number_raw( "&#x1000000;", 3, 7 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, high_surrogate)
{
    EXPECT_EQ( 0xD800, MISC::decode_spchar_number_raw( "&#55296;", 2, 5 ) );
    EXPECT_EQ( 0xD800, MISC::decode_spchar_number_raw( "&#xD800;", 3, 4 ) );

    EXPECT_EQ( 0xDBFF, MISC::decode_spchar_number_raw( "&#56319;", 2, 5 ) );
    EXPECT_EQ( 0xDBFF, MISC::decode_spchar_number_raw( "&#xDBFF;", 3, 4 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, low_surrogate)
{
    EXPECT_EQ( 0xDC00, MISC::decode_spchar_number_raw( "&#56320;", 2, 5 ) );
    EXPECT_EQ( 0xDC00, MISC::decode_spchar_number_raw( "&#xDC00;", 3, 4 ) );

    EXPECT_EQ( 0xDFFF, MISC::decode_spchar_number_raw( "&#57343;", 2, 5 ) );
    EXPECT_EQ( 0xDFFF, MISC::decode_spchar_number_raw( "&#xDFFF;", 3, 4 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, noncharacter)
{
    EXPECT_EQ( 0xFDD0, MISC::decode_spchar_number_raw( "&#64976;", 2, 5 ) );
    EXPECT_EQ( 0xFDD0, MISC::decode_spchar_number_raw( "&#xFDD0;", 3, 4 ) );

    EXPECT_EQ( 0xFDEF, MISC::decode_spchar_number_raw( "&#65007;", 2, 5 ) );
    EXPECT_EQ( 0xFDEF, MISC::decode_spchar_number_raw( "&#xFDEF;", 3, 4 ) );

    EXPECT_EQ( 0xFFFE, MISC::decode_spchar_number_raw( "&#65534;", 2, 5 ) );
    EXPECT_EQ( 0xFFFE, MISC::decode_spchar_number_raw( "&#xFFFE;", 3, 4 ) );

    EXPECT_EQ( 0xFFFF, MISC::decode_spchar_number_raw( "&#65535;", 2, 5 ) );
    EXPECT_EQ( 0xFFFF, MISC::decode_spchar_number_raw( "&#xFFFF;", 3, 4 ) );

    EXPECT_EQ( 0x1FFFE, MISC::decode_spchar_number_raw( "&#131070;", 2, 6 ) );
    EXPECT_EQ( 0x1FFFE, MISC::decode_spchar_number_raw( "&#x1FFFE;", 3, 5 ) );

    EXPECT_EQ( 0x1FFFF, MISC::decode_spchar_number_raw( "&#131071;", 2, 6 ) );
    EXPECT_EQ( 0x1FFFF, MISC::decode_spchar_number_raw( "&#x1FFFF;", 3, 5 ) );

    EXPECT_EQ( 0x10FFFE, MISC::decode_spchar_number_raw( "&#1114110;", 2, 7 ) );
    EXPECT_EQ( 0x10FFFE, MISC::decode_spchar_number_raw( "&#x10FFFE;", 3, 6 ) );

    EXPECT_EQ( 0x10FFFF, MISC::decode_spchar_number_raw( "&#1114111;", 2, 7 ) );
    EXPECT_EQ( 0x10FFFF, MISC::decode_spchar_number_raw( "&#x10FFFF;", 3, 6 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, between_u007F_and_u009F_decimal)
{
    EXPECT_EQ( 0x007F, MISC::decode_spchar_number_raw( "&#127;", 2, 3 ) );
    EXPECT_EQ( 0x0080, MISC::decode_spchar_number_raw( "&#128;", 2, 3 ) );
    EXPECT_EQ( 0x0081, MISC::decode_spchar_number_raw( "&#129;", 2, 3 ) );
    EXPECT_EQ( 0x0082, MISC::decode_spchar_number_raw( "&#130;", 2, 3 ) );
    EXPECT_EQ( 0x0083, MISC::decode_spchar_number_raw( "&#131;", 2, 3 ) );
    EXPECT_EQ( 0x0084, MISC::decode_spchar_number_raw( "&#132;", 2, 3 ) );
    EXPECT_EQ( 0x0085, MISC::decode_spchar_number_raw( "&#133;", 2, 3 ) );
    EXPECT_EQ( 0x0086, MISC::decode_spchar_number_raw( "&#134;", 2, 3 ) );
    EXPECT_EQ( 0x0087, MISC::decode_spchar_number_raw( "&#135;", 2, 3 ) );
    EXPECT_EQ( 0x0088, MISC::decode_spchar_number_raw( "&#136;", 2, 3 ) );
    EXPECT_EQ( 0x0089, MISC::decode_spchar_number_raw( "&#137;", 2, 3 ) );
    EXPECT_EQ( 0x008A, MISC::decode_spchar_number_raw( "&#138;", 2, 3 ) );
    EXPECT_EQ( 0x008B, MISC::decode_spchar_number_raw( "&#139;", 2, 3 ) );
    EXPECT_EQ( 0x008C, MISC::decode_spchar_number_raw( "&#140;", 2, 3 ) );
    EXPECT_EQ( 0x008D, MISC::decode_spchar_number_raw( "&#141;", 2, 3 ) );
    EXPECT_EQ( 0x008E, MISC::decode_spchar_number_raw( "&#142;", 2, 3 ) );
    EXPECT_EQ( 0x008F, MISC::decode_spchar_number_raw( "&#143;", 2, 3 ) );
    EXPECT_EQ( 0x0090, MISC::decode_spchar_number_raw( "&#144;", 2, 3 ) );
    EXPECT_EQ( 0x0091, MISC::decode_spchar_number_raw( "&#145;", 2, 3 ) );
    EXPECT_EQ( 0x0092, MISC::decode_spchar_number_raw( "&#146;", 2, 3 ) );
    EXPECT_EQ( 0x0093, MISC::decode_spchar_number_raw( "&#147;", 2, 3 ) );
    EXPECT_EQ( 0x0094, MISC::decode_spchar_number_raw( "&#148;", 2, 3 ) );
    EXPECT_EQ( 0x0095, MISC::decode_spchar_number_raw( "&#149;", 2, 3 ) );
    EXPECT_EQ( 0x0096, MISC::decode_spchar_number_raw( "&#150;", 2, 3 ) );
    EXPECT_EQ( 0x0097, MISC::decode_spchar_number_raw( "&#151;", 2, 3 ) );
    EXPECT_EQ( 0x0098, MISC::decode_spchar_number_raw( "&#152;", 2, 3 ) );
    EXPECT_EQ( 0x0099, MISC::decode_spchar_number_raw( "&#153;", 2, 3 ) );
    EXPECT_EQ( 0x009A, MISC::decode_spchar_number_raw( "&#154;", 2, 3 ) );
    EXPECT_EQ( 0x009B, MISC::decode_spchar_number_raw( "&#155;", 2, 3 ) );
    EXPECT_EQ( 0x009C, MISC::decode_spchar_number_raw( "&#156;", 2, 3 ) );
    EXPECT_EQ( 0x009D, MISC::decode_spchar_number_raw( "&#157;", 2, 3 ) );
    EXPECT_EQ( 0x009E, MISC::decode_spchar_number_raw( "&#158;", 2, 3 ) );
    EXPECT_EQ( 0x009F, MISC::decode_spchar_number_raw( "&#159;", 2, 3 ) );
}

TEST_F(MISC_DecodeSpcharNumberRawTest, between_u007F_and_u009F_hexdecimal)
{
    EXPECT_EQ( 0x007F, MISC::decode_spchar_number_raw( "&#x7F;", 3, 2 ) );
    EXPECT_EQ( 0x0080, MISC::decode_spchar_number_raw( "&#x80;", 3, 2 ) );
    EXPECT_EQ( 0x0081, MISC::decode_spchar_number_raw( "&#x81;", 3, 2 ) );
    EXPECT_EQ( 0x0082, MISC::decode_spchar_number_raw( "&#x82;", 3, 2 ) );
    EXPECT_EQ( 0x0083, MISC::decode_spchar_number_raw( "&#x83;", 3, 2 ) );
    EXPECT_EQ( 0x0084, MISC::decode_spchar_number_raw( "&#x84;", 3, 2 ) );
    EXPECT_EQ( 0x0085, MISC::decode_spchar_number_raw( "&#x85;", 3, 2 ) );
    EXPECT_EQ( 0x0086, MISC::decode_spchar_number_raw( "&#x86;", 3, 2 ) );
    EXPECT_EQ( 0x0087, MISC::decode_spchar_number_raw( "&#x87;", 3, 2 ) );
    EXPECT_EQ( 0x0088, MISC::decode_spchar_number_raw( "&#x88;", 3, 2 ) );
    EXPECT_EQ( 0x0089, MISC::decode_spchar_number_raw( "&#x89;", 3, 2 ) );
    EXPECT_EQ( 0x008A, MISC::decode_spchar_number_raw( "&#x8A;", 3, 2 ) );
    EXPECT_EQ( 0x008B, MISC::decode_spchar_number_raw( "&#x8B;", 3, 2 ) );
    EXPECT_EQ( 0x008C, MISC::decode_spchar_number_raw( "&#x8C;", 3, 2 ) );
    EXPECT_EQ( 0x008D, MISC::decode_spchar_number_raw( "&#x8D;", 3, 2 ) );
    EXPECT_EQ( 0x008E, MISC::decode_spchar_number_raw( "&#x8E;", 3, 2 ) );
    EXPECT_EQ( 0x008F, MISC::decode_spchar_number_raw( "&#x8F;", 3, 2 ) );
    EXPECT_EQ( 0x0090, MISC::decode_spchar_number_raw( "&#x90;", 3, 2 ) );
    EXPECT_EQ( 0x0091, MISC::decode_spchar_number_raw( "&#x91;", 3, 2 ) );
    EXPECT_EQ( 0x0092, MISC::decode_spchar_number_raw( "&#x92;", 3, 2 ) );
    EXPECT_EQ( 0x0093, MISC::decode_spchar_number_raw( "&#x93;", 3, 2 ) );
    EXPECT_EQ( 0x0094, MISC::decode_spchar_number_raw( "&#x94;", 3, 2 ) );
    EXPECT_EQ( 0x0095, MISC::decode_spchar_number_raw( "&#x95;", 3, 2 ) );
    EXPECT_EQ( 0x0096, MISC::decode_spchar_number_raw( "&#x96;", 3, 2 ) );
    EXPECT_EQ( 0x0097, MISC::decode_spchar_number_raw( "&#x97;", 3, 2 ) );
    EXPECT_EQ( 0x0098, MISC::decode_spchar_number_raw( "&#x98;", 3, 2 ) );
    EXPECT_EQ( 0x0099, MISC::decode_spchar_number_raw( "&#x99;", 3, 2 ) );
    EXPECT_EQ( 0x009A, MISC::decode_spchar_number_raw( "&#x9A;", 3, 2 ) );
    EXPECT_EQ( 0x009B, MISC::decode_spchar_number_raw( "&#x9B;", 3, 2 ) );
    EXPECT_EQ( 0x009C, MISC::decode_spchar_number_raw( "&#x9C;", 3, 2 ) );
    EXPECT_EQ( 0x009D, MISC::decode_spchar_number_raw( "&#x9D;", 3, 2 ) );
    EXPECT_EQ( 0x009E, MISC::decode_spchar_number_raw( "&#x9E;", 3, 2 ) );
    EXPECT_EQ( 0x009F, MISC::decode_spchar_number_raw( "&#x9F;", 3, 2 ) );
}


class MISC_DecodeSpcharNumberTest : public ::testing::Test {};

TEST_F(MISC_DecodeSpcharNumberTest, result_ok)
{
    EXPECT_EQ( 0xC, MISC::decode_spchar_number( "&#xC;", 3, 1 ) );
    EXPECT_EQ( 32, MISC::decode_spchar_number( "&#32;", 2, 2 ) );
    EXPECT_EQ( 0x20, MISC::decode_spchar_number( "&#x20;", 3, 2 ) );
    EXPECT_EQ( 0xA0, MISC::decode_spchar_number( "&#xA0;", 3, 2 ) );
    EXPECT_EQ( 1234, MISC::decode_spchar_number( "&#1234;", 2, 4 ) );
    EXPECT_EQ( 0x1234, MISC::decode_spchar_number( "&#x1234;", 3, 4 ) );

    EXPECT_EQ( 0xD7FF, MISC::decode_spchar_number( "&#xD7FF;", 3, 4 ) );
    EXPECT_EQ( 0xE000, MISC::decode_spchar_number( "&#xE000;", 3, 4 ) );
    EXPECT_EQ( 0xFDCF, MISC::decode_spchar_number( "&#xFDCF;", 3, 4 ) );
    EXPECT_EQ( 0xFDF0, MISC::decode_spchar_number( "&#xFDF0;", 3, 4 ) );

    EXPECT_EQ( 1114109, MISC::decode_spchar_number( "&#1114109;", 2, 7 ) );
    EXPECT_EQ( 0x10FFFD, MISC::decode_spchar_number( "&#x10FFFD;", 3, 6 ) );
    EXPECT_EQ( 0x0abcde, MISC::decode_spchar_number( "&#xabcde;", 3, 5 ) );
}

TEST_F(MISC_DecodeSpcharNumberTest, result_error)
{
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#0;", 2, 1 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#8;", 2, 1 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xB;", 3, 1 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xD;", 3, 1 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#31;", 2, 2 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xD800;", 3, 4 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xDFFF;", 3, 4 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xFDD0;", 3, 4 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xFDEF;", 3, 4 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x110000;", 3, 6 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#1114112;", 2, 7 ) );
}

TEST_F(MISC_DecodeSpcharNumberTest, result_transform)
{
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x7F;", 3, 2 ) );
    EXPECT_EQ( 0x20AC, MISC::decode_spchar_number( "&#x80;", 3, 2 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x81;", 3, 2 ) );
    EXPECT_EQ( 0x201A, MISC::decode_spchar_number( "&#x82;", 3, 2 ) );
    EXPECT_EQ( 0x0192, MISC::decode_spchar_number( "&#x83;", 3, 2 ) );
    EXPECT_EQ( 0x201E, MISC::decode_spchar_number( "&#x84;", 3, 2 ) );
    EXPECT_EQ( 0x2026, MISC::decode_spchar_number( "&#x85;", 3, 2 ) );
    EXPECT_EQ( 0x2020, MISC::decode_spchar_number( "&#x86;", 3, 2 ) );
    EXPECT_EQ( 0x2021, MISC::decode_spchar_number( "&#x87;", 3, 2 ) );
    EXPECT_EQ( 0x02C6, MISC::decode_spchar_number( "&#x88;", 3, 2 ) );
    EXPECT_EQ( 0x2030, MISC::decode_spchar_number( "&#x89;", 3, 2 ) );
    EXPECT_EQ( 0x0160, MISC::decode_spchar_number( "&#x8A;", 3, 2 ) );
    EXPECT_EQ( 0x2039, MISC::decode_spchar_number( "&#x8B;", 3, 2 ) );
    EXPECT_EQ( 0x0152, MISC::decode_spchar_number( "&#x8C;", 3, 2 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x8D;", 3, 2 ) );
    EXPECT_EQ( 0x017D, MISC::decode_spchar_number( "&#x8E;", 3, 2 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x8F;", 3, 2 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x90;", 3, 2 ) );
    EXPECT_EQ( 0x2018, MISC::decode_spchar_number( "&#x91;", 3, 2 ) );
    EXPECT_EQ( 0x2019, MISC::decode_spchar_number( "&#x92;", 3, 2 ) );
    EXPECT_EQ( 0x201C, MISC::decode_spchar_number( "&#x93;", 3, 2 ) );
    EXPECT_EQ( 0x201D, MISC::decode_spchar_number( "&#x94;", 3, 2 ) );
    EXPECT_EQ( 0x2022, MISC::decode_spchar_number( "&#x95;", 3, 2 ) );
    EXPECT_EQ( 0x2013, MISC::decode_spchar_number( "&#x96;", 3, 2 ) );
    EXPECT_EQ( 0x2014, MISC::decode_spchar_number( "&#x97;", 3, 2 ) );
    EXPECT_EQ( 0x02DC, MISC::decode_spchar_number( "&#x98;", 3, 2 ) );
    EXPECT_EQ( 0x2122, MISC::decode_spchar_number( "&#x99;", 3, 2 ) );
    EXPECT_EQ( 0x0161, MISC::decode_spchar_number( "&#x9A;", 3, 2 ) );
    EXPECT_EQ( 0x203A, MISC::decode_spchar_number( "&#x9B;", 3, 2 ) );
    EXPECT_EQ( 0x0153, MISC::decode_spchar_number( "&#x9C;", 3, 2 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x9D;", 3, 2 ) );
    EXPECT_EQ( 0x017E, MISC::decode_spchar_number( "&#x9E;", 3, 2 ) );
    EXPECT_EQ( 0x0178, MISC::decode_spchar_number( "&#x9F;", 3, 2 ) );
}


class MISC_SanitizeNumericCharrefTest : public ::testing::Test {};

TEST_F(MISC_SanitizeNumericCharrefTest, null_character)
{
    char32_t out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0, &out ) );
    EXPECT_EQ( 0, out );
}

TEST_F(MISC_SanitizeNumericCharrefTest, carriage_return)
{
    char32_t out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x000D, &out ) );
    EXPECT_EQ( 0, out );
}

TEST_F(MISC_SanitizeNumericCharrefTest, ascii_whitespace)
{
    char32_t out = 0;
    EXPECT_EQ( 0x0009, MISC::sanitize_numeric_charref( 0x0009, &out ) );
    EXPECT_EQ( 0, out );

    out = 0;
    EXPECT_EQ( 0x000A, MISC::sanitize_numeric_charref( 0x000A, &out ) );
    EXPECT_EQ( 0, out );

    out = 0;
    EXPECT_EQ( 0x000C, MISC::sanitize_numeric_charref( 0x000C, &out ) );
    EXPECT_EQ( 0, out );

    out = 0;
    EXPECT_EQ( 0x0020, MISC::sanitize_numeric_charref( 0x0020, &out ) );
    EXPECT_EQ( 0, out );
}

TEST_F(MISC_SanitizeNumericCharrefTest, out_of_range)
{
    char32_t out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x110000, &out ) );
    EXPECT_EQ( 0, out );
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x1000000, nullptr ) );
}

TEST_F(MISC_SanitizeNumericCharrefTest, high_surrogate)
{
    // 数が多いため網羅していない
    char32_t out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0xD800, &out ) );
    EXPECT_EQ( 0xD800, out );

    out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0xDBFF, &out ) );
    EXPECT_EQ( 0xDBFF, out );
}

TEST_F(MISC_SanitizeNumericCharrefTest, low_surrogate)
{
    // 数が多いため網羅していない
    char32_t out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0xDC00, &out ) );
    EXPECT_EQ( 0, out );

    out = 0;
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0xDFFF, &out ) );
    EXPECT_EQ( 0, out );
}

TEST_F(MISC_SanitizeNumericCharrefTest, noncharacter)
{
    // 数が多いため網羅していない
    constexpr char32_t nonchars[] = { 0xFDD0, 0xFDEF, 0xFFFE, 0xFFFF,
                                      0x1FFFE, 0x1FFFF, 0x10FFFE, 0x10FFFF };
    for( const char32_t uch : nonchars ) {
        char32_t out = 0;
        EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( uch, &out ) );
        EXPECT_EQ( 0, out );
    }
}

TEST_F(MISC_SanitizeNumericCharrefTest, between_u007F_and_u009F)
{
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x007F, nullptr ) );
    EXPECT_EQ( 0x20AC, MISC::sanitize_numeric_charref( 0x0080, nullptr ) );
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x0081, nullptr ) );
    EXPECT_EQ( 0x201A, MISC::sanitize_numeric_charref( 0x0082, nullptr ) );
    EXPECT_EQ( 0x0192, MISC::sanitize_numeric_charref( 0x0083, nullptr ) );
    EXPECT_EQ( 0x201E, MISC::sanitize_numeric_charref( 0x0084, nullptr ) );
    EXPECT_EQ( 0x2026, MISC::sanitize_numeric_charref( 0x0085, nullptr ) );
    EXPECT_EQ( 0x2020, MISC::sanitize_numeric_charref( 0x0086, nullptr ) );
    EXPECT_EQ( 0x2021, MISC::sanitize_numeric_charref( 0x0087, nullptr ) );
    EXPECT_EQ( 0x02C6, MISC::sanitize_numeric_charref( 0x0088, nullptr ) );
    EXPECT_EQ( 0x2030, MISC::sanitize_numeric_charref( 0x0089, nullptr ) );
    EXPECT_EQ( 0x0160, MISC::sanitize_numeric_charref( 0x008A, nullptr ) );
    EXPECT_EQ( 0x2039, MISC::sanitize_numeric_charref( 0x008B, nullptr ) );
    EXPECT_EQ( 0x0152, MISC::sanitize_numeric_charref( 0x008C, nullptr ) );
    EXPECT_EQ( 0x017D, MISC::sanitize_numeric_charref( 0x008E, nullptr ) );
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x008F, nullptr ) );
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x0090, nullptr ) );
    EXPECT_EQ( 0x2018, MISC::sanitize_numeric_charref( 0x0091, nullptr ) );
    EXPECT_EQ( 0x2019, MISC::sanitize_numeric_charref( 0x0092, nullptr ) );
    EXPECT_EQ( 0x201C, MISC::sanitize_numeric_charref( 0x0093, nullptr ) );
    EXPECT_EQ( 0x201D, MISC::sanitize_numeric_charref( 0x0094, nullptr ) );
    EXPECT_EQ( 0x2022, MISC::sanitize_numeric_charref( 0x0095, nullptr ) );
    EXPECT_EQ( 0x2013, MISC::sanitize_numeric_charref( 0x0096, nullptr ) );
    EXPECT_EQ( 0x2014, MISC::sanitize_numeric_charref( 0x0097, nullptr ) );
    EXPECT_EQ( 0x02DC, MISC::sanitize_numeric_charref( 0x0098, nullptr ) );
    EXPECT_EQ( 0x2122, MISC::sanitize_numeric_charref( 0x0099, nullptr ) );
    EXPECT_EQ( 0x0161, MISC::sanitize_numeric_charref( 0x009A, nullptr ) );
    EXPECT_EQ( 0x203A, MISC::sanitize_numeric_charref( 0x009B, nullptr ) );
    EXPECT_EQ( 0x0153, MISC::sanitize_numeric_charref( 0x009C, nullptr ) );
    EXPECT_EQ( 0xFFFD, MISC::sanitize_numeric_charref( 0x009D, nullptr ) );
    EXPECT_EQ( 0x017E, MISC::sanitize_numeric_charref( 0x009E, nullptr ) );
    EXPECT_EQ( 0x0178, MISC::sanitize_numeric_charref( 0x009F, nullptr ) );
}


class AsciiIgnoreCaseFindTest : public ::testing::Test {};

TEST_F(AsciiIgnoreCaseFindTest, empty)
{
    EXPECT_EQ( 0, MISC::ascii_ignore_case_find( std::string{}, std::string{} ) );
    EXPECT_EQ( std::string::npos, MISC::ascii_ignore_case_find( std::string{}, std::string{}, 10 ) );
}

TEST_F(AsciiIgnoreCaseFindTest, not_match)
{
    const std::string haystack = "spam ham eggs";
    EXPECT_EQ( std::string::npos, MISC::ascii_ignore_case_find( haystack, "foo bar" ) );
}

TEST_F(AsciiIgnoreCaseFindTest, out_of_bounds)
{
    const std::string haystack = "out of bounds";
    EXPECT_EQ( std::string::npos, MISC::ascii_ignore_case_find( haystack, "needle", haystack.size() + 1 ) );
}

TEST_F(AsciiIgnoreCaseFindTest, match_same_case)
{
    const std::string haystack = "helloworld";
    EXPECT_EQ( 0, MISC::ascii_ignore_case_find( haystack, "hello" ) );
    EXPECT_EQ( 5, MISC::ascii_ignore_case_find( haystack, "world" ) );
}

TEST_F(AsciiIgnoreCaseFindTest, match_lowercase)
{
    const std::string haystack_lower = "helloworld";
    EXPECT_EQ( 0, MISC::ascii_ignore_case_find( haystack_lower, "HELLO" ) );
    EXPECT_EQ( 5, MISC::ascii_ignore_case_find( haystack_lower, "WORLD" ) );
}

TEST_F(AsciiIgnoreCaseFindTest, match_uppercase)
{
    const std::string haystack_upper = "HELLOWORLD";
    EXPECT_EQ( 0, MISC::ascii_ignore_case_find( haystack_upper, "hello" ) );
    EXPECT_EQ( 5, MISC::ascii_ignore_case_find( haystack_upper, "world" ) );
}

TEST_F(AsciiIgnoreCaseFindTest, match_mix_case)
{
    const std::string haystack_mix = "HeLlOwOrLd";
    EXPECT_EQ( 0, MISC::ascii_ignore_case_find( haystack_mix, "hElLo" ) );
    EXPECT_EQ( 5, MISC::ascii_ignore_case_find( haystack_mix, "WoRlD" ) );
}

TEST_F(AsciiIgnoreCaseFindTest, match_lead_word)
{
    const std::string haystack = "foo bar baz bar";
    EXPECT_EQ( 4, MISC::ascii_ignore_case_find( haystack, "bar" ) );
    EXPECT_EQ( 12, MISC::ascii_ignore_case_find( haystack, "BAR", 5 ) );
}

} // namespace
