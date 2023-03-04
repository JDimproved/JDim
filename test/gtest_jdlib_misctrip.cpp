// License: GPL-2.0
// Copyright (C) 2019, JDimproved project

#include "jdlib/misctrip.h"

#include "gtest/gtest.h"


namespace {

class GetTripTest : public ::testing::Test {};

// 実際に運用されているShift_JISエンコーディングでテスト
// ASCII以外の文字はShift_JISに変換されるのでバイト数に注意すること
// 戻り値のテストデータはWikipediaのperlスクリプトを使って計算した

// ヘルパー関数
inline static std::string get_trip_sjis( std::string u8key )
{
    return MISC::get_trip( u8key, Encoding::sjis );
}

TEST_F(GetTripTest, trip8_sjis_empty)
{
    EXPECT_EQ( "", get_trip_sjis( u8"" ) );
}

TEST_F(GetTripTest, trip8_sjis_A)
{
    EXPECT_EQ( "hRJ9Ya./t.", get_trip_sjis( u8"A" ) );
}

TEST_F(GetTripTest, trip8_sjis_hello7)
{
    EXPECT_EQ( "/wfpxFEFeQ", get_trip_sjis( u8"hellowo" ) );
}

TEST_F(GetTripTest, trip8_sjis_hello8)
{
    EXPECT_EQ( "d75etXAowg", get_trip_sjis( u8"hellowor" ) );
}

TEST_F(GetTripTest, trip8_sjis_hello11)
{
    // 11バイトまでは従来の方式(9〜11バイト目は無視される)
    EXPECT_EQ( "d75etXAowg", get_trip_sjis( u8"helloworld!" ) );
}

TEST_F(GetTripTest, trip8_sjis_yojijukugo)
{
    EXPECT_EQ( "sX.SlNvMe.", get_trip_sjis( u8"四字熟語" ) );
}

TEST_F(GetTripTest, trip8_sjis_hex_less_12)
{
    // シャープで始まる16進数キーでも12バイト未満なら従来の方式
    EXPECT_EQ( "RTDIJZhD3g", get_trip_sjis( u8"#0123456789" ) );
}

TEST_F(GetTripTest, trip8_sjis_dollar_less_12)
{
    // ドル記号で始まるキーでも12バイト未満なら従来の方式
    EXPECT_EQ( "46g6cHndYk", get_trip_sjis( u8"$0123456789" ) );
}

TEST_F(GetTripTest, trip12_sjis_hello12)
{
    // 12バイト以上は新方式
    EXPECT_EQ( "xwumaTFfu1OK", get_trip_sjis( u8"helloworld!?" ) );
}

TEST_F(GetTripTest, trip12_sjis_jdimprovedproject)
{
    // 新方式の+は.に変換される
    EXPECT_EQ( "Y7G9gYfXrr6.", get_trip_sjis( u8"jdimprovedproject" ) );
}

TEST_F(GetTripTest, trip12_sjis_hex)
{
    // 新方式 16進数のキー
    EXPECT_EQ( "ClNHFHdYIw", get_trip_sjis( u8"#0123456789abcdef" ) );
}

TEST_F(GetTripTest, trip12_sjis_non_hex)
{
    // 念の為トライグラフ対策としてエスケープ
    EXPECT_EQ( "\?\?\?", get_trip_sjis( u8"#あいうえおか" ) );
}

TEST_F(GetTripTest, trip12_sjis_dollar)
{
    EXPECT_EQ( "\?\?\?", get_trip_sjis( u8"$将来の拡張用" ) );
}

} // namespace
