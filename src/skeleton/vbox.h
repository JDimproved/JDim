// ライセンス: GPL2
//
// VBoxクラス
//

#ifndef _VBOX_H
#define _VBOX_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDVBox : public Gtk::Box
    {
      public:

        JDVBox();
        ~JDVBox() noexcept override;

        // unpack = true の時取り除く
        void pack_remove_start( bool unpack, Widget& child, bool expand = true, bool fill = true, guint padding = 0 );
        void pack_remove_end( bool unpack, Widget& child, bool expand = true, bool fill = true, guint padding = 0 );
    };
}

#endif
