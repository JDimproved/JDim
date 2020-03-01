// フォントID

#ifndef _FONT_ID_H
#define _FONT_ID_H

enum
{
    FONT_MAIN = 0,   // スレッドビューなどの基本の物
    FONT_MAIL,       // メールとか日付とかの部分
    FONT_POPUP,      // ポップアップ
    FONT_AA,         // AA(スレビュー)

    FONT_BBS,        // スレ一覧

    FONT_BOARD,      // 板一覧

    FONT_MESSAGE,    // 書き込みビューのエディタ

    FONT_ENTRY_DEFAULT,    // Gtk::Entryのデフォルトフォント

    FONT_NUM,

    FONT_EMPTY,      // フォントID未設定
    FONT_DEFAULT,    // ビューの標準フォントを使用
};

#endif
