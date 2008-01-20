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

    class HistoryMenuBase : public Gtk::MenuItem
    {
        HistorySubMenu* m_submenu;

      public:

        HistoryMenuBase( const std::string& label );
        virtual ~HistoryMenuBase();

        void append( const std::string& url, const std::string& name, int type );
        void update();

        // 履歴クリア
        void slot_clear();

      protected:

        void setup( HistorySubMenu* submenu );

      private:

        // メニューがactiveになった時にラベルをセットする
        void slot_activate_menu();
    };



    // スレ履歴
    class HistoryMenuThread : public HistoryMenuBase
    {
      public:
        HistoryMenuThread();
    };


    // 板履歴
    class HistoryMenuBoard : public HistoryMenuBase
    {
      public:
        HistoryMenuBoard();
    };


    // 最近閉じたスレ履歴
    class HistoryMenuClose : public HistoryMenuBase
    {
      public:
        HistoryMenuClose();
    };
}

#endif
