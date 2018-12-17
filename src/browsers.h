// ライセンス: GPL2

// ブラウザ設定

#ifndef _BROWSERS_H
#define _BROWSERS_H

#include <string>

namespace CORE
{
    std::string get_browser_label( const int num );

    std::string get_browser_name( const int num );

    int get_browser_number();
}


#endif
