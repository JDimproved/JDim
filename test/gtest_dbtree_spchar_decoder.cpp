#include "dbtree/node.h"
#include "dbtree/spchar_decoder.h"
#include "dbtree/spchar_tbl.h"

#include "gtest/gtest.h"


namespace {

class DBTREE_DecodeCharTest : public ::testing::Test {};

TEST_F(DBTREE_DecodeCharTest, named_empty_string)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_non_charref)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "hello world", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_invalid_name)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "&foobar;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_without_semicoron)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "&hearts", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u0022)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&quot;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\x22", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&QUOT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\x22", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u0026)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&amp;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x26", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&AMP;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x26", out_char );
    EXPECT_EQ( 1, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u0391)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&Alpha;", n_in, out_char, n_out ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\xCE\x91", out_char );
    EXPECT_EQ( 2, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u2233)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&awconint;", n_in, out_char, n_out ) );
    EXPECT_EQ( 10, n_in );
    EXPECT_STREQ( "\xE2\x88\xB3", out_char );
    EXPECT_EQ( 3, n_out );

    // 一番長い名前付き文字参照
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&CounterClockwiseContourIntegral;", n_in, out_char, n_out ) );
    EXPECT_EQ( 33, n_in );
    EXPECT_STREQ( "\xE2\x88\xB3", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_lt_cases)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&lt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3C", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&LT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3C", out_char );
    EXPECT_EQ( 1, n_out );

    // &LT; や &lt; と異なる文字
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&Lt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\xE2\x89\xAA", out_char );
    EXPECT_EQ( 3, n_out );

    // 存在しない
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "&lT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_gt_cases)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&gt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3E", out_char );
    EXPECT_EQ( 1, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&GT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\x3E", out_char );
    EXPECT_EQ( 1, n_out );


    // &GT; や &gt; と異なる文字
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&Gt;", n_in, out_char, n_out ) );
    EXPECT_EQ( 4, n_in );
    EXPECT_STREQ( "\xE2\x89\xAB", out_char );
    EXPECT_EQ( 3, n_out );

    // 存在しない
    EXPECT_EQ( DBTREE::NODE_NONE, DBTREE::decode_char( "&gT;", n_in, out_char, n_out ) );
    EXPECT_EQ( 0, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u1D504)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&Afr;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\xF0\x9D\x94\x84", out_char );
    EXPECT_EQ( 4, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u003D_u20E5)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&bne;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "\x3D\xE2\x83\xA5", out_char );
    EXPECT_EQ( 4, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u0066_u006A)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&fjlig;", n_in, out_char, n_out ) );
    EXPECT_EQ( 7, n_in );
    EXPECT_STREQ( "\x66\x6A", out_char );
    EXPECT_EQ( 2, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u222D_u0331)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&race;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\xE2\x88\xBD\xCC\xB1", out_char );
    EXPECT_EQ( 5, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u228B_uFE00)
{
    char out_char[16]{};
    int n_in;
    int n_out;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&vsupne;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x8A\x8B\xEF\xB8\x80", out_char );
    EXPECT_EQ( 6, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_convert_to_zwsp)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // ZWSP(U+200B)に変換される文字参照は今のところ空文字列にする
    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&NegativeMediumSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 21, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&NegativeThickSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 20, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&NegativeThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 19, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&NegativeVeryThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 23, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&ZeroWidthSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 16, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_zwnj_zwj_lrm_rlm)
{
    char out_char[16]{};
    int n_in;
    int n_out;

    // zwnj(U+200C), zwj(U+200D), lrm(U+200E), rlm(U+200F) は今のところ空文字列にする(zwspにする)
    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&zwnj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&zwj;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&lrm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );

    EXPECT_EQ( DBTREE::NODE_ZWSP, DBTREE::decode_char( "&rlm;", n_in, out_char, n_out ) );
    EXPECT_EQ( 5, n_in );
    EXPECT_STREQ( "", out_char );
    EXPECT_EQ( 0, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u200A)
{
    // ゼロ幅文字の処理に対する境界チェック
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&VeryThinSpace;", n_in, out_char, n_out ) );
    EXPECT_EQ( 15, n_in );
    EXPECT_STREQ( "\xE2\x80\x8A", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&hairsp;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x8A", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharTest, named_u2010)
{
    // ゼロ幅文字の処理に対する境界チェック
    char out_char[16]{};
    int n_in;
    int n_out;

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&hyphen;", n_in, out_char, n_out ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( "\xE2\x80\x90", out_char );
    EXPECT_EQ( 3, n_out );

    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&dash;", n_in, out_char, n_out ) );
    EXPECT_EQ( 6, n_in );
    EXPECT_STREQ( "\xE2\x80\x90", out_char );
    EXPECT_EQ( 3, n_out );
}

} // namespace
