// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "misctime.h"
#include "miscmsg.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <locale>
#include <sstream>
#include <sys/time.h>
#include <vector>


//
// timeval を str に変換
//
std::string MISC::timevaltostr( const struct timeval& tv )
{
    std::ostringstream sstr;
    sstr << ( tv.tv_sec >> 16 ) << " " << ( tv.tv_sec & 0xffff ) << " " << tv.tv_usec;
    return sstr.str();
}


//
// 時刻を紀元からの経過秒に直す
// 日時のフォーマットはHTTPリクエストの形式(RFC 7232 IMF-fixdate)
//
time_t MISC::datetotime( const std::string& date )
{
    if( date.empty() ) return 0;

    std::tm buf{};
    std::istringstream iss( date );
    iss.imbue( std::locale::classic() ); // Cロケール

    iss >> std::get_time( &buf, "%a, %d %b %Y %T GMT" );
    if( iss.fail() ) return 0;

    const std::time_t utc = timegm( &buf );
#ifdef _DEBUG
    std::cout << "MISC::datetotime " << date << " -> " << utc << std::endl;
#endif
    return utc;
}


//
// time_t を月日の文字列に変換
//
std::string MISC::timettostr( const time_t time_from, const int mode )
{
    std::tm tm_tmp;
    std::ostringstream ss;

    if( mode == MISC::TIME_NORMAL ){
        if( localtime_r( &time_from, &tm_tmp ) ) {
            ss << std::put_time( &tm_tmp, "%Y/%m/%d %H:%M" );
        }
    }
    else if( mode == MISC::TIME_NO_YEAR ){
        if( localtime_r( &time_from, &tm_tmp ) ) {
            ss << std::put_time( &tm_tmp, "%m/%d %H:%M" );
        }
    }
    else if( mode == MISC::TIME_WEEK ){

        constexpr char week[][32] = { "日","月","火","水","木","金","土" };

        if( localtime_r( &time_from, &tm_tmp ) ) {
            // ロケール依存の書式(%a)は使わない
            ss << std::put_time( &tm_tmp, "%Y/%m/%d(" )
               << week[ tm_tmp.tm_wday ]
               << std::put_time( &tm_tmp, ") %H:%M:%S" );
        }
    }
    else if( mode == MISC::TIME_SECOND ){
        if( localtime_r( &time_from, &tm_tmp ) ) {
            ss << std::put_time( &tm_tmp, "%Y/%m/%d %H:%M:%S" );
        }
    }
    else if( mode == MISC::TIME_PASSED ){

        const std::time_t duration = std::time( nullptr ) - time_from;

        if( duration < 0 ) ss << "未来";
        else if( duration < 60 ) ss << duration << " 秒前";
        else if( duration < 60 * 60 ) ss << ( duration / 60 )  << " 分前";
        else if( duration < 60 * 60 * 24 ) ss << ( duration / ( 60 * 60 ) ) << " 時間前";
        else if( duration < 60 * 60 * 24 * 365 ) ss << ( duration / ( 60 * 60 * 24 ) ) <<  " 日前";
        else ss << ( duration / ( 60 * 60 * 24 * 365 ) ) <<  " 年前";
    }

    std::string str_ret = ss.str();

#ifdef _DEBUG
    std::cout << "MISC::timettostr " << time_from << " -> " << str_ret << std::endl;
#endif

    return str_ret;
}


#ifdef NO_TIMEGM
//
// timegm
//
// Solarisの場合はtimegmが存在しないため、代替コードを宣言する(by kohju)
// 原典：linux の man timegm
//
time_t timegm (struct tm *tm)
{
    time_t ret;
    char *tz;

    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    ret = mktime(tm);
    if (tz)
 	setenv("TZ", tz, 1);
    else
 	unsetenv("TZ");
    tzset();
    return ret;
}
#endif



// 実行時間測定用

static std::vector<std::chrono::steady_clock::time_point> tv_measurement;

void MISC::start_measurement( const unsigned int id )
{
    if( tv_measurement.size() <= id ) {
        tv_measurement.resize( id + 1 );
    }
    tv_measurement[id] = std::chrono::steady_clock::now();
}

long long MISC::measurement( const unsigned int id )
{
    if( id >= tv_measurement.size() ) return 0;

    const auto current = std::chrono::steady_clock::now();
    const auto duration = current - tv_measurement[id];
    tv_measurement[id] = current;

    return std::chrono::duration_cast<std::chrono::nanoseconds>( duration ).count();
}
