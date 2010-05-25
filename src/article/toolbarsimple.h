// ライセンス: GPL2

// 簡易版ツールバーのクラス

#ifndef _ARTICLE_TOOLBARSIMPLE_H
#define _ARTICLE_TOOLBARSIMPLE_H

#include "skeleton/toolbar.h"

namespace ARTICLE
{
    class ArticleToolBarSimple : public SKELETON::ToolBar
    {
      public:

        ArticleToolBarSimple();
        virtual ~ArticleToolBarSimple(){}

      protected:

        virtual void pack_buttons();
    };
}

#endif
