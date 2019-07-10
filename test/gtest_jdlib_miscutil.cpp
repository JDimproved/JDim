// License: GPL2

#include "jdlib/miscutil.h"

#include "gtest/gtest.h"


namespace {

class SplitLineTest : public ::testing::Test {};

TEST_F(SplitLineTest, split_empty)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"" ) );
}

TEST_F(SplitLineTest, split_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"    " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8" the quick  brown   fox  " ) );
}

TEST_F(SplitLineTest, split_U_3000)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000 \u3000 " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000the\u3000quick \u3000brown\u3000 \u3000fox\u3000 " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"  \"\"  " ) );

    expect.assign( { u8"the quick", u8" ", u8" brown   fox " } );
    EXPECT_EQ( expect, MISC::split_line( u8" \"the quick\" \" \" \" brown   fox \" " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_3000)
{
    std::list< std::string > expect = { u8"the\u3000quick", u8"\u3000", u8"\u3000brown \u3000fox\u3000" };
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000\"the\u3000quick\" \"\u3000\" \"\u3000brown \u3000fox\u3000\"" ) );
}


class RemoveSpaceTest : public ::testing::Test {};

TEST_F(RemoveSpaceTest, remove_empty)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"" ) );
}

TEST_F(RemoveSpaceTest, remove_U_0020)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"    " ) );

    expect.assign( u8"the quick  brown   fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8" the quick  brown   fox  " ) );
}

TEST_F(RemoveSpaceTest, remove_U_3000)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \u3000 " ) );

    expect.assign( u8"the quick\u3000brown\u3000 fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000the quick\u3000brown\u3000 fox\u3000 " ) );
}

TEST_F(RemoveSpaceTest, remove_doublequote)
{
    std::string expect = u8"\"\"";
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \"\"\u3000 " ) );
}

} // namespace
