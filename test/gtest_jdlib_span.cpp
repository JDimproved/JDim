// SPDX-License-Identifier: GPL-2.0-only

#include "gtest/gtest.h"

#include "jdlib/span.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>


namespace {

class SpanTest : public ::testing::Test
{
protected:
    // test fixtures
    int bounded_array5[5] = { 1, 2, 3, 4, 5 };
    std::array<const char*, 4> std_array = { "abc", "def", "ghi", "jkl" };
    std::string std_string = "helloworld";
    std::string_view std_string_view = "lorem ipsum";
    std::vector<std::string> std_vector = { "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog" };
};

TEST_F(SpanTest, construct_const)
{
    JDLIB::span<const int> arr_span{ bounded_array5 };
    EXPECT_EQ( 1, *arr_span.begin() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array[0], stdarr_span[0] );

    const std::array<const char*, 3> const_std_array = { "qrs", "tuv", "wxyz" };
    JDLIB::span<const char* const> const_stdarr_span{ const_std_array };
    EXPECT_EQ( const_std_array.back(), const_stdarr_span.back() );

    JDLIB::span<const char> const_std_string_span{ std_string };
    EXPECT_EQ( std_string.data(), const_std_string_span.data() );

    JDLIB::span<const char> const_std_string_view_span{ std_string_view };
    EXPECT_EQ( std_string_view.data(), const_std_string_view_span.data() );

    JDLIB::span<const std::string> vec_span{ std_vector };
    EXPECT_EQ( "quick", vec_span.front() );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    EXPECT_EQ( 7, static_arr_span.size() );

    static const std::array<short, 3> static_stdarr = { 111, 222, 333 };
    constexpr JDLIB::span<const short> static_stdarr_span{ static_stdarr };
    EXPECT_EQ( 3, static_stdarr_span.size() );
}

TEST_F(SpanTest, operator_equal)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    JDLIB::span<int> mutable_copy = arr_span;
    EXPECT_EQ( 1, *mutable_copy.begin() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    JDLIB::span<const char* const> const_copy = stdarr_span;
    EXPECT_EQ( std_array[0], const_copy[0] );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr JDLIB::span<std::size_t> constexpr_copy = static_arr_span;
    EXPECT_EQ( 123, constexpr_copy.front() );
}

TEST_F(SpanTest, begin)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 1, *arr_span.begin() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( 2, *arr_subspan.begin() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( *std_array.begin(), *stdarr_span.begin() );

    JDLIB::span<std::string> vec_span{ std_vector };
    *vec_span.begin() = "moge";
    EXPECT_EQ( *std_vector.begin(), "moge" );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr std::size_t value = *static_arr_span.begin();
    EXPECT_EQ( 123, value );
}

TEST_F(SpanTest, end)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5 + 5, arr_span.end() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 4, arr_subspan.end() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array.data() + std_array.size(), stdarr_span.end() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( std_vector.data() + std_vector.size(), vec_span.end() );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto end = static_arr_span.end();
    EXPECT_EQ( static_arr + 7, end );
}

TEST_F(SpanTest, rbegin)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 5, *arr_span.rbegin() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( 4, *arr_subspan.rbegin() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( *std_array.rbegin(), *stdarr_span.rbegin() );

    JDLIB::span<std::string> vec_span{ std_vector };
    *vec_span.rbegin() = "cat";
    EXPECT_EQ( *std_vector.rbegin(), "cat" );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto value = *static_arr_span.rbegin();
    EXPECT_EQ( 789, value );
}

TEST_F(SpanTest, rend)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5, arr_span.rend().base() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 1, arr_subspan.rend().base() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array.data(), stdarr_span.rend().base() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( std_vector.data(), vec_span.rend().base() );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto it = static_arr_span.rend().base();
    EXPECT_EQ( static_arr, it );
}

TEST_F(SpanTest, front)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5[0], arr_span.front() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5[1], arr_subspan.front() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array[0], stdarr_span.front() );

    JDLIB::span<char> const_std_string_span{ std_string };
    EXPECT_EQ( std_string.front(), const_std_string_span.front() );

    JDLIB::span<const char> const_std_string_view_span{ std_string_view };
    EXPECT_EQ( std_string_view.front(), const_std_string_view_span.front() );

    JDLIB::span<std::string> vec_span{ std_vector };
    vec_span.front() = "foobar";
    EXPECT_EQ( std_vector[0], "foobar" );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto value = static_arr_span.front();
    EXPECT_EQ( static_arr[0], value );
}

TEST_F(SpanTest, back)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5[4], arr_span.back() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5[3], arr_subspan.back() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array[3], stdarr_span.back() );

    JDLIB::span<char> const_std_string_span{ std_string };
    EXPECT_EQ( std_string.back(), const_std_string_span.back() );

    JDLIB::span<const char> const_std_string_view_span{ std_string_view };
    EXPECT_EQ( std_string_view.back(), const_std_string_view_span.back() );

    JDLIB::span<std::string> vec_span{ std_vector };
    vec_span.back() = "bazqux";
    EXPECT_EQ( std_vector[7], "bazqux" );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto value = static_arr_span.back();
    EXPECT_EQ( static_arr[6], value );
}

TEST_F(SpanTest, operator_brackets)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5[1], arr_span[1] );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5[2], arr_subspan[1] );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array[1], stdarr_span[1] );

    JDLIB::span<char> const_std_string_span{ std_string };
    EXPECT_EQ( std_string[1], const_std_string_span[1] );

    JDLIB::span<const char> const_std_string_view_span{ std_string_view };
    EXPECT_EQ( std_string_view[1], const_std_string_view_span[1] );

    JDLIB::span<std::string> vec_span{ std_vector };
    vec_span[1] = "hogefuga";
    EXPECT_EQ( std_vector[1], "hogefuga" );

    static constexpr std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<const std::size_t> static_arr_span{ static_arr };
    constexpr auto value = static_arr_span[1];
    EXPECT_EQ( static_arr[1], value );
}

TEST_F(SpanTest, data)
{
    JDLIB::span<char> empty_span;
    EXPECT_EQ( nullptr, empty_span.data() );

    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( bounded_array5, arr_span.data() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 1, arr_subspan.data() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( std_array.data(), stdarr_span.data() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( std_vector.data(), vec_span.data() );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto data = static_arr_span.data();
    EXPECT_EQ( static_arr, data );
}

TEST_F(SpanTest, size)
{
    JDLIB::span<char> empty_span;
    EXPECT_EQ( 0, empty_span.size() );

    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 5, arr_span.size() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( 3, arr_subspan.size() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( 4, stdarr_span.size() );

    JDLIB::span<char> const_std_string_span{ std_string };
    EXPECT_EQ( std_string.size(), const_std_string_span.size() );

    JDLIB::span<const char> const_std_string_view_span{ std_string_view };
    EXPECT_EQ( std_string_view.size(), const_std_string_view_span.size() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( 8, vec_span.size() );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto arr_size = static_arr_span.size();
    EXPECT_EQ( 7, arr_size );

    static std::array<short, 3> static_stdarr = { 111, 222, 333 };
    constexpr JDLIB::span<short> static_stdarr_span{ static_stdarr };
    constexpr auto stdarr_size = static_stdarr_span.size();
    EXPECT_EQ( 3, stdarr_size );
}

TEST_F(SpanTest, empty)
{
    JDLIB::span<char> empty_span;
    EXPECT_TRUE( empty_span.empty() );

    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_FALSE( arr_span.empty() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_FALSE( arr_subspan.empty() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_FALSE( stdarr_span.empty() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_FALSE( vec_span.empty() );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto empty = static_arr_span.empty();
    EXPECT_FALSE( empty );
}

TEST_F(SpanTest, first)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 1, *arr_span.first(3).begin() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 3, arr_subspan.first(2).end() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( 4, stdarr_span.first(4).size() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( "jumps", vec_span.first(6)[3] );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto constexpr_first = static_arr_span.first(5);
    EXPECT_EQ( static_arr + 5, constexpr_first.end() );
}

TEST_F(SpanTest, last)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 3, *arr_span.last(3).begin() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 4, arr_subspan.last(1).end() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( 4, stdarr_span.last(4).size() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( "the", vec_span.last(6)[3] );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto constexpr_last = static_arr_span.last(5);
    EXPECT_EQ( static_arr + 2, constexpr_last.begin() );
}

TEST_F(SpanTest, subspan)
{
    JDLIB::span<int> arr_span{ bounded_array5 };
    EXPECT_EQ( 2, *arr_span.subspan(1, 4).begin() );

    JDLIB::span<int> arr_subspan{ bounded_array5 + 1, 3 };
    EXPECT_EQ( bounded_array5 + 4, arr_subspan.subspan(2, 1).end() );

    JDLIB::span<const char*> stdarr_span{ std_array };
    EXPECT_EQ( 2, stdarr_span.subspan(2, 2).size() );

    JDLIB::span<std::string> vec_span{ std_vector };
    EXPECT_EQ( "lazy", vec_span.subspan(3, 4)[3] );

    static std::size_t static_arr[] = { 123, 234, 345, 456, 567, 678, 789 };
    constexpr JDLIB::span<std::size_t> static_arr_span{ static_arr };
    constexpr auto constexpr_sub = static_arr_span.subspan(1, 5);
    EXPECT_EQ( static_arr + 1, constexpr_sub.data() );
}

} // namespace
