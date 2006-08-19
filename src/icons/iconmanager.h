// ライセンス: 最新のGPL

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
        ICON_JD16,
        ICON_JD32,
        ICON_JD48,

        ICON_ADD16,
        ICON_CHECK16,
        ICON_DOWN16,

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
