// ライセンス: GPL2

// メッセージ表示関数

#ifndef _MISCMSG_H
#define _MISCMSG_H

#include <string>

namespace MISC
{
    // エラー出力
    void ERRMSG( const std::string& err );

    // メッセージ
    void MSG( const std::string& msg );
}

#endif
