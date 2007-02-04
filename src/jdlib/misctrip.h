// ライセンス: GPL2

// トリップ関係の関数

//
// Thanks to 「パッチ投稿」スレの9氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/9
//

#ifndef _MISCTRIP_H
#define _MISCTRIP_H

#include <string>

namespace MISC
{
    // key から salt を取得
    const std::string get_salt( const std::string& key );

    // key と salt から trip を計算
    const std:: string create_trip( const std::string& key, const std::string& salt );

    // トリップ取得
    const std::string get_trip( const std::string& str, const std::string& charset );
};

#endif
