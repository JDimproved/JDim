// ライセンス: GPL2

//
// 自分の書き込みを判定するために書き込み内容を一時的に保存しておくクラス
//

#ifndef _LOGITEM_H
#define _LOGITEM_H

#include "jdlib/miscutil.h"

#include <list>
#include <string>
#include <sys/time.h>

namespace MESSAGE
{
    class LogItem
    {
      public:

        const std::string url;
        const std::string subject;
        const std::string msg;
        std::list< std::string > msg_lines;
        const time_t time_write;
        bool remove;

        LogItem( const std::string& _url, const std::string& _subject, const std::string& _msg, const time_t _time_write )
        : url( _url ), subject( _subject ), msg( _msg ), time_write( _time_write ), remove( false )
        {
            // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
            // 改行のみの行を削除しないようにするため
            msg_lines = MISC::get_lines( MISC::replace_str( MISC::remove_spaces( msg ), "\n", " \n" ) );
        }
    };
}

#endif
