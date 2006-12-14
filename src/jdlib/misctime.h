// ライセンス: GPL2

// 時間関係の関数

#ifndef _MISCTIME_H
#define _MISCTIME_H

#include <string>

namespace MISC
{
    // timeval を str に変換
    std::string timevaltostr( struct timeval& tv );

    // 時刻の文字列を紀元からの経過秒に直す
    // (例) Tue, 27 Dec 2005 14:28:10 GMT -> 1135693690
    time_t datetotime( const std::string& date );

    // time_t を月日の文字列に変換
    // (例) 1135785252 -> 2005/12/29 0:54
    std::string timettostr( time_t time_from );
}

#endif
