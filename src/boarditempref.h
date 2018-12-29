// ライセンス: GPL2

// スレ一覧のツールバーと列の表示項目設定

#ifndef _BOARDITEMPREF_H
#define _BOARDITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class BoardItemColumnPref : public SKELETON::SelectItemPref
    {
      public:

        BoardItemColumnPref( Gtk::Window* parent, const std::string& url );
        ~BoardItemColumnPref() noexcept {}

      private:

        // OK押した
        void slot_ok_clicked() override;

        // デフォルトボタン
        void slot_default() override;
    };

    class BoardItemPref : public SKELETON::SelectItemPref
    {
      public:

        BoardItemPref( Gtk::Window* parent, const std::string& url );
        ~BoardItemPref() noexcept {}

      private:

        // OK押した
        void slot_ok_clicked() override;

        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
