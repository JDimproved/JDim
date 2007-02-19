// ライセンス: GPL2

// X 関係の関数

#ifndef _MISCX_H
#define _MISCX_H

#include <gtkmm.h>

namespace MISC
{
    void WarpPointer( Glib::RefPtr< Gdk::Window > src, Glib::RefPtr< Gdk::Window > dest, int x, int y );
}

#endif
