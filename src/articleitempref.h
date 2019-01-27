// ライセンス: GPL2

// スレビューのツールバーの表示項目設定

#ifndef _ARTICLEITEMPREF_H
#define _ARTICLEITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class ArticleItemPref : public SKELETON::SelectItemPref
    {
      public:

        ArticleItemPref( Gtk::Window* parent, const std::string& url );
        ~ArticleItemPref() noexcept {}

      private:

        // OKボタン
        void slot_ok_clicked() override;

        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
