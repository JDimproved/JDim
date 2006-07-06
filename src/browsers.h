// ライセンス: 最新のGPL

// ブラウザ設定

#ifndef _BROWSERS_H
#define _BROWSERS_H

#define MAX_TEXT 256

namespace CORE
{
    char browsers[][ 2 ][ MAX_TEXT ]={

        { "ユーザ設定", "" },
        { "RedHat系標準ブラウザ",    "htmlview %s" },
        { "firefox 1.5*", "firefox %s" },
        { "firefox 1.0*", "firefox -remote \"openURL(%s,new-tab)\"" },
        { "opera 9.*",    "opera -remote \"openURL(%s,new-tab)\"" },
        
        { "",""}
    };
}


#endif
