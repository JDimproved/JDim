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
        std::vector<Glib::RefPtr<Gio::Icon>> m_list_icons;

      public:

        ICON_Manager();
        virtual ~ICON_Manager();

        Glib::RefPtr<Gio::Icon> get_icon( const int id );
        Glib::RefPtr<Gdk::Pixbuf> get_pixbuf( const int id );

        void reload_themed_icons( const bool use_symbolic );

      private:

        void load_theme();

        static void load_builtin_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons );
        static void load_themed_color_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons );
        static void load_themed_symbolic_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons );
    };

    ///////////////////////////////////////
    // インターフェース

    ICON_Manager* get_icon_manager();
    void delete_icon_manager();

    Glib::RefPtr<Gio::Icon> get_icon( const int id );
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf( const int id );
    std::vector<std::string> get_installed_gtk_theme_names();
    std::vector<std::string> get_installed_icon_theme_names();
}

#endif
