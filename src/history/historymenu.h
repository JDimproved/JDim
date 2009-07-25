// ライセンス: GPL2

//
// 履歴メニュー
//

#ifndef _HISTORYMENU_H
#define _HISTORYMENU_H

#include <gtkmm.h>

namespace HISTORY
{
    class HistorySubMenu;

    class HistoryMenu : public Gtk::MenuItem
    {
        HistorySubMenu* m_submenu;
        bool m_activate;

      public:

        HistoryMenu( const std::string& url_history, const std::string& label );

        void set_menulabel();

      private:

        // メニューがactiveになった時にラベルをセットする
        void slot_activate_menu();

        void slot_deactivate_menu();
    };
}

#endif
