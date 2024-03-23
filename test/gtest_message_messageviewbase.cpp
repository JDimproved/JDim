// SPDX-License-Identifier: GPL-2.0-or-later

#include "message/messageviewbase.h"

#include "gtest/gtest.h"


namespace {

class MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest : public ::testing::Test {};

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, empty_string)
{
    EXPECT_EQ( 0, MESSAGE::MessageViewBase::count_diffs_for_special_char( "" ) );
}

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, not_include_special_characters)
{
    EXPECT_EQ( 0, MESSAGE::MessageViewBase::count_diffs_for_special_char( "abcdefghijklmnopqrstuvwxyz" ) );
    EXPECT_EQ( 0, MESSAGE::MessageViewBase::count_diffs_for_special_char( "0123456789" ) );
    EXPECT_EQ( 0, MESSAGE::MessageViewBase::count_diffs_for_special_char( "!#$%&'zz89=~|-^\\@`{}[];:+*,./?_" ) );
}

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, include_line_feed)
{
    EXPECT_EQ( 5, MESSAGE::MessageViewBase::count_diffs_for_special_char( "abc\ndef" ) );
    EXPECT_EQ( 15, MESSAGE::MessageViewBase::count_diffs_for_special_char( "\n\n0123456789\n" ) );
}

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, include_double_quote)
{
    EXPECT_EQ( 5, MESSAGE::MessageViewBase::count_diffs_for_special_char( R"(abc"def)" ) );
    EXPECT_EQ( 10, MESSAGE::MessageViewBase::count_diffs_for_special_char( R"( "hello world" )" ) );
}

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, include_less_than_sign)
{
    EXPECT_EQ( 3, MESSAGE::MessageViewBase::count_diffs_for_special_char( "abc<def" ) );
    EXPECT_EQ( 6, MESSAGE::MessageViewBase::count_diffs_for_special_char( "<hello world<" ) );
}

TEST_F(MESSAGE_MessageViewBase_CountDiffsForSpecialCharTest, include_greater_than_sign)
{
    EXPECT_EQ( 3, MESSAGE::MessageViewBase::count_diffs_for_special_char( "abc>def" ) );
    EXPECT_EQ( 6, MESSAGE::MessageViewBase::count_diffs_for_special_char( ">hello world>" ) );
}

}
