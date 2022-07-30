// SPDX-License-Identifier: GPL-2.0-only

#include "dbtree/nodetreebase.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

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
