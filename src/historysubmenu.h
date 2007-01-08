// ライセンス: GPL2

//
// 履歴サブメニュー
//

#ifndef _HISTORYSUBMENU_H
#define _HISTORYSUBMENU_H

#include <gtkmm.h>
#include <string>

namespace CORE
{
    struct HIST_ITEM
    {
        std::string url;
        std::string name;
        int type;
    };

    class HistorySubMenu : public Gtk::Menu
    {
        std::string m_path_xml;
        std::list< Gtk::MenuItem* > m_itemlist;
        std::list< CORE::HIST_ITEM* > m_histlist;

      public:

        HistorySubMenu( const std::string path_xml );
        ~HistorySubMenu();

        void clear();
        void append_item( const std::string& url, const std::string& name, int type );
        void update();

        // ラベルをセット
        void set_menulabel();

      private:

        void xml2list( const std::string& xml );
        std::string list2xml();

        // メニューアイテムがactiveになった
        virtual void slot_activate( int i );
    };
}

#endif
