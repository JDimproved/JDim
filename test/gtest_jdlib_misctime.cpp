// License: GPL-2.0

#include "jdlib/misctime.h"

#include "gtest/gtest.h"

#include <glibmm.h>
#include <time.h> // tzset

#ifdef _POSIX_C_SOURCE
namespace {

static_assert( sizeof(time_t) >= 4, "MISC_TimetToStrTest requires sizeof(time_t) >= 4." );

// NOTE: 関数の戻り値はUTF-8文字列であるのが前提
class MISC_TimetToStrTest : public ::testing::Test
{
    std::string m_save_tz;

    void SetUp() override
    {
        m_save_tz = Glib::getenv( "TZ" );
        Glib::setenv( "TZ", "GMT" );
        ::tzset();
    }

    void TearDown() override
    {
        Glib::setenv( "TZ", m_save_tz );
        ::tzset();
    }
};

TEST_F(MISC_TimetToStrTest, time_normal)
{
    EXPECT_EQ( "1970/01/01 00:00", MISC::timettostr( 0, MISC::TIME_NORMAL ) );
    EXPECT_EQ( "2000/10/02 15:20", MISC::timettostr( 970500000, MISC::TIME_NORMAL ) );
    EXPECT_EQ( "2009/02/13 23:31", MISC::timettostr( 1234567890, MISC::TIME_NORMAL ) );
    EXPECT_EQ( "2038/01/19 03:14", MISC::timettostr( 2147483647, MISC::TIME_NORMAL ) );
}

TEST_F(MISC_TimetToStrTest, time_no_year)
{
    EXPECT_EQ( "01/01 00:00", MISC::timettostr( 0, MISC::TIME_NO_YEAR ) );
    EXPECT_EQ( "10/02 15:20", MISC::timettostr( 970500000, MISC::TIME_NO_YEAR ) );
    EXPECT_EQ( "02/13 23:31", MISC::timettostr( 1234567890, MISC::TIME_NO_YEAR ) );
    EXPECT_EQ( "01/19 03:14", MISC::timettostr( 2147483647, MISC::TIME_NO_YEAR ) );
}

TEST_F(MISC_TimetToStrTest, time_week)
{
    EXPECT_EQ( "1970/01/01(木) 00:00:00", MISC::timettostr( 0, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2000/10/02(月) 15:20:00", MISC::timettostr( 970500000, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2000/10/04(水) 22:53:20", MISC::timettostr( 970700000, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2000/10/07(土) 06:26:40", MISC::timettostr( 970900000, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2000/10/08(日) 10:13:20", MISC::timettostr( 971000000, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2009/02/13(金) 23:31:30", MISC::timettostr( 1234567890, MISC::TIME_WEEK ) );
    EXPECT_EQ( "2038/01/19(火) 03:14:07", MISC::timettostr( 2147483647, MISC::TIME_WEEK ) );
}

TEST_F(MISC_TimetToStrTest, time_passed)
{
    // NOTE: テストケースは60秒以内に関数の実行が完了すること＆算術オーバフローしないことが前提
    // 〜秒後はタイミングがシビアなので数値はテストしない
    const std::string sec = MISC::timettostr( time( nullptr ), MISC::TIME_PASSED ); // 現時刻
    EXPECT_EQ( " 秒前", sec.substr( sec.size() - 7, std::string::npos ) );
    EXPECT_EQ( "1 分前", MISC::timettostr( time( nullptr ) - 60, MISC::TIME_PASSED ) ); // 60秒前
    EXPECT_EQ( "1 時間前", MISC::timettostr( time( nullptr ) - 3600, MISC::TIME_PASSED ) ); // 60分前
    EXPECT_EQ( "1 日前", MISC::timettostr( time( nullptr ) - 86400, MISC::TIME_PASSED ) ); // 24時間前
    EXPECT_EQ( "1 年前", MISC::timettostr( time( nullptr ) - 31622400, MISC::TIME_PASSED ) ); // 366日前
    EXPECT_EQ( "未来", MISC::timettostr( time( nullptr ) + 60, MISC::TIME_PASSED ) ); // 60秒後
}

TEST_F(MISC_TimetToStrTest, time_second)
{
    EXPECT_EQ( "1970/01/01 00:00:00", MISC::timettostr( 0, MISC::TIME_SECOND ) );
    EXPECT_EQ( "2000/10/02 15:20:00", MISC::timettostr( 970500000, MISC::TIME_SECOND ) );
    EXPECT_EQ( "2009/02/13 23:31:30", MISC::timettostr( 1234567890, MISC::TIME_SECOND ) );
    EXPECT_EQ( "2038/01/19 03:14:07", MISC::timettostr( 2147483647, MISC::TIME_SECOND ) );
}

} // namespace
#endif // _POSIX_C_SOURCE
