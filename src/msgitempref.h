// ライセンス: GPL2

// 書き込みビューのツールバーの表示項目設定

#ifndef _MSGITEMPREF_H
#define _MSGITEMPREF_H

#include "skeleton/selectitempref.h"

namespace CORE
{
    class MsgItemPref : public SKELETON::SelectItemPref
    {
      public:

        MsgItemPref( Gtk::Window* parent, const std::string& url );
        virtual ~MsgItemPref(){}

      private:

        // OK押した
        virtual void slot_ok_clicked();
    };
}

#endif
