// ライセンス: GPL2

// メインツールバーの表示項目設定

#ifndef _MAINITEMPREF_H
#define _MAINITEMPREF_H

#include "skeleton/selectitempref.h"

#include <string>

namespace CORE
{
    class MainItemPref : public SKELETON::SelectItemPref
    {
      public:

        MainItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~MainItemPref(){}

      private:

        // OK押した
        virtual void slot_ok_clicked();

        // デフォルトボタン
        virtual void slot_def();
    };
}

#endif
