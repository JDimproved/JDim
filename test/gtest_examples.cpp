// License: GPL2

// Unittest機能を確認するためのコード例
// gtest_examples.cppは実際のテストケースが追加されたときにrevertしてよい。

#include "gtest/gtest.h"


namespace example {

int add(int a, int b) { return a + b; }

const char* helloworld( bool camelcase ) { return camelcase ? "HelloWorld" : "helloworld"; }

} // namespace example


namespace {

class ExampleTest : public ::testing::Test {};

TEST_F(ExampleTest, add_7_3)
{
    EXPECT_EQ( 10, example::add( 7, 3 ) );
}

TEST_F(ExampleTest, helloworld_camelcase)
{
    EXPECT_STREQ( "HelloWorld", example::helloworld( true ) );
}

} // namespace
