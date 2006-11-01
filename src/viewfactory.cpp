// ライセンス: 最新のGPL

#include "viewfactory.h"

#include "bbslist/bbslistview.h"
#include "bbslist/etclistview.h"
#include "bbslist/favoriteview.h"
#include "bbslist/selectlistview.h"

#include "board/boardview.h"

#include "article/articleview.h"
#include "article/articleviewpreview.h"
#include "article/articleviewpopup.h"

#include "image/imageview.h"
#include "image/imageviewicon.h"
#include "image/imageviewpopup.h"

#include "message/messageview.h"

SKELETON::View* CORE::ViewFactory( int type, const std::string& url, VIEWFACTORY_ARGS args )
{
    switch( type )
    {
        case VIEW_BBSLISTVIEW:
            return new BBSLIST::BBSListViewMain( url, args.arg1, args.arg2 );

        case VIEW_ETCLIST:
            return new BBSLIST::EtcListView( url, args.arg1, args.arg2 );

        case VIEW_FAVORITELIST:
            return new BBSLIST::FavoriteListView( url, args.arg1, args.arg2 );

        case VIEW_SELECTLIST:
            return new BBSLIST::SelectListView( url, args.arg1, args.arg2 );

            //////////////////

        case VIEW_BOARDVIEW:
            return new BOARD::BoardView( url, args.arg1, args.arg2 );

            /////////////////

        case VIEW_ARTICLEVIEW:
            return new ARTICLE::ArticleViewMain( url );

        case VIEW_ARTICLERES:
            return new ARTICLE::ArticleViewRes( url,  args.arg1, ( args.arg2 == "true" ), args.arg3 );

        case VIEW_ARTICLENAME:
            return new ARTICLE::ArticleViewName( url, args.arg1 );

        case VIEW_ARTICLEID:
            return new ARTICLE::ArticleViewID( url, args.arg1 );

        case VIEW_ARTICLEBM:
            return new ARTICLE::ArticleViewBM( url );

        case VIEW_ARTICLEURL:
            return new ARTICLE::ArticleViewURL( url );

        case VIEW_ARTICLEREFER:
            return new ARTICLE::ArticleViewRefer( url, args.arg1 );

        case VIEW_ARTICLEDRAWOUT:
            return new ARTICLE::ArticleViewDrawout( url, args.arg1, ( args.arg2 == "OR" ) );

        case VIEW_ARTICLEPREVIEW:
            return new ARTICLE::ArticleViewPreview( url );

            /////////////////

        case VIEW_ARTICLEPOPUPHTML:
            return new ARTICLE::ArticleViewPopupHTML( url, args.arg1 );

        case VIEW_ARTICLEPOPUPRES:
            return new ARTICLE::ArticleViewPopupRes( url, args.arg1, ( args.arg2 == "true" ), ( args.arg3 == "true" ) );

        case VIEW_ARTICLEPOPUPNAME:
            return new ARTICLE::ArticleViewPopupName( url, args.arg1 );

        case VIEW_ARTICLEPOPUPID:
            return new ARTICLE::ArticleViewPopupID( url, args.arg1 );

        case VIEW_ARTICLEPOPUPREFER:
            return new ARTICLE::ArticleViewPopupRefer( url, args.arg1 );

            /////////////////

        case VIEW_IMAGEVIEW:
            return new IMAGE::ImageViewMain( url );

        case VIEW_IMAGEICON:
            return new IMAGE::ImageViewIcon( url );

        case VIEW_IMAGEPOPUP:
            return new IMAGE::ImageViewPopup( url );

            /////////////////

        case VIEW_MESSAGE:
            return new MESSAGE::MessageViewMain( url, args.arg1 );

        case VIEW_NEWTHREAD:
            return new MESSAGE::MessageViewNew( url, args.arg1 );

        default:
            return NULL;
    }
}

