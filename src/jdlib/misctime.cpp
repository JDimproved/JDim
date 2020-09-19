// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "misctime.h"
#include "miscmsg.h"

#include <sstream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sys/time.h>
#include <vector>


//
// gettimeofday()の秒を文字列で取得
//
std::string MISC::get_sec_str()
{
	std::ostringstream sec_str;

	struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    sec_str << tv.tv_sec;

    return sec_str.str();
}


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
//
time_t MISC::datetotime( const std::string& date )
{
    if( date.empty() ) return 0;

    struct tm tm_out;
    memset( &tm_out, 0, sizeof( struct tm ) );

    // (注意) LC_TIMEが"C"でないと環境によってはstrptime()が失敗する
    std::string lcl;
    char *lcl_tmp = setlocale( LC_TIME, nullptr );
    if( lcl_tmp ) lcl = lcl_tmp;
#ifdef _DEBUG
    std::cout << "locale = " << lcl << std::endl;
#endif    
    if( ! lcl.empty() ) setlocale( LC_TIME, "C" ); 
    char *ret = strptime( date.c_str(), "%a, %d %b %Y %T %Z", &tm_out );
    if( ! lcl.empty() ) setlocale( LC_TIME, lcl.c_str() ); 

    if( ret == nullptr ) return 0;

#ifdef USE_MKTIME
    time_t t_ret = mktime( &tm_out );
#else
    time_t t_ret = timegm( &tm_out );
#endif

#ifdef _DEBUG
    std::cout << "MISC::datetotime " << date << " -> " << t_ret << std::endl;
#endif

    return t_ret;
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

std::vector< struct timeval > tv_measurement;

void MISC::start_measurement( const int id )
{
    while( (int)tv_measurement.size() <= id ){
        struct timeval tv;
        tv_measurement.push_back( tv );
    }

    gettimeofday( &tv_measurement[ id ], nullptr );
}

int MISC::measurement( const int id )
{
    if( id >= (int)tv_measurement.size() ) return 0;

    struct timeval tv;
    gettimeofday( &tv, nullptr );

    int ret = ( tv.tv_sec * 1000000 + tv.tv_usec ) - ( tv_measurement[ id ].tv_sec * 1000000 + tv_measurement[ id ].tv_usec );
    tv_measurement[ id ] = tv;

    return ret;
}
