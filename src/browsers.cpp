// ライセンス: GPL2

#include "browsers.h"

enum
{
    MAX_TEXT = 256,
    BROWSER_NUM = 6
};

namespace CORE
{
    char browsers[ BROWSER_NUM ][ 2 ][ MAX_TEXT ]={

        { "ユーザ設定", "" },
#ifdef _WIN32
        { "ie",    "C:/Program\\ Files/Internet\\ Explorer/iexplore.exe \"%LINK\"" },
        { "firefox 1.5/2.0/3.0 以降", "C:/Program\\ Files/Mozilla\\ Firefox/firefox.exe \"%LINK\"" },
        { "firefox 1.0*", "C:/Program\\ Files/Mozilla\\ Firefox/firefox.exe -remote \"openURL(%LINK,new-tab)\"" },
        { "opera 9.*",    "C:/Program\\ Files/Opera/opera.exe \"%LINK\"" },
        { "chrome",    "" }
#else
        { "標準ブラウザ(xdg-open)",    "xdg-open \"%LINK\"" },
        { "firefox 1.5/2.0/3.0 以降", "firefox \"%LINK\"" },
        { "firefox 1.0*", "firefox -remote \"openURL(%LINK,new-tab)\"" },
        { "opera 9.*",    "opera -remote \"openURL(%LINK,new-tab)\"" },
        { "chrome",    "google-chrome %U -open \"%LINK\"" }
#endif
    };

    const std::string get_browser_label( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        return browsers[ num ][ 0 ];
    }

    const std::string get_browser_name( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        return browsers[ num ][ 1 ];
    }

    const int get_browser_number(){ return BROWSER_NUM; }
}
