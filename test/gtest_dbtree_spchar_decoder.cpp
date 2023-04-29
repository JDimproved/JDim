#include "dbtree/node.h"
#include "dbtree/spchar_decoder.h"
#include "dbtree/spchar_tbl.h"

#include "gtest/gtest.h"


namespace {

class DBTREE_DecodeCharNumberTest : public ::testing::Test {};

TEST_F(DBTREE_DecodeCharNumberTest, non_charref)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_number( "hello world", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, prefix_only)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    // 数値文字参照の3文字目をチェックするのでヌル終端を除いて長さ2未満の文字列は未定義動作になる
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_number( "&#", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, decimal_hiragana_a)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#12354;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE3\x81\x82", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, hexadecimal_hiragana_a)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x3042;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE3\x81\x82", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, non_digits)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_number( "&#qux;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, invalid_sequence)
{
    // '#' の後に続く数字のみ考慮する
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#12354hoge7;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\xE3\x81\x82", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x3042hoge9;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\xE3\x81\x82", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, without_semicoron)
{
    // '#' の後に続く数字のみ考慮する
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#123", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "{", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x7b", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "{", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, padding_zeros)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#0097;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "a", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x0041;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "A", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, zwsp_u200B)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // zwsp(U+200B) は今のところ空文字列にする
    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_number( "&#X200B;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, zwnj_zwj_lrm_rlm)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#X200C;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8C", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x200d;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8D", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x200E;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8E", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#X200f;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8F", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, line_separator_u2028)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // U+2028 LINE SEPARATOR を描画処理に渡すと改行が乱れるため空白に置き換える (webブラウザと同じ挙動)
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( "&#x2028;", n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( " ", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, do_not_correct_surrogate_pair)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // U+1F600 GRINNING FACE
    constexpr const char* emoji = "&#55357;&#56832;";
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( emoji, n_in, out_char, n_out, false ) );
    EXPECT_EQ( 8, n_in );
    // サロゲートペアをデコードしないときは1つ目の数値文字参照だけ U+FFFD REPLACEMENT CHARACTER に変換する
    EXPECT_STREQ( "\xEF\xBF\xBD", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, correct_surrogate_pair)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    constexpr bool correct_surroagete = true;
    // U+1F600 GRINNING FACE
    constexpr const char* emoji = "&#55357;&#56832;";
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( emoji, n_in, out_char, n_out, correct_surroagete ) );
    EXPECT_EQ( 16, n_in );
    EXPECT_STREQ( "\xf0\x9f\x98\x80", out_char );
    EXPECT_EQ( 4, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, only_high_surrogate)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    constexpr bool correct_surroagete = true;
    constexpr const char* chrefs = "&#55357;";
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( chrefs, n_in, out_char, n_out, correct_surroagete ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xEF\xBF\xBD", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNumberTest, following_no_low_surrogate)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    constexpr bool correct_surroagete = true;
    constexpr const char* chrefs = "&#55357;&#12354;";
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_number( chrefs, n_in, out_char, n_out, correct_surroagete ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xEF\xBF\xBD", out_char );
    EXPECT_EQ( 3, n_out );
}


class DBTREE_DecodeCharNameTest : public ::testing::Test {};

TEST_F(DBTREE_DecodeCharNameTest, ampersand_only)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    // 文字実体参照の2文字目をチェックするのでヌル終端を除いて長さ0の文字列は未定義動作になる
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "&", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, non_charref)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "hello world", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, invalid_name)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "&foobar;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, without_semicoron)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "&hearts", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u0022)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&quot;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\x22", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&QUOT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\x22", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u0026)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&amp;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x26", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&AMP;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x26", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u0391)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&Alpha;", n_in, out_char, n_out ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\xCE\x91", out_char );
    EXPECT_EQ( 2, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u2233)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&awconint;", n_in, out_char, n_out ) );
    EXPECT_EQ( 10, n_in );
    EXPECT_STREQ( "\xE2\x88\xB3", out_char );
    EXPECT_EQ( 3, n_out );

    // 一番長い名前付き文字参照
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&CounterClockwiseContourIntegral;", n_in, out_char, n_out ) );
    EXPECT_EQ( 33, n_in );
    EXPECT_STREQ( "\xE2\x88\xB3", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, lt_cases)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&lt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3C", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&LT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3C", out_char );
    EXPECT_EQ( 1, n_out );

    // &LT; や &lt; と異なる文字
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&Lt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\xE2\x89\xAA", out_char );
    EXPECT_EQ( 3, n_out );

    // 存在しない
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "&lT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, gt_cases)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&gt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3E", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&GT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3E", out_char );
    EXPECT_EQ( 1, n_out );


    // &GT; や &gt; と異なる文字
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&Gt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\xE2\x89\xAB", out_char );
    EXPECT_EQ( 3, n_out );

    // 存在しない
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char_name( "&gT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u1D504)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&Afr;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\xF0\x9D\x94\x84", out_char );
    EXPECT_EQ( 4, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u003D_u20E5)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&bne;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x3D\xE2\x83\xA5", out_char );
    EXPECT_EQ( 4, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u0066_u006A)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&fjlig;", n_in, out_char, n_out ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\x66\x6A", out_char );
    EXPECT_EQ( 2, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u222D_u0331)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&race;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\xE2\x88\xBD\xCC\xB1", out_char );
    EXPECT_EQ( 5, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u228B_uFE00)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&vsupne;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x8A\x8B\xEF\xB8\x80", out_char );
    EXPECT_EQ( 6, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, convert_to_zwsp)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // ZWSP(U+200B)に変換される文字参照は今のところ空文字列にする
    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&NegativeMediumSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 21, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&NegativeThickSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 20, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&NegativeThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 19, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&NegativeVeryThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 23, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&ZeroWidthSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 16, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, zwnj_zwj_lrm_rlm)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&zwnj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\xE2\x80\x8C", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&zwj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\xE2\x80\x8D", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&lrm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\xE2\x80\x8E", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&rlm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\xE2\x80\x8F", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u200A)
{
    // ゼロ幅文字 ZWSP(U+200B) の処理がはみ出していないか境界チェック
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&VeryThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 15, n_in );
    EXPECT_STREQ( "\xE2\x80\x8A", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&hairsp;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8A", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharNameTest, u2010)
{
    // ゼロ幅文字 rlm(U+200F) の処理がはみ出していないか境界チェック
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&hyphen;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x90", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char_name( "&dash;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\xE2\x80\x90", out_char );
    EXPECT_EQ( 3, n_out );
}

} // namespace
