// ライセンス: GPL2

//
// 履歴サブメニュー
//

#ifndef _HISTORYSUBMENU_H
#define _HISTORYSUBMENU_H

#include <gtkmm.h>
#include <string>

namespace HISTORY
{
    class HistorySubMenu : public Gtk::Menu
    {
        std::string m_url_history;

        std::vector< Gtk::Image* > m_vec_images;
        std::vector< Gtk::Label* > m_vec_label;

        // ポップアップメニュー
        Gtk::Menu m_popupmenu;
        int m_number_menuitem;

      public:

        HistorySubMenu( const std::string& url_history );
        ~HistorySubMenu();

        // 履歴の先頭を復元
        void restore_history();

        // アクティブ時にラベルをセットする
        void set_menulabel();

      private:

        bool open_history( const int i );

        // メニューのslot関数
        void slot_clear();
        void slot_switch_sideber();

        // メニューアイテムがactiveになった
        void slot_active( const int i );
        bool slot_button_press( GdkEventButton* event, int i );

        // ポップアップメニューのslot
        void slot_open_history();
        void slot_remove_history();
        void slot_show_property();
    };
}

#endif
