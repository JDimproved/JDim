// ライセンス: GPL2

// スレ一覧のコンテキストメニューの表示項目設定

#ifndef _BOARDITEMMENUPREF_H
#define _BOARDITEMMENUPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class BoardItemMenuPref : public SKELETON::SelectItemPref
    {
      public:

        BoardItemMenuPref( Gtk::Window* parent, const std::string& url );
        ~BoardItemMenuPref() noexcept {}

      private:

        // OKボタン
        void slot_ok_clicked() override;


        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
