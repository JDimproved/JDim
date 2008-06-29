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
#include <cstring>

enum
{
    LOGITEM_SIZE_HEAD = 64
};

namespace MESSAGE
{
    class LogItem
    {
      public:

        const std::string url;
        const bool newthread;
        const std::string msg;
        const time_t time_write;
        std::list< std::string > msg_lines;
        char head[ LOGITEM_SIZE_HEAD ];
        bool remove;

        LogItem( const std::string& _url, const bool _newthread, const std::string& _msg, const time_t _time_write )
        : url( _url ), newthread( _newthread ), msg( _msg ), time_write( _time_write ), remove( false )
        {
            // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
            // 改行のみの行を削除しないようにするため
            msg_lines = MISC::get_lines( MISC::replace_str( MISC::remove_spaces( msg ), "\n", " \n" ) );

            // 簡易チェック用に先頭の文字列をコピー(空白は除く)
            memset( head, 0, LOGITEM_SIZE_HEAD );
            for( size_t i = 0, i2 = 0; i < LOGITEM_SIZE_HEAD && i2 < _msg.length(); ++i2 ){
                if( _msg.c_str()[ i2 ] != ' ' ) head[ i++ ] = _msg.c_str()[ i2 ];
            }
        }
    };
}

#endif
