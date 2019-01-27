// ライセンス: GPL2

// メインツールバーの表示項目設定

#ifndef _MAINITEMPREF_H
#define _MAINITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class MainItemPref : public SKELETON::SelectItemPref
    {
      public:

        MainItemPref( Gtk::Window* parent, const std::string& url );
        ~MainItemPref() noexcept {}

      private:

        // OK押した
        void slot_ok_clicked() override;

        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
