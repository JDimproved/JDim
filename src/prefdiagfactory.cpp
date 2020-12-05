// ライセンス: GPL2

#include "prefdiagfactory.h"

#include "passwdpref.h"
#include "privacypref.h"
#include "browserpref.h"
#include "linkfilterpref.h"
#include "replacestrpref.h"
#include "usrcmdpref.h"
#include "proxypref.h"
#include "globalabonepref.h"
#include "globalabonethreadpref.h"
#include "fontcolorpref.h"
#include "livepref.h"
#include "openurldiag.h"

#include "mainitempref.h"
#include "sidebaritempref.h"
#include "boarditempref.h"
#include "boarditemmenupref.h"
#include "articleitempref.h"
#include "articleitemmenupref.h"
#include "searchitempref.h"
#include "msgitempref.h"

#include "dbimg/delimgdiag.h"

#include "board/preference.h"
#include "article/preference.h"
#include "image/preference.h"

#include "control/keypref.h"
#include "control/mousepref.h"
#include "control/buttonpref.h"

#include "config/aboutconfig.h"

SKELETON::PrefDiag* CORE::PrefDiagFactory( Gtk::Window* parent, const int type, const std::string& url,
                                           const std::string& command )
{
    switch( type )
    {
        case PREFDIAG_PASSWD:
            return new CORE::PasswdPref( parent, url );

        case PREFDIAG_PRIVACY:
            return new CORE::PrivacyPref( parent, url );

        case PREFDIAG_BROWSER:
            return new CORE::BrowserPref( parent, url );

        case PREFDIAG_LINKFILTER:
            return new CORE::LinkFilterPref( parent, url );

        case PREFDIAG_REPLACESTR:
            return new CORE::ReplaceStrPref( parent, url );

        case PREFDIAG_USRCMD:
            return new CORE::UsrCmdPref( parent, url );

        case PREFDIAG_PROXY:
            return new CORE::ProxyPref( parent, url );

        case PREFDIAG_GLOBALABONE:
            return new CORE::GlobalAbonePref( parent, url );

        case PREFDIAG_GLOBALABONETHREAD:
            return new CORE::GlobalAboneThreadPref( parent, url );

        case PREFDIAG_FONTCOLOR:
            return new CORE::FontColorPref( parent, url );

        case PREFDIAG_LIVE:
            return new CORE::LivePref( parent, url );

        case PREFDIAG_MAINITEM:
            return new CORE::MainItemPref( parent, url );            

        case PREFDIAG_SIDEBARITEM:
            return new CORE::SidebarItemPref( parent, url );            

        case PREFDIAG_BOARDITEM_COLUM:
            return new CORE::BoardItemColumnPref( parent, url );            

        case PREFDIAG_BOARDITEM:
            return new CORE::BoardItemPref( parent, url );            

        case PREFDIAG_BOARDITEM_MENU:
            return new CORE::BoardItemMenuPref( parent, url );

        case PREFDIAG_ARTICLEITEM:
            return new CORE::ArticleItemPref( parent, url );            

        case PREFDIAG_ARTICLEITEM_MENU:
            return new CORE::ArticleItemMenuPref( parent, url );

        case PREFDIAG_SEARCHITEM:
            return new CORE::SearchItemPref( parent, url );            

        case PREFDIAG_MSGITEM:
            return new CORE::MsgItemPref( parent, url );            

        case PREFDIAG_DELIMG:
            return new DBIMG::DelImgDiag( parent, url );

        case PREFDIAG_BOARD:
            return new BOARD::Preferences( parent, url, command );

        case PREFDIAG_ARTICLE:
            return new ARTICLE::Preferences( parent, url, command );

        case PREFDIAG_IMAGE:
            return new IMAGE::Preferences( parent, url );

        case PREFDIAG_KEY:
            return new CONTROL::KeyPref( parent, url );

        case PREFDIAG_MOUSE:
            return new CONTROL::MousePref( parent, url );

        case PREFDIAG_BUTTON:
            return new CONTROL::ButtonPref( parent, url );

        case PREFDIAG_ABOUTCONFIG:
            return new CONFIG::AboutConfig( parent );

        case PREFDIAG_OPENURL:
            return new CORE::OpenURLDialog( url );

        default:
            return nullptr;
    }
}
