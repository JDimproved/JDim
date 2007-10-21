// ライセンス: GPL2

// スレビューのツールバーの表示項目設定

#ifndef _ARTICLEITEMPREF_H
#define _ARTICLEITEMPREF_H

#include "skeleton/selectitempref.h"

#include <string>

namespace CORE
{
    class ArticleItemPref : public SKELETON::SelectItemPref
    {
      public:

        ArticleItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~ArticleItemPref(){}

      private:

        // OKボタン
        virtual void slot_ok_clicked();

        // デフォルトボタン
        virtual void slot_def();
    };
}

#endif
