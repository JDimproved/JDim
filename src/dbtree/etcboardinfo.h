// ライセンス: GPL2
//
// 外部板情報
//

#ifndef _ETCBOARDINFO_H
#define _ETCBOARDINFO_H

#include <string>

namespace DBTREE
{
    struct ETCBOARDINFO
    {
        std::string name;
        std::string url;
        std::string basicauth; // basic認証 ID:PASS
        std::string boardid;  // navi2chのetc形式との互換のため
    };
}

#endif
