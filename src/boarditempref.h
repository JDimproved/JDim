// ライセンス: GPL2

// スレ一覧のツールバーと列の表示項目設定

#ifndef _BOARDITEMPREF_H
#define _BOARDITEMPREF_H

#include "skeleton/selectitempref.h"

#include <string>

namespace CORE
{
    class BoardItemColumnPref : public SKELETON::SelectItemPref
    {
      public:

        BoardItemColumnPref( Gtk::Window* parent, const std::string& url );
        virtual ~BoardItemColumnPref(){}

      private:

        // OK押した
        virtual void slot_ok_clicked();

        // デフォルトボタン
        virtual void slot_def();
    };

    class BoardItemPref : public SKELETON::SelectItemPref
    {
      public:

        BoardItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~BoardItemPref(){}

      private:

        // OK押した
        virtual void slot_ok_clicked();

        // デフォルトボタン
        virtual void slot_def();
    };
}

#endif
