// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misctime.h"

#include <sstream>
#include <cstring>
#include <time.h>
#include <sys/time.h>

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
std::string MISC::timevaltostr( struct timeval& tv )
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
    if( strptime( date.c_str(), "%a, %d %b %Y %T %Z", &tm_out ) == NULL ) return 0;

    time_t t_ret = timegm( &tm_out );

#ifdef _DEBUG
    std::cout << "MISC::datetotime " << date << " -> " << t_ret << std::endl;
#endif

    return t_ret;
}


//
// time_t を月日の文字列に変換
//
std::string MISC::timettostr( time_t time_from )
{
    char str_ret[ 64 ];
    str_ret[ 0 ] = '\0';

    struct tm tm_tmp;
    if( localtime_r( &time_from, &tm_tmp ) ){

        snprintf( str_ret, 64, "%d/%02d/%02d %02d:%02d",
                  ( 1900 + tm_tmp.tm_year ), ( 1 + tm_tmp.tm_mon ), tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min );
    }

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
