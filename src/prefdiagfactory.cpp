// ライセンス: GPL2

#include "prefdiagfactory.h"

#include "passwdpref.h"
#include "privacypref.h"
#include "browserpref.h"
#include "proxypref.h"
#include "globalabonepref.h"
#include "globalabonethreadpref.h"
#include "fontcolorpref.h"
#include "boarditempref.h"

#include "dbimg/delimgdiag.h"

#include "board/preference.h"
#include "article/preference.h"
#include "image/preference.h"

SKELETON::PrefDiag* CORE::PrefDiagFactory( Gtk::Window* parent, int type, const std::string& url )
{
    switch( type )
    {
        case PREFDIAG_PASSWD:
            return new CORE::PasswdPref( parent, url );

        case PREFDIAG_PRIVARY:
            return new CORE::PrivacyPref( parent, url );

        case PREFDIAG_BROWSER:
            return new CORE::BrowserPref( parent, url );

        case PREFDIAG_PROXY:
            return new CORE::ProxyPref( parent, url );

        case PREFDIAG_GLOBALABONE:
            return new CORE::GlobalAbonePref( parent, url );

        case PREFDIAG_GLOBALABONETHREAD:
            return new CORE::GlobalAboneThreadPref( parent, url );

        case PREFDIAG_FONTCOLOR:
            return new CORE::FontColorPref( parent, url );

        case PREFDIAG_BOARDITEM:
            return new CORE::BoardItemPref( parent, url );            


        case PREFDIAG_DELIMG:
            return new DBIMG::DelImgDiag( parent, url );



        case PREFDIAG_BOARD:
            return new BOARD::Preferences( parent, url );

        case PREFDIAG_ARTICLE:
            return new ARTICLE::Preferences( parent, url );

        case PREFDIAG_IMAGE:
            return new IMAGE::Preferences( parent, url );


        default:
            return NULL;
    }
}

