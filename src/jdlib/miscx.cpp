// ライセンス: GPL2

//#define _DEBUG
#include "gtkmmversion.h"
#include "jddebug.h"

#include "miscx.h"

#ifndef _WIN32
#include <gdk/gdkx.h>
#else
#include <gdk/gdkwin32.h>
#include <windows.h>
#endif

#if !defined( GDK_WINDOW_XWINDOW ) && GTKMM_CHECK_VERSION(3,0,0)
#define GDK_WINDOW_XWINDOW( win ) gdk_x11_window_get_xid( win )
#endif

//
// dest ウインドウ上の、クライアント座標 x,y に移動する
//
void MISC::WarpPointer( Glib::RefPtr< Gdk::Window > src, Glib::RefPtr< Gdk::Window > dest, int x, int y ){

#ifndef _WIN32
    XWarpPointer( GDK_WINDOW_XDISPLAY( Glib::unwrap( src ) ), 
                  None,
                  GDK_WINDOW_XWINDOW( Glib::unwrap( dest ) )
                  , 0, 0, 0, 0, x, y );
#else
    HWND hWnd;
    POINT pos;

    // スクリーン座標を取得してからカーソル位置を設定する
    hWnd = (HWND)GDK_WINDOW_HWND( Glib::unwrap( dest ) );
    if( hWnd != NULL ){
        pos.x = x;
        pos.y = y;
        if( ClientToScreen( hWnd, &pos ) ){
            SetCursorPos( pos.x, pos.y );
        }
    }
#endif
}
