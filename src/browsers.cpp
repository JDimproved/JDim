// ライセンス: GPL2

#include "browsers.h"
#include "jdlib/miscutil.h"

enum
{
    MAX_TEXT = 256,
    BROWSER_NUM = 8,
#ifdef _WIN32
    MAX_SAFE_PATH = 1024,
#endif
};

namespace CORE
{
    char browsers[ BROWSER_NUM ][ 2 ][ MAX_TEXT ]={

        { "ユーザ設定", "" },
#ifdef _WIN32
        { "ie",                       "\"C:/Program Files/Internet Explorer/iexplore.exe\" \"%LINK\"" },
        { "Firefox",                  "\"C:/Program Files/Mozilla Firefox/firefox.exe\" \"%LINK\"" },
        { "opera 9.* 以降",           "\"C:/Program Files/Opera/opera.exe\" \"%LINK\"" },
        { "chrome (Vista/7)",         "\"%USERPROFILE%/AppData/Local/Google/Chrome/Application/chrome.exe\" \"%LINK\"" },
        { "chrome (XP)",              "\"%USERPROFILE%/Local Settings/Application Data/Google/Chrome/Application/chrome.exe\" \"%LINK\"" },
        { "chrome (Google pack)",     "\"C:/Program Files/Google/Chrome/Application/chrome.exe\" \"%LINK\"" }
#else
        { "標準ブラウザ(xdg-open)",   "xdg-open \"%LINK\"" },
        { "Firefox",                  "firefox \"%LINK\"" },
        { "konqeror",                 "konqeror \"%LINK\"" },
        { "opera 9.* 以降",           "opera -remote \"openURL(%LINK,new-tab)\"" },
        { "chrome",                   "google-chrome \"%LINK\"" },
        { "chromium",                 "chromium \"%LINK\"" },
        { "w3m",                      "w3m \"%LINK\"" }
#endif
    };

    std::string get_browser_label( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        return browsers[ num ][ 0 ];
    }

    std::string get_browser_name( const int num ){

        if( num >= BROWSER_NUM ) return std::string();
        std::string name = browsers[ num ][ 1 ];
#ifdef _WIN32
        if( name.find( "%USERPROFILE%", 0 ) != std::string::npos ){
            std::string home = MISC::getenv_limited( "USERPROFILE" , MAX_SAFE_PATH );
            name = MISC::replace_str( name, "%USERPROFILE%", home );
        }
#endif
        return name;
    }

    int get_browser_number(){ return BROWSER_NUM; }
}
