// SPDX-License-Identifier: GPL-2.0-or-later

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


class DBTREE_Root_IsJBBSTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsJBBSTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_JBBS( "" ) );
}

TEST_F(DBTREE_Root_IsJBBSTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_JBBS( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_JBBS( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_JBBS( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsJBBSTest, match_jbbs_livedoor_jp)
{
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "http://jbbs.livedoor.jp/board/12345" ) );
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "https://jbbs.livedoor.jp/board/67890" ) );
}

TEST_F(DBTREE_Root_IsJBBSTest, match_jbbs_shitaraba_com)
{
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "http://jbbs.shitaraba.com/board/98765" ) );
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "https://jbbs.shitaraba.com/board/43210" ) );
}

TEST_F(DBTREE_Root_IsJBBSTest, match_jbbs_shitaraba_net)
{
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "http://jbbs.shitaraba.net/board/13579" ) );
    EXPECT_TRUE( DBTREE::Root::is_JBBS( "https://jbbs.shitaraba.net/board/24680" ) );
}


class DBTREE_Root_IsMachiTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsMachiTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_machi( "" ) );
}

TEST_F(DBTREE_Root_IsMachiTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_machi( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_machi( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_machi( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsMachiTest, match_machi_to_without_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_machi( "http://machi.to/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_machi( "https://machi.to/board" ) );
}

TEST_F(DBTREE_Root_IsMachiTest, match_machi_to_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_machi( "http://subdomain.machi.to/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_machi( "https://subdomain.machi.to/board" ) );
}


class DBTREE_Root_IsVip2chTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsVip2chTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "" ) );
}

TEST_F(DBTREE_Root_IsVip2chTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsVip2chTest, not_match_vip2ch_com_without_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "http://vip2ch.com/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_vip2ch( "https://vip2ch.com/board" ) );
}

TEST_F(DBTREE_Root_IsVip2chTest, match_vip2ch_com_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_vip2ch( "http://subdomain.vip2ch.com/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_vip2ch( "https://subdomain.vip2ch.com/board" ) );
}


class DBTREE_Root_IsOpen2chTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsOpen2chTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_open2ch( "" ) );
}

TEST_F(DBTREE_Root_IsOpen2chTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_open2ch( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_open2ch( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_open2ch( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsOpen2chTest, not_match_open2ch_net_without_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_open2ch( "http://open2ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_open2ch( "https://open2ch.net/board" ) );
}

TEST_F(DBTREE_Root_IsOpen2chTest, match_open2ch_net_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_open2ch( "http://subdomain.open2ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_open2ch( "https://subdomain.open2ch.net/board" ) );
}


class DBTREE_Root_IsNext2chTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsNext2chTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_open2ch( "" ) );
}

TEST_F(DBTREE_Root_IsNext2chTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_next2ch( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_next2ch( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_next2ch( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsNext2chTest, not_match_next2ch_net_without_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_next2ch( "http://next2ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_next2ch( "https://next2ch.net/board" ) );
}

TEST_F(DBTREE_Root_IsNext2chTest, match_next2ch_net_with_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_next2ch( "http://subdomain.next2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_next2ch( "https://subdomain.next2ch.net/board" ) );
}


class DBTREE_Root_Is2chscTest : public ::testing::Test {};

TEST_F(DBTREE_Root_Is2chscTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "" ) );
}

TEST_F(DBTREE_Root_Is2chscTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "https://subdomain.open2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "https://next2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "http://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "http://subdomain.5ch.net/board" ) );
}

TEST_F(DBTREE_Root_Is2chscTest, not_match_2chsc_without_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2chsc( "http://2ch.sc/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2chsc( "https://2ch.sc/board" ) );
}

TEST_F(DBTREE_Root_Is2chscTest, match_2chsc_with_subdomain)
{
    EXPECT_TRUE( DBTREE::Root::is_2chsc( "http://subdomain.2ch.sc/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_2chsc( "https://subdomain.2ch.sc/board" ) );
}

TEST_F(DBTREE_Root_Is2chscTest, not_match_2chsc_with_info_subdomain)
{
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "http://info.2ch.sc/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_2chsc( "https://info.2ch.sc/board" ) );
}


class DBTREE_Root_IsLocalTest : public ::testing::Test {};

TEST_F(DBTREE_Root_IsLocalTest, empty_string)
{
    EXPECT_FALSE( DBTREE::Root::is_local( "" ) );
}

TEST_F(DBTREE_Root_IsLocalTest, not_match_other_domains)
{
    EXPECT_FALSE( DBTREE::Root::is_local( "https://subdomain.2ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_local( "https://5ch.net/board" ) );
    EXPECT_FALSE( DBTREE::Root::is_local( "http://subdomain.bbspink.com/board" ) );
}

TEST_F(DBTREE_Root_IsLocalTest, match_file_scheme_at_head)
{
    EXPECT_TRUE( DBTREE::Root::is_local( "file://2ch.net/board" ) );
    EXPECT_TRUE( DBTREE::Root::is_local( "file:///home/user/foobar" ) );
}

TEST_F(DBTREE_Root_IsLocalTest, match_file_scheme_at_middle_or_tail)
{
    // file:// の位置が先頭にあるかチェックしていない
    EXPECT_TRUE( DBTREE::Root::is_local( "http://2ch.net/file://board" ) );
    EXPECT_TRUE( DBTREE::Root::is_local( "https:://5ch.net/board/file://" ) );
}

} // namespace
