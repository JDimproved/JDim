// SPDX-License-Identifier: GPL-2.0-only

#include "dbtree/nodetreebase.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

class NodeTreeBase_RemoveImenuTest : public ::testing::Test {};

TEST_F(NodeTreeBase_RemoveImenuTest, empty_string)
{
    std::string inout;
    EXPECT_FALSE( DBTREE::NodeTreeBase::remove_imenu( inout ) );
    EXPECT_EQ( "", inout );
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
        { "http://machi.to/bbs/link.cgi?URL=", "http://machi.to/bbs/link.cgi?URL=" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_FALSE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_ime_nu)
{
    constexpr const char* test_data[][2] = {
        { "http://ime.nu/foobar.baz", "http://foobar.baz" },
        { "https://ime.nu/foobar.baz", "https://foobar.baz" },
        { "https://ime.nu/http://foobar.baz", "http://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_ime_st)
{
    constexpr const char* test_data[][2] = {
        { "http://ime.st/foobar.baz", "http://foobar.baz" },
        { "https://ime.st/foobar.baz", "https://foobar.baz" },
        { "http://ime.nu/https://foobar.baz", "https://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_nun_nu)
{
    constexpr const char* test_data[][2] = {
        { "http://nun.nu/foobar.baz", "http://foobar.baz" },
        { "https://nun.nu/foobar.baz", "https://foobar.baz" },
        { "https://nun.nu/http://foobar.baz", "http://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_pinktower_com)
{
    constexpr const char* test_data[][2] = {
        { "http://pinktower.com/foobar.baz", "http://foobar.baz" },
        { "https://pinktower.com/foobar.baz", "https://foobar.baz" },
        { "http://pinktower.com/https://foobar.baz", "https://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_jump_5ch_net)
{
    constexpr const char* test_data[][2] = {
        { "http://jump.5ch.net/?http://foobar.baz", "http://foobar.baz" },
        { "https://jump.5ch.net/?https://foobar.baz", "https://foobar.baz" },
        { "http://jump.5ch.net/?https://foobar.baz", "https://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_jump_2ch_net)
{
    constexpr const char* test_data[][2] = {
        { "http://jump.2ch.net/?http://foobar.baz", "http://foobar.baz" },
        { "https://jump.2ch.net/?https://foobar.baz", "https://foobar.baz" },
        { "http://jump.2ch.net/?https://foobar.baz", "https://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

TEST_F(NodeTreeBase_RemoveImenuTest, single_machi_to_bbs_link_cgi)
{
    constexpr const char* test_data[][2] = {
        { "http://machi.to/bbs/link.cgi?URL=https://foobar.baz", "https://foobar.baz" },
        { "https://machi.to/bbs/link.cgi?URL=http://foobar.baz", "http://foobar.baz" },
        { "http://machi.to/bbs/link.cgi?URL=foobar.baz", "http://foobar.baz" },
    };

    std::string buffer;
    for( auto [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}

} // namespace
