// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscx.h"

#ifndef _WIN32
#include <gdk/gdkx.h>
#endif

void MISC::WarpPointer( Glib::RefPtr< Gdk::Window > src, Glib::RefPtr< Gdk::Window > dest, int x, int y ){

#ifndef _WIN32
    XWarpPointer( GDK_WINDOW_XDISPLAY( Glib::unwrap( src ) ), 
                  None,
                  GDK_WINDOW_XWINDOW( Glib::unwrap( dest ) )
                  , 0, 0, 0, 0, x, y );
#endif
}
