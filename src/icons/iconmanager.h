// ライセンス: GPL2

//
// アイコンの管理クラス
//

#ifndef _ICONNAGER_H
#define _ICONNAGER_H

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

        static void load_builtin_icons( std::vector<Glib::RefPtr<Gdk::Pixbuf>>& list_icons );
    };

    ///////////////////////////////////////
    // インターフェース

    ICON_Manager* get_icon_manager();
    void delete_icon_manager();

    Glib::RefPtr< Gdk::Pixbuf > get_icon( const int id );
    std::vector<std::string> get_installed_gtk_theme_names();
    std::vector<std::string> get_installed_icon_theme_names();
}

#endif
