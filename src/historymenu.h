// ライセンス: 最新のGPL

//
// 履歴メニュー
//

#ifndef _HISTORYMENU_H
#define _HISTORYMENU_H

#include <gtkmm.h>

namespace CORE
{
    class HistorySubMenu;

    class HistoryMenu : public Gtk::MenuItem
    {
        CORE::HistorySubMenu* m_submenu;
        CORE::HistorySubMenu* m_submenu_board;

      public:

        HistoryMenu();
        ~HistoryMenu();

        void append( const std::string& url, const std::string& name, int type );

      private:

        // メニューがactiveになった時にラベルをセットする
        void slot_activate_menu();

        // 履歴クリア
        void slot_clear();
    };
}

#endif
