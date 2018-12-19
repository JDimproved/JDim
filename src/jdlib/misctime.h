// ライセンス: GPL2

// 時間関係の関数

#ifndef _MISCTIME_H
#define _MISCTIME_H

#include <string>
#include <sys/time.h>
#include <ctime>

namespace MISC
{

    // timetostr のモード
    enum
    {
        TIME_NORMAL = 0, // 年/月/日 時:分
        TIME_NO_YEAR, // 月/日 時:分
        TIME_WEEK,  // 年/月/日(曜日) 時:分:秒
        TIME_PASSED, // ～前
        TIME_SECOND, // 年/月/日 時:分:秒
        // この値が設定ファイルに保存されているので、最後に追加

        TIME_NUM
    };

    // gettimeofday()の秒を文字列で取得
    std::string get_sec_str();

    // timeval を str に変換
    std::string timevaltostr( const struct timeval& tv );

    // 時刻の文字列を紀元からの経過秒に直す
    // (例) Tue, 27 Dec 2005 14:28:10 GMT -> 1135693690
    time_t datetotime( const std::string& date );

    // time_t を月日の文字列に変換
    // (例) mode == TIME_NORMAL なら 1135785252 -> 2005/12/29 0:54
    std::string timettostr( const time_t time_from, const int mode );

    // 実行時間測定用
    void start_measurement( const int id );
    int measurement( const int id );
}

#endif
