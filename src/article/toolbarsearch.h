// ライセンス: GPL2

// ログ検索などのツールバーのクラス
//
// ARTICLE::ArticleViewSearch 以外では使わない
//

#ifndef _ARTICLE_TOOLBARSEARCH_H
#define _ARTICLE_TOOLBARSEARCH_H

#include "skeleton/toolbar.h"

namespace ARTICLE
{
    class SearchToolBar : public SKELETON::ToolBar
    {
      public:

        SearchToolBar();
        virtual ~SearchToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
