// ライセンス: GPL2

// ツールバーに表示する SKELETON::MenuButton

// メニューがオーバーフローしたときにアイコン、または label を表示して
// 選択したら MenuButton::on_clicked() を呼び出す

#ifndef _TOOLMENUBUTTON_H
#define _TOOLMENUBUTTON_H

#include "menubutton.h"

namespace SKELETON
{
    class ToolMenuButton : public Gtk::ToolItem
    {
      public:

        ToolMenuButton( SKELETON::MenuButton& button, const std::string& label, const bool expand );
    };
}

#endif
