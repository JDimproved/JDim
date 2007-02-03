// ライセンス: GPL2
//
// VBoxクラス
//

#ifndef _VBOX_H
#define _VBOX_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDVBox : public Gtk::VBox
    {
      public:

        JDVBox();
        ~JDVBox();

        // unpack = true の時取り除く
        void pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET, guint padding = 0 );
    };
}

#endif
