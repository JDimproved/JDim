// ライセンス: GPL2

//
// SKELETON::PrefDiagのファクトリー
//

#ifndef _PREFDIAGFACTORY_H
#define _PREFDIAGFACTORY_H

#include "skeleton/prefdiag.h"

#include <gtkmm.h>
#include <string>

namespace CORE
{
    enum
    {
        PREFDIAG_PASSWD,
        PREFDIAG_PRIVACY,
        PREFDIAG_BROWSER,
        PREFDIAG_LINKFILTER,
        PREFDIAG_REPLACESTR,
        PREFDIAG_USRCMD,
        PREFDIAG_PROXY,
        PREFDIAG_GLOBALABONETHREAD,
        PREFDIAG_GLOBALABONE,
        PREFDIAG_FONTCOLOR,
        PREFDIAG_LIVE,

        PREFDIAG_MAINITEM,
        PREFDIAG_SIDEBARITEM,
        PREFDIAG_BOARDITEM,
        PREFDIAG_BOARDITEM_MENU,
        PREFDIAG_BOARDITEM_COLUM,
        PREFDIAG_ARTICLEITEM,
        PREFDIAG_ARTICLEITEM_MENU,
        PREFDIAG_SEARCHITEM,
        PREFDIAG_MSGITEM,

        PREFDIAG_DELIMG,

        PREFDIAG_BOARD,
        PREFDIAG_ARTICLE,
        PREFDIAG_IMAGE,

        PREFDIAG_KEY,
        PREFDIAG_MOUSE,
        PREFDIAG_BUTTON,

        PREFDIAG_ABOUTCONFIG,

        PREFDIAG_OPENURL,

        PREFDIAG_NONE
    };
    
    SKELETON::PrefDiag* PrefDiagFactory( Gtk::Window* parent, const int type, const std::string& url,
                                         const std::string& command = {} );
}

#endif
