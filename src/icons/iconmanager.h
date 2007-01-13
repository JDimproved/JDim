// ライセンス: GPL2

//
// アイコンの管理クラス
//

#ifndef _ICONNAGER_H
#define _ICONNAGER_H

#include <gtkmm.h>
#include <vector>

namespace ICON
{
    enum
    {
        JD16,
        JD32,
        JD48,

        BKMARK_UPDATE,
        BKMARK,
        UPDATE,
        NEWTHREAD,
        CHECK,
        DOWN,
        LOADING,
        LOADING_STOP,

        DIR,
        FAVORITE,
        BOARD,
        BOARD_UPDATE,
        THREAD,
        THREAD_UPDATE,
        IMAGE,
        LINK,

        TRANSPARENT,

        NUM_ICONS
    };

    class ICON_Manager
    {
        std::vector< Glib::RefPtr< Gdk::Pixbuf > > m_list_icons;

      public:

        ICON_Manager();
        virtual ~ICON_Manager();

        Glib::RefPtr< Gdk::Pixbuf > get_icon( int id );
    };

    ///////////////////////////////////////
    // インターフェース

    ICON_Manager* get_icon_manager();
    void delete_icon_manager();

    Glib::RefPtr< Gdk::Pixbuf > get_icon( int id );
}

#endif
