// ライセンス: GPL2

// キー名 <-> keysym 変換テーブル

#ifndef _KEYSYMS_H
#define _KEYSYMS_H

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

        { "Space", GDK_KEY_space },
        { "Escape", GDK_KEY_Escape },
        { "Delete", GDK_KEY_Delete },
        { "Enter", GDK_KEY_Return },

        { "Up", GDK_KEY_Up },
        { "Down", GDK_KEY_Down },
        { "Left", GDK_KEY_Left },
        { "Right", GDK_KEY_Right },
        { "Page_Up", GDK_KEY_Page_Up },
        { "Page_Down", GDK_KEY_Page_Down },
        { "Tab", GDK_KEY_Tab },
        { "Left_Tab", GDK_KEY_ISO_Left_Tab },
        { "Home", GDK_KEY_Home },
        { "End", GDK_KEY_End },
        { "BackSpace", GDK_KEY_BackSpace },

        { "F1", GDK_KEY_F1 },
        { "F2", GDK_KEY_F2 },
        { "F3", GDK_KEY_F3 },
        { "F4", GDK_KEY_F4 },
        { "F5", GDK_KEY_F5 },
        { "F6", GDK_KEY_F6 },
        { "F7", GDK_KEY_F7 },
        { "F8", GDK_KEY_F8 },
        { "F9", GDK_KEY_F9 },
        { "F10", GDK_KEY_F10 },
        { "F11", GDK_KEY_F11 },
        { "F12", GDK_KEY_F12 },

        { "Menu", GDK_KEY_Menu },

        // テンキー
        { "KP_Divide", GDK_KEY_KP_Divide },     // "/"
        { "KP_Multiply", GDK_KEY_KP_Multiply }, // "*"
        { "KP_Subtract", GDK_KEY_KP_Subtract }, // "-"
        { "KP_Home", GDK_KEY_KP_Home },         // "Home(7)"
        { "KP_Up", GDK_KEY_KP_Up },             // "↑(8)"
        { "KP_Prior", GDK_KEY_KP_Prior },       // "Pg UP(9)"
        { "KP_Add", GDK_KEY_KP_Add },           // "+"
        { "KP_Left", GDK_KEY_KP_Left },         // "←(4)"
        //{ "KP_Begin", GDK_KEY_KP_Begin },       // "(5)"
        { "KP_Right", GDK_KEY_KP_Right },       // "→(6)"
        { "KP_End", GDK_KEY_KP_End },           // "End(1)"
        { "KP_Down", GDK_KEY_KP_Down },         // "↓(2)"
        { "KP_Next", GDK_KEY_KP_Next },         // "Pg Dn(3)"
        { "KP_Enter", GDK_KEY_KP_Enter },       // "Enter"
        { "KP_Insert", GDK_KEY_KP_Insert },     // "Ins(0)"
        { "KP_Delete", GDK_KEY_KP_Delete },     // "Del(.)"
    };
}

#endif
