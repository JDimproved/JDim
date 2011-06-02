// ライセンス: GPL2

// ログ/スレタイ検索のツールバーの表示項目設定

#ifndef _SEARCHITEMPREF_H
#define _SEARCHITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class SearchItemPref : public SKELETON::SelectItemPref
    {
      public:

        SearchItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~SearchItemPref(){}

      private:

        // OKボタン
        virtual void slot_ok_clicked();

        // デフォルトボタン
        virtual void slot_default();
    };
}

#endif
