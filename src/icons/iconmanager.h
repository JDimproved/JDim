// ライセンス: GPL2

//
// アイコンの管理クラス
//

#ifndef _ICONNAGER_H
#define _ICONNAGER_H

#ifdef _WIN32
 #ifdef TRANSPARENT
 // will be conflict to wingdi.h
 #undef TRANSPARENT
 #endif
#endif

#include "iconid.h"

#include <gtkmm.h>
#include <vector>

namespace ICON
{
    class ICON_Manager
    {
        std::vector< Glib::RefPtr< Gdk::Pixbuf > > m_list_icons;

      public:

        ICON_Manager();
        virtual ~ICON_Manager();

        Glib::RefPtr< Gdk::Pixbuf > get_icon( const int id );

      private:

        void load_theme();
    };

    ///////////////////////////////////////
    // インターフェース

    ICON_Manager* get_icon_manager();
    void delete_icon_manager();

    Glib::RefPtr< Gdk::Pixbuf > get_icon( const int id );
}

#endif
