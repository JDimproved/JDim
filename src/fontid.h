// フォントID

#ifndef _FONT_ID_H
#define _FONT_ID_H

enum
{
    FONT_MAIN = 0,   // スレッドビューなどの基本の物
    FONT_POPUP,      // ポップアップ

    FONT_BBS,        // スレ一覧

    FONT_BOARD,      // 板一覧

    FONT_MESSAGE,    // 書き込みビューのエディタ

    FONT_ENTRY_DEFAULT,    // Gtk::Entryのデフォルトフォント

    FONT_NUM
};

#endif
