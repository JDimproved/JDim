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
    // トリップ取得
    std::string get_trip( const std::string& str, const std::string& charset );
};

#endif
