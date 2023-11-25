// SPDX-License-Identifier: GPL-2.0-only

#include "dbtree/root.h"

#include "gtest/gtest.h"


namespace {

class DBTREE_Root_Is2chTest : public ::testing::Test {};

TEST_F(DBTREE_Root_Is2chTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "" ) );
}

TEST_F(DBTREE_Root_Is2chTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://subdomain.open2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://next2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "http://subdomain.2ch.sc/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, not_match_2ch_without_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "http://2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://2ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, match_2ch_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2ch( "http://subdomain.2ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2ch( "https://subdomain.2ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, not_match_2ch_with_info_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "http://info.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://info.2ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, match_5ch_without_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2ch( "http://5ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2ch( "https://5ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, match_5ch_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2ch( "http://subdomain.5ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2ch( "https://subdomain.5ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, not_match_5ch_with_info_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "http://info.5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://info.5ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, not_match_bbspink_without_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_2ch( "http://bbspink.com/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2ch( "https://bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, match_bbspink_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2ch( "http://subdomain.bbspink.com/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2ch( "https://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_Is2chTest, match_bbspink_with_info_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2ch( "http://info.bbspink.com/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2ch( "https://info.bbspink.com/board" ) );
}

} // namespace
