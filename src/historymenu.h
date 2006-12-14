// ライセンス: GPL2

//
// 履歴メニュー
//

#ifndef _HISTORYMENU_H
#define _HISTORYMENU_H

#include <gtkmm.h>

namespace CORE
{
    class HistorySubMenu;

    class HistoryMenuBase : public Gtk::MenuItem
    {
        CORE::HistorySubMenu* m_submenu;

      public:

        HistoryMenuBase( const std::string& label );
        virtual ~HistoryMenuBase();

        void append( const std::string& url, const std::string& name, int type );

      protected:

        void setup( CORE::HistorySubMenu* submenu );

      private:

        // メニューがactiveになった時にラベルをセットする
        void slot_activate_menu();

        // 履歴クリア
        void slot_clear();
    };



    // スレ履歴
    class HistoryMenuThread : public CORE::HistoryMenuBase
    {
      public:
        HistoryMenuThread();
    };


    // 板履歴
    class HistoryMenuBoard : public CORE::HistoryMenuBase
    {
      public:
        HistoryMenuBoard();
    };

}

#endif
