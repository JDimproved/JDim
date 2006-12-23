// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscx.h"

#include <gdk/gdkx.h>

void MISC::WarpPointer( Glib::RefPtr< Gdk::Window > src, Glib::RefPtr< Gdk::Window > dest, int x, int y ){

    XWarpPointer( GDK_WINDOW_XDISPLAY( Glib::unwrap( src ) ), 
                  None,
                  GDK_WINDOW_XWINDOW( Glib::unwrap( dest ) )
                  , 0, 0, 0, 0, x, y );
}
