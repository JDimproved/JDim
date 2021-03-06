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

std::unique_ptr<SKELETON::PrefDiag> CORE::PrefDiagFactory( Gtk::Window* parent, const int type, const std::string& url,
                                                           const std::string& command )
{
    switch( type )
    {
        case PREFDIAG_PASSWD:
            return std::make_unique<CORE::PasswdPref>( parent, url );

        case PREFDIAG_PRIVACY:
            return std::make_unique<CORE::PrivacyPref>( parent, url );

        case PREFDIAG_BROWSER:
            return std::make_unique<CORE::BrowserPref>( parent, url );

        case PREFDIAG_LINKFILTER:
            return std::make_unique<CORE::LinkFilterPref>( parent, url );

        case PREFDIAG_REPLACESTR:
            return std::make_unique<CORE::ReplaceStrPref>( parent, url );

        case PREFDIAG_USRCMD:
            return std::make_unique<CORE::UsrCmdPref>( parent, url );

        case PREFDIAG_PROXY:
            return std::make_unique<CORE::ProxyPref>( parent, url );

        case PREFDIAG_GLOBALABONE:
            return std::make_unique<CORE::GlobalAbonePref>( parent, url );

        case PREFDIAG_GLOBALABONETHREAD:
            return std::make_unique<CORE::GlobalAboneThreadPref>( parent, url );

        case PREFDIAG_FONTCOLOR:
            return std::make_unique<CORE::FontColorPref>( parent, url );

        case PREFDIAG_LIVE:
            return std::make_unique<CORE::LivePref>( parent, url );

        case PREFDIAG_MAINITEM:
            return std::make_unique<CORE::MainItemPref>( parent, url );

        case PREFDIAG_SIDEBARITEM:
            return std::make_unique<CORE::SidebarItemPref>( parent, url );

        case PREFDIAG_BOARDITEM_COLUM:
            return std::make_unique<CORE::BoardItemColumnPref>( parent, url );

        case PREFDIAG_BOARDITEM:
            return std::make_unique<CORE::BoardItemPref>( parent, url );

        case PREFDIAG_BOARDITEM_MENU:
            return std::make_unique<CORE::BoardItemMenuPref>( parent, url );

        case PREFDIAG_ARTICLEITEM:
            return std::make_unique<CORE::ArticleItemPref>( parent, url );

        case PREFDIAG_ARTICLEITEM_MENU:
            return std::make_unique<CORE::ArticleItemMenuPref>( parent, url );

        case PREFDIAG_SEARCHITEM:
            return std::make_unique<CORE::SearchItemPref>( parent, url );

        case PREFDIAG_MSGITEM:
            return std::make_unique<CORE::MsgItemPref>( parent, url );

        case PREFDIAG_DELIMG:
            return std::make_unique<DBIMG::DelImgDiag>( parent, url );

        case PREFDIAG_BOARD:
            return std::make_unique<BOARD::Preferences>( parent, url, command );

        case PREFDIAG_ARTICLE:
            return std::make_unique<ARTICLE::Preferences>( parent, url, command );

        case PREFDIAG_IMAGE:
            return std::make_unique<IMAGE::Preferences>( parent, url );

        case PREFDIAG_KEY:
            return std::make_unique<CONTROL::KeyPref>( parent, url );

        case PREFDIAG_MOUSE:
            return std::make_unique<CONTROL::MousePref>( parent, url );

        case PREFDIAG_BUTTON:
            return std::make_unique<CONTROL::ButtonPref>( parent, url );

        case PREFDIAG_ABOUTCONFIG:
            return std::make_unique<CONFIG::AboutConfig>( parent );

        case PREFDIAG_OPENURL:
            return std::make_unique<CORE::OpenURLDialog>( url );

        default:
            return nullptr;
    }
}
