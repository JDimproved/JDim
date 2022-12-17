#include "dbtree/node.h"
#include "dbtree/spchar_decoder.h"
#include "dbtree/spchar_tbl.h"

#include "gtest/gtest.h"


namespace {

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

    // zwnj(U+200C), zwj(U+200D), lrm(U+200E), rlm(U+200F) は今のところ空文字列にする(zwspにする)
    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&zwnj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&zwj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&lrm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char_name( "&rlm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
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
