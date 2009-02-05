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
        std::string msg;
        const time_t time_write;
        std::list< std::string > msg_lines;
        char head[ LOGITEM_SIZE_HEAD ];
        bool remove;

        LogItem( const std::string& _url, const bool _newthread, const std::string& _msg, const time_t _time_write )
        : url( _url ), newthread( _newthread ), msg( _msg ), time_write( _time_write ), remove( false )
        {

            // WAVE DASH 問題
            const char wavedash[] = { 0xe3, 0x80, 0x9c, '\0' }; // WAVE DASH (U+301C)
            const char fulltilde[] = { 0xef, 0xbd, 0x9e, '\0' }; // FULLWIDTH TILDE (U+FF5E)

            const char emdash[] = { 0xe2, 0x80, 0x94, '\0' }; // EM DASH(U+2014)
            const char hbar[] = { 0xe2, 0x80, 0x95, '\0' }; // HORIZONTAL BAR (U+2015)

            const char dvline[] = { 0xe2, 0x80, 0x96, '\0' }; // DOUBLE VERTICAL LINE (U+2016)
            const char plto[] = { 0xe2, 0x88, 0xa5, '\0' }; // PARALLEL TO (U+2225)

            const char msign[] = { 0xe2, 0x88, 0x92, '\0' }; // MINUS SIGN (U+2212) 
            const char fullhm[] = { 0xef, 0xbc, 0x8d, '\0' }; // FULLWIDTH HYPHEN-MINUS (U+FF0D)

            for( size_t i = 0; i < msg.length(); ++i ){

                if( msg.c_str()[ i ] == wavedash[ 0 ] && msg.c_str()[ i+1 ] == wavedash[ 1 ] && msg.c_str()[ i+2 ] == wavedash[ 2 ] ){
                    msg = msg.replace( i, 3, fulltilde ); i += 3;
                }
                if( msg.c_str()[ i ] == emdash[ 0 ] && msg.c_str()[ i+1 ] == emdash[ 1 ] && msg.c_str()[ i+2 ] == emdash[ 2 ] ){
                    msg = msg.replace( i, 3, hbar ); i += 3;
                }
                if( msg.c_str()[ i ] == dvline[ 0 ] && msg.c_str()[ i+1 ] == dvline[ 1 ] && msg.c_str()[ i+2 ] == dvline[ 2 ] ){
                    msg = msg.replace( i, 3, plto ); i += 3;
                }
                if( msg.c_str()[ i ] == msign[ 0 ] && msg.c_str()[ i+1 ] == msign[ 1 ] && msg.c_str()[ i+2 ] == msign[ 2 ] ){
                    msg = msg.replace( i, 3, fullhm ); i += 3;
                }
            }

            // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
            // 改行のみの行を削除しないようにするため
            msg_lines = MISC::get_lines( MISC::replace_str( MISC::remove_spaces( msg ), "\n", " \n" ) );

            // 簡易チェック用に先頭の文字列をコピー(空白は除く)
            memset( head, 0, LOGITEM_SIZE_HEAD );
            for( size_t i = 0, i2 = 0; i < LOGITEM_SIZE_HEAD && i2 < msg.length(); ++i2 ){
                if( msg.c_str()[ i2 ] != ' ' ) head[ i++ ] = msg.c_str()[ i2 ];
            }
        }
    };
}

#endif
