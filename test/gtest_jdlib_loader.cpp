// SPDX-License-Identifier: GPL-2.0-only

#include "jdlib/loader.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

struct ChunkedDecoderDataSet
{
    const char* input{};
    const char* output{};
    std::size_t output_size{};
    bool is_completed{};
};

class JDLIB_ChunkedDecoder_Decode : public ::testing::Test {};

TEST_F(JDLIB_ChunkedDecoder_Decode, empty)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "";
    std::size_t size = 0;
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "" );
    EXPECT_EQ( size, 0 );
    EXPECT_FALSE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, last_chunk_only)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "0\r\n";
    std::size_t size = 3;
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "" );
    EXPECT_EQ( size, 0 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, one_chunk)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "a\r\nhelloworld\r\n0\r\n\r\n";
    std::size_t size = std::strlen( buf );
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "helloworld" );
    EXPECT_EQ( size, 10 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, one_chunk_including_crlf)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "C\r\nhello\r\nworld\r\n0\r\n\r\n";
    std::size_t size = std::strlen( buf );
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "hello\r\nworld" );
    EXPECT_EQ( size, 12 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, multiple_chunks)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "5\r\nhello\r\n5\r\nworld\r\n0\r\n\r\n";
    std::size_t size = std::strlen( buf );
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "helloworld" );
    EXPECT_EQ( size, 10 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, chunk_ext)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "5;foo=bar\r\nhello\r\n5;baz\r\nworld\r\n0\r\n\r\n";
    std::size_t size = std::strlen( buf );
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "helloworld" );
    EXPECT_EQ( size, 10 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, traier_part)
{
    JDLIB::ChunkedDecoder decoder;
    char buf[] = "5\r\nhello\r\n5\r\nworld\r\n0\r\nAdditional: Data\r\n\r\n";
    std::size_t size = std::strlen( buf );
    EXPECT_TRUE( decoder.decode( buf, size ) );
    EXPECT_STREQ( buf, "helloworld" );
    EXPECT_EQ( size, 10 );
    EXPECT_TRUE( decoder.is_completed() );
}

TEST_F(JDLIB_ChunkedDecoder_Decode, multiple_time_feed_chunks)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "5\r\nhello\r\n", "hello", 5, false },
        { "5\r\nworld\r\n", "world", 5, false },
        { "0\r\n\r\n", "", 0, true },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, output, output_size, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_TRUE( decoder.decode( buf, size ) );
        EXPECT_STREQ( buf, output );
        EXPECT_EQ( size, output_size );
        EXPECT_EQ( decoder.is_completed(), is_completed );
    }
}

TEST_F(JDLIB_ChunkedDecoder_Decode, multiple_time_feed_crlf_fragmentation)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "5\r\nQuick", "Quick", 5, false },
        { "\r\n5\r\nBrown\r", "Brown", 5, false },
        { "\n3\r\nFox\r\n", "Fox", 3, false },
        { "0\r\n\r\n", "", 0, true },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, output, output_size, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_TRUE( decoder.decode( buf, size ) );
        EXPECT_STREQ( buf, output );
        EXPECT_EQ( size, output_size );
        EXPECT_EQ( decoder.is_completed(), is_completed );
    }
}

TEST_F(JDLIB_ChunkedDecoder_Decode, multiple_time_feed_body_fragmentation)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "5\r", "", 0, false },
        { "\nQuick\r\n5\r\nB", "QuickB", 6, false },
        { "rown\r\n3\r\nFox\r\n5", "rownFox", 7, false },
        { "\r\nJumps\r\n4\r\nOve", "JumpsOve", 8, false },
        { "r\r\n0\r", "r", 1, false },
        { "\n\r\n", "", 0, true },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, output, output_size, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_TRUE( decoder.decode( buf, size ) );
        EXPECT_STREQ( buf, output );
        EXPECT_EQ( size, output_size );
        EXPECT_EQ( decoder.is_completed(), is_completed );
    }
}

TEST_F(JDLIB_ChunkedDecoder_Decode, fail_pase_size)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "5\r\nQuick\r\n\r\n" },
        { "5\r\nQuick\r\nZ\r\nBrown\r\n0\r\n\r\n" },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, unused_1, unused_2, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_FALSE( decoder.decode( buf, size ) );
        EXPECT_FALSE( decoder.is_completed() );
        decoder.clear();
    }
}

TEST_F(JDLIB_ChunkedDecoder_Decode, fail_pase_body_cr_lf)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "5\r\nQuick\n0\r\n" },
        { "5\r\nQuick\r\n5\r\nBrown\r0\r\n\r\n" },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, unused_1, unused_2, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_FALSE( decoder.decode( buf, size ) );
        EXPECT_FALSE( decoder.is_completed() );
        decoder.clear();
    }
}

TEST_F(JDLIB_ChunkedDecoder_Decode, call_again_after_completed)
{
    constexpr const ChunkedDecoderDataSet chunks[] = {
        { "F\r\nQuick Brown Fox\r\n0\r\n", "Quick Brown Fox", 15, true },
        { "F\r\nQuick Brown Fox\r\n0\r\n", "", 0, true },
    };
    JDLIB::ChunkedDecoder decoder;
    char buf[64];
    std::size_t size;
    for( auto [input, output, output_size, is_completed] : chunks ) {
        std::strcpy( buf, input );
        size = std::strlen( buf );
        EXPECT_TRUE( decoder.decode( buf, size ) );
        EXPECT_STREQ( buf, output );
        EXPECT_EQ( size, output_size );
        EXPECT_TRUE( decoder.is_completed() );
    }
}

} // namespace
