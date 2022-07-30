// SPDX-License-Identifier: GPL-2.0-only

#include "dbtree/nodetreebase.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

class NodeTreeBase_RemoveImenuTest : public ::testing::Test {};

TEST_F(NodeTreeBase_RemoveImenuTest, empty_string)
{
    char inout[] = "";
    EXPECT_FALSE( DBTREE::NodeTreeBase::remove_imenu( inout ) );
    EXPECT_STREQ( "", inout );
}

TEST_F(NodeTreeBase_RemoveImenuTest, not_remove)
{
    constexpr const char* test_data[][2] = {
        { "ftp://ime.nu/foobar.baz", "ftp://ime.nu/foobar.baz" },
        { "http://example.test/foobar.baz", "http://example.test/foobar.baz" },

        { "https://ime.nu/", "https://ime.nu/" },
        { "https://ime.st/", "https://ime.st/" },
        { "https://nun.nu/", "https://nun.nu/" },
        { "https://jump.5ch.net/?", "https://jump.5ch.net/?" },
        { "https://jump.2ch.net/?", "https://jump.2ch.net/?" },
        { "https://pinktower.com/", "https://pinktower.com/" },
    };

    char buffer[128];
    for( auto [input, expect] : test_data ) {
        std::strcpy( buffer, input );
        EXPECT_FALSE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_STREQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_ime_nu)
{
    constexpr const char* test_data[][2] = {
        { "http://ime.nu/foobar.baz", "http://foobar.baz" },
        { "https://ime.nu/foobar.baz", "https://foobar.baz" },
    };

    char buffer[128];
    for( auto [input, expect] : test_data ) {
        std::strcpy( buffer, input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_STREQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_ime_st)
{
    constexpr const char* test_data[][2] = {
        { "http://ime.st/foobar.baz", "http://foobar.baz" },
        { "https://ime.st/foobar.baz", "https://foobar.baz" },
    };

    char buffer[128];
    for( auto [input, expect] : test_data ) {
        std::strcpy( buffer, input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_STREQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_nun_nu)
{
    constexpr const char* test_data[][2] = {
        { "http://nun.nu/foobar.baz", "http://foobar.baz" },
        { "https://nun.nu/foobar.baz", "https://foobar.baz" },
    };

    char buffer[128];
    for( auto [input, expect] : test_data ) {
        std::strcpy( buffer, input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_STREQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_pinktower_com)
{
    constexpr const char* test_data[][2] = {
        { "http://pinktower.com/foobar.baz", "http://foobar.baz" },
        { "https://pinktower.com/foobar.baz", "https://foobar.baz" },
    };

    char buffer[128];
    for( auto [input, expect] : test_data ) {
        std::strcpy( buffer, input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_STREQ( expect, buffer );
    }
}


class NodeTreeBase_ConvertAmpTest : public ::testing::Test {};

TEST_F(NodeTreeBase_ConvertAmpTest, empty_string)
{
    char buffer[] = "";
    EXPECT_EQ( 0, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, no_amp)
{
    char buffer[] = "hello world!";
    EXPECT_EQ( 12, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "hello world!", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, missing_ampersand)
{
    char buffer[] = "helloamp;world!";
    EXPECT_EQ( 15, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "helloamp;world!", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, missing_semicolon)
{
    char buffer[] = "hello&ampworld!";
    EXPECT_EQ( 15, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "hello&ampworld!", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, one_amp)
{
    char buffer[] = "hello&amp;world!";
    EXPECT_EQ( 12, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "hello&world!", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, multi_amp)
{
    char buffer[] = "&amp;hello&amp;world&amp;";
    EXPECT_EQ( 13, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "&hello&world&", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, amp_amp)
{
    char buffer[] = "&amp;amp; &amp;&amp;";
    EXPECT_EQ( 8, DBTREE::NodeTreeBase::convert_amp( buffer, std::strlen( buffer ) ) );
    EXPECT_STREQ( "&amp; &&", buffer );
}

TEST_F(NodeTreeBase_ConvertAmpTest, include_nul)
{
    char buffer[] = "hello&amp;\0&amp;world!";
    EXPECT_EQ( 14, DBTREE::NodeTreeBase::convert_amp( buffer, 22 ) );
    EXPECT_STREQ( "hello&\0&world!", buffer );
}

} // namespace
