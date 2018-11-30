// ライセンス: GPL2

// 「戻る」「進む」履歴付きボタン

#ifndef _BACKFORWARDBUTTON_H
#define _BACKFORWARDBUTTON_H

#include "menubutton.h"

namespace SKELETON
{
    class BackForwardButton : public SKELETON::MenuButton
    {
        std::string m_url;
        bool m_back;

      public:

        BackForwardButton( const std::string& url, const bool back );

        void set_url( const std::string& url );

      protected:

      // ポップアップメニュー表示
      void show_popupmenu() override;
    };
}

#endif
