// ライセンス: GPL2

// スレビューのコンテキストメニューの表示項目設定

#ifndef _ARTICLEITEMMENUPREF_H
#define _ARTICLEITEMMENUPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class ArticleItemMenuPref : public SKELETON::SelectItemPref
    {
      public:

        ArticleItemMenuPref( Gtk::Window* parent, const std::string& url );
        ~ArticleItemMenuPref() noexcept {}

      private:

        // OKボタン
        void slot_ok_clicked() override;


        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
