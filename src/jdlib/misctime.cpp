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
#include <sys/time.h>
#include <vector>

#ifdef _WIN32
#include <stdlib.h>
// not exist _r suffix functions in mingw time.h
#define localtime_r( _clock, _result ) \
        ( *(_result) = *localtime( (_clock) ), \
          (_result) )
#endif

//
// gettimeofday()の秒を文字列で取得
//
const std::string MISC::get_sec_str()
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
const std::string MISC::timevaltostr( const struct timeval& tv )
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

#ifdef _WIN32
    int rc;
    char month[4];
    char monthes[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"  };
    char tzone[4];
    rc = sscanf(date.c_str(), "%*3s, %2d %3s %4d %2d:%2d:%2d %3s",
                &tm_out.tm_mday, month, &tm_out.tm_year,
                &tm_out.tm_hour, &tm_out.tm_min, &tm_out.tm_sec, tzone);
    if ( rc != 7 )
    {
        ERRMSG( "MISC::datetotime : unknown format = " + date );
        return 0;
    }
    tm_out.tm_year -= 1900;
    for (int i=0; i<12; i++)
    {
        if (strcasecmp(monthes[i], month) == 0)
        {
            tm_out.tm_mon = i;
            break;
        }
    }

    // 通常はTZにGMTが渡される
    std::string env = "TZ=";
    putenv( (env + tzone).c_str() );
    tzset();
    time_t t_ret = mktime( &tm_out );
    putenv( "TZ=" );
    tzset();

#else // _WIN32
    // (注意) LC_TIMEが"C"でないと環境によってはstrptime()が失敗する
    std::string lcl;
    char *lcl_tmp = setlocale( LC_TIME, NULL );
    if( lcl_tmp ) lcl = lcl_tmp;
#ifdef _DEBUG
    std::cout << "locale = " << lcl << std::endl;
#endif    
    if( ! lcl.empty() ) setlocale( LC_TIME, "C" ); 
    char *ret = strptime( date.c_str(), "%a, %d %b %Y %T %Z", &tm_out );
    if( ! lcl.empty() ) setlocale( LC_TIME, lcl.c_str() ); 

    if( ret == NULL ) return 0;

#ifdef USE_MKTIME
    time_t t_ret = mktime( &tm_out );
#else
    time_t t_ret = timegm( &tm_out );
#endif
#endif // _WIN32

#ifdef _DEBUG
    std::cout << "MISC::datetotime " << date << " -> " << t_ret << std::endl;
#endif

    return t_ret;
}


//
// time_t を月日の文字列に変換
//
const std::string MISC::timettostr( const time_t& time_from, const int mode )
{
    const int lng = 64;
    struct tm tm_tmp;

    char str_ret[ lng ];
    str_ret[ 0 ] = '\0';

    if( mode == MISC::TIME_NORMAL ){

        if( localtime_r( &time_from, &tm_tmp ) )
            snprintf( str_ret, lng, "%d/%02d/%02d %02d:%02d",
                      ( 1900 + tm_tmp.tm_year ), ( 1 + tm_tmp.tm_mon ), tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min );
    }
    else if( mode == MISC::TIME_NO_YEAR ){

        if( localtime_r( &time_from, &tm_tmp ) )
            snprintf( str_ret, lng, "%02d/%02d %02d:%02d",
                      ( 1 + tm_tmp.tm_mon ), tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min );
    }
    else if( mode == MISC::TIME_WEEK ){

        const char week[][32] = { "日","月","火","水","木","金","土" };

        if( localtime_r( &time_from, &tm_tmp ) )
            snprintf( str_ret, lng, "%d/%02d/%02d(%s) %02d:%02d:%02d",
                      ( 1900 + tm_tmp.tm_year ), ( 1 + tm_tmp.tm_mon ), tm_tmp.tm_mday,
                      week[ tm_tmp.tm_wday ], tm_tmp.tm_hour, tm_tmp.tm_min, tm_tmp.tm_sec );
    }
    else if( mode == MISC::TIME_SECOND ){

        if( localtime_r( &time_from, &tm_tmp ) )
            snprintf( str_ret, lng, "%d/%02d/%02d %02d:%02d:%02d",
                      ( 1900 + tm_tmp.tm_year ), ( 1 + tm_tmp.tm_mon ), tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min, tm_tmp.tm_sec );
    }
    else if( mode == MISC::TIME_PASSED ){

        const time_t tmp_t = time( NULL ) - time_from;

        if( tmp_t < 0 ) snprintf( str_ret, lng, "未来" );
        else if( tmp_t < 60 ) snprintf( str_ret, lng, "%d 秒前", (int) tmp_t );
        else if( tmp_t < 60 * 60 ) snprintf( str_ret, lng, "%d 分前", (int)tmp_t / 60 );
        else if( tmp_t < 60 * 60 * 24 ) snprintf( str_ret, lng, "%d 時間前", (int)tmp_t / ( 60 * 60 ) );
        else if( tmp_t < 60 * 60 * 24 * 365 ) snprintf( str_ret, lng, "%d 日前", (int)tmp_t / ( 60 * 60 * 24 ) );
        else snprintf( str_ret, lng, "%d 年前", (int)tmp_t / ( 60 * 60 * 24 * 365 ) );
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



// 実行時間測定用

std::vector< struct timeval > tv_measurement;

void MISC::start_measurement( const int id )
{
    while( (int)tv_measurement.size() <= id ){
        struct timeval tv;
        tv_measurement.push_back( tv );
    }

    gettimeofday( &tv_measurement[ id ], NULL );
}

int MISC::measurement( const int id )
{
    if( id >= (int)tv_measurement.size() ) return 0;

    struct timeval tv;
    gettimeofday( &tv, NULL );

    int ret = ( tv.tv_sec * 1000000 + tv.tv_usec ) - ( tv_measurement[ id ].tv_sec * 1000000 + tv_measurement[ id ].tv_usec );
    tv_measurement[ id ] = tv;

    return ret;
}
