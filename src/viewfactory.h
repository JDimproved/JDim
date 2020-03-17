// ライセンス: GPL2

//
// SKELETON::VIEWのファクトリー
//

#ifndef _VIEWFACTORY_H
#define _VIEWFACTORY_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class View;
}

namespace CORE
{
    enum
    {
        VIEW_BBSLISTVIEW,
        VIEW_FAVORITELIST,
        VIEW_SELECTLIST,
        VIEW_HISTTHREAD,
        VIEW_HISTCLOSE,
        VIEW_HISTBOARD,
        VIEW_HISTCLOSEBOARD,
        VIEW_HISTCLOSEIMG,

        VIEW_BOARDVIEW,
        VIEW_BOARDNEXT,
        VIEW_BOARDLOG,
        VIEW_BOARDSIDEBAR,

        VIEW_ARTICLEVIEW,
        VIEW_ARTICLERES,
        VIEW_ARTICLENAME,
        VIEW_ARTICLEID,
        VIEW_ARTICLEBM,
        VIEW_ARTICLEPOST,
        VIEW_ARTICLEHIGHREFRES,
        VIEW_ARTICLEURL,
        VIEW_ARTICLEREFER,
        VIEW_ARTICLEDRAWOUT,
        VIEW_ARTICLEPREVIEW,
        VIEW_ARTICLEINFO,
        VIEW_ARTICLESEARCHLOG,
        VIEW_ARTICLESEARCHALLLOG,
        VIEW_ARTICLESEARCHTITLE,
        VIEW_ARTICLEPOSTLOG,

        VIEW_ARTICLEPOPUPHTML,
        VIEW_ARTICLEPOPUPRES,
        VIEW_ARTICLEPOPUPNAME,
        VIEW_ARTICLEPOPUPID,
        VIEW_ARTICLEPOPUPREFER,
        VIEW_ARTICLEPOPUPDRAWOUT,
        VIEW_ARTICLEPOPUPBM,

        VIEW_IMAGEVIEW,
        VIEW_IMAGEICON,
        VIEW_IMAGEPOPUP,

        VIEW_MESSAGE,
        VIEW_NEWTHREAD,

        VIEW_NONE
    };

    struct VIEWFACTORY_ARGS
    {
        std::string arg1;
        std::string arg2;
        std::string arg3;
        std::string arg4;
    };
    
    SKELETON::View* ViewFactory( int type, const std::string& url, VIEWFACTORY_ARGS view_args = VIEWFACTORY_ARGS() );
}

#endif
