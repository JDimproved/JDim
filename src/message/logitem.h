// ライセンス: GPL2

//
// 自分の書き込みを判定するために書き込み内容を一時的に保存しておくクラス
//

#ifndef _LOGITEM_H
#define _LOGITEM_H

#include "messageadmin.h"

#include "jdlib/miscutil.h"

#include <ctime>
#include <list>
#include <string>


enum
{
    LOGITEM_SIZE_HEAD = 64
};

namespace MESSAGE
{
    class LogItem
    {
      public:

        std::string url;
        const bool newthread;
        std::string msg;
        std::time_t time_write;
        std::list< std::string > msg_lines;
        char head[ LOGITEM_SIZE_HEAD ]{};
        bool remove{};

        LogItem( const std::string& _url, const bool _newthread, const std::string& _msg )
            : url( _url )
            , newthread( _newthread )
            , msg( _msg )
            , time_write{ std::time( nullptr ) }
        {
            if( newthread && url.find( ID_OF_NEWTHREAD ) != std::string::npos ) url = url.substr( 0, url.find( ID_OF_NEWTHREAD ) );

            // WAVE DASH 問題
            msg = MISC::utf8_fix_wavedash( msg, MISC::UNIXtoWIN );

            // 水平タブを空白に置き換える
            msg = MISC::replace_str( msg, "\t", " " );

            // 数字参照を変換
            msg = MISC::decode_spchar_number( msg );

            // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
            // 改行のみの行を削除しないようにするため
            msg_lines = MISC::get_lines( MISC::replace_str( MISC::ascii_trim( msg ), "\n", " \n" ) );

            // 簡易チェック用に先頭の文字列をコピー(空白は除く)
            for( size_t i = 0, i2 = 0; i < LOGITEM_SIZE_HEAD -1 && i2 < msg.length(); ++i2 ){
                if( msg[ i2 ] != ' ' ) head[ i++ ] = msg[ i2 ];
            }
        }
    };
}

#endif
