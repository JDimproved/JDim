// ライセンス: GPL2

// キー名 <-> keysym 変換テーブル

#ifndef _KEYSYMS_H
#define _KEYSYMS_H

#include "gtkmmversion.h"

#if GTKMM_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms-compat.h>
#endif

enum
{
    MAX_KEYNAME = 64
};


struct KEYSYMS
{
    char keyname[ MAX_KEYNAME ];
    size_t keysym;
};


namespace CONTROL
{
    KEYSYMS keysyms[] ={

        { "Space", GDK_space },
        { "Escape", GDK_Escape },
        { "Delete", GDK_Delete },
        { "Enter", GDK_Return },

        { "Up", GDK_Up },
        { "Down", GDK_Down },
        { "Left", GDK_Left },
        { "Right", GDK_Right },
        { "Page_Up", GDK_Page_Up },
        { "Page_Down", GDK_Page_Down },
        { "Tab", GDK_Tab },
        { "Left_Tab", GDK_ISO_Left_Tab },
        { "Home", GDK_Home },
        { "End", GDK_End },
        { "BackSpace", GDK_BackSpace },

        { "F1", GDK_F1 },
        { "F2", GDK_F2 },
        { "F3", GDK_F3 },
        { "F4", GDK_F4 },
        { "F5", GDK_F5 },
        { "F6", GDK_F6 },
        { "F7", GDK_F7 },
        { "F8", GDK_F8 },
        { "F9", GDK_F9 },
        { "F10", GDK_F10 },
        { "F11", GDK_F11 },
        { "F12", GDK_F12 },

        { "Menu", GDK_Menu },

        // テンキー
        { "KP_Divide", GDK_KP_Divide },     // "/"
        { "KP_Multiply", GDK_KP_Multiply }, // "*"
        { "KP_Subtract", GDK_KP_Subtract }, // "-"
        { "KP_Home", GDK_KP_Home },         // "Home(7)"
        { "KP_Up", GDK_KP_Up },             // "↑(8)"
        { "KP_Prior", GDK_KP_Prior },       // "Pg UP(9)"
        { "KP_Add", GDK_KP_Add },           // "+"
        { "KP_Left", GDK_KP_Left },         // "←(4)"
        //{ "KP_Begin", GDK_KP_Begin },       // "(5)"
        { "KP_Right", GDK_KP_Right },       // "→(6)"
        { "KP_End", GDK_KP_End },           // "End(1)"
        { "KP_Down", GDK_KP_Down },         // "↓(2)"
        { "KP_Next", GDK_KP_Next },         // "Pg Dn(3)"
        { "KP_Enter", GDK_KP_Enter },       // "Enter"
        { "KP_Insert", GDK_KP_Insert },     // "Ins(0)"
        { "KP_Delete", GDK_KP_Delete },     // "Del(.)"
    };
}

#endif
