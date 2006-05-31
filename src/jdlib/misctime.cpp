// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "misctime.h"

#include <sstream>
#include <time.h>


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
    std::string str_ret;

    struct tm tm_tmp;
    if( localtime_r( &time_from, &tm_tmp ) ){
        std::stringstream ss;
        ss << ( 1900 + tm_tmp.tm_year ) << "/" << ( 1 + tm_tmp.tm_mon ) << "/"
           << tm_tmp.tm_mday << " "  << tm_tmp.tm_hour << ":" << tm_tmp.tm_min;

        str_ret = ss.str();
    }

#ifdef _DEBUG
    std::cout << "MISC::timettostr " << time_from << " -> " << str_ret << std::endl;
#endif

    return str_ret;
}
