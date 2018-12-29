// ライセンス: GPL2

// サイドバーのツールバーの表示項目設定

#ifndef _SIDEBARITEMPREF_H
#define _SIDEBARITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class SidebarItemPref : public SKELETON::SelectItemPref
    {
      public:

        SidebarItemPref( Gtk::Window* parent, const std::string& url  );
        ~SidebarItemPref() noexcept {}

      private:

        // OK押した
        void slot_ok_clicked() override;

        // デフォルトボタン
        void slot_default() override;
    };
}

#endif
