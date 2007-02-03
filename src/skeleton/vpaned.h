// ライセンス: GPL2
//
// VPandeクラス
//

#ifndef _VPANED_H
#define _VPANED_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDVPaned : public Gtk::VPaned
    {
      public:

        JDVPaned();
        ~JDVPaned();

        // unpack = true の時取り除く
        void add_remove1( bool unpack, Gtk::Widget& child );
        void add_remove2( bool unpack, Gtk::Widget& child );
    };
}

#endif
