// License: GPL2

#ifndef _TOOLS_H
#define _TOOLS_H

#include <string>
#include <gtkmm.h>


namespace XML
{
	// TYPE_ID から要素名を取得( TYPE_ID は global.h を参照 )
    std::string get_name( const int type_id );

    // 要素名から TYPE_ID を取得
    int get_type( const std::string& node_name );

    // TYPE_ID からアイコンを取得 ( アイコンは icons/iconmanager.h を参照 )
    Glib::RefPtr< Gio::Icon > get_icon( const int type_id );
}

#endif
