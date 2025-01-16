// SPDX-License-Identifier: GPL-2.0-or-later

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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
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
    for( auto& [input, expect] : test_data ) {
        buffer.assign( input );
        EXPECT_TRUE( DBTREE::NodeTreeBase::remove_imenu( buffer ) );
        EXPECT_EQ( expect, buffer );
    }
}


class DBTREE_NodeTreeBase_GetAboneReasonTest : public ::testing::Test {};

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_none)
{
    EXPECT_STREQ( "", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::none ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_res)
{
    EXPECT_STREQ( "あぼ〜ん [NG レス番号]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::res ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_noid)
{
    EXPECT_STREQ( "あぼ〜ん [ID無し]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::noid ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_id_thread)
{
    EXPECT_STREQ( "あぼ〜ん [NG ID:スレ]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::id_thread ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_id_board)
{
    EXPECT_STREQ( "あぼ〜ん [NG ID:板]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::id_board ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_default_name)
{
    EXPECT_STREQ( "あぼ〜ん [デフォルト名無し]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::default_name ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_name_thread)
{
    EXPECT_STREQ( "あぼ〜ん [NG 名前:スレ]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::name_thread ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_name_board)
{
    EXPECT_STREQ( "あぼ〜ん [NG 名前:板]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::name_board ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_name_global)
{
    EXPECT_STREQ( "あぼ〜ん [NG 名前:全体]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::name_global ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_not_sage)
{
    EXPECT_STREQ( "あぼ〜ん [sage以外]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::not_sage ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_word_thread)
{
    EXPECT_STREQ( "あぼ〜ん [NG ワード:スレ]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::word_thread ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_regex_thread)
{
    EXPECT_STREQ( "あぼ〜ん [NG 正規表現:スレ]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::regex_thread ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_word_board)
{
    EXPECT_STREQ( "あぼ〜ん [NG ワード:板]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::word_board ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_regex_board)
{
    EXPECT_STREQ( "あぼ〜ん [NG 正規表現:板]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::regex_board ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_word_global)
{
    EXPECT_STREQ( "あぼ〜ん [NG ワード:全体]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::word_global ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_regex_global)
{
    EXPECT_STREQ( "あぼ〜ん [NG 正規表現:全体]", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::regex_global ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_chain)
{
    EXPECT_STREQ( "連鎖あぼ〜ん", DBTREE::NodeTreeBase::get_abone_reason( DBTREE::Abone::chain ) );
}

TEST_F(DBTREE_NodeTreeBase_GetAboneReasonTest, abone_out_of_range)
{
    DBTREE::Abone out_of_range;

    out_of_range = static_cast<DBTREE::Abone>( static_cast<unsigned char>( DBTREE::Abone::none ) - 1 );
    EXPECT_STREQ( "", DBTREE::NodeTreeBase::get_abone_reason( out_of_range ) );

    out_of_range = static_cast<DBTREE::Abone>( static_cast<unsigned char>( DBTREE::Abone::chain ) + 1 );
    EXPECT_STREQ( "", DBTREE::NodeTreeBase::get_abone_reason( out_of_range ) );
}

} // namespace
