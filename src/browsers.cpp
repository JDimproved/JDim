// ライセンス: GPL2

#include "browsers.h"
#include "jdlib/miscutil.h"

enum
{
    MAX_TEXT = 256,
    BROWSER_NUM = 8,
};

namespace CORE
{
    char browsers[ BROWSER_NUM ][ 2 ][ MAX_TEXT ]={

        { "ユーザ設定", "" },
        { "標準ブラウザ(xdg-open)",   "xdg-open \"%LINK\"" },
        { "Firefox",                  "firefox \"%LINK\"" },
        { "konqeror",                 "konqeror \"%LINK\"" },
        { "opera 9.* 以降",           "opera -remote \"openURL(%LINK,new-tab)\"" },
        { "chrome",                   "google-chrome \"%LINK\"" },
        { "chromium",                 "chromium \"%LINK\"" },
        { "w3m",                      "w3m \"%LINK\"" }
    };

    std::string get_browser_label( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        return browsers[ num ][ 0 ];
    }

    std::string get_browser_name( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        return browsers[ num ][ 1 ];
    }

    int get_browser_number(){ return BROWSER_NUM; }
}
