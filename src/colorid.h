// カラーID

#ifndef _COLOR_ID_H
#define _COLOR_ID_H

enum
{
    COLOR_DEFAULT = 0,
    COLOR_CHAR,             // スレビューなど基本の文字
    COLOR_CHAR_NAME,        // 名前欄
    COLOR_CHAR_NAME_B,      // トリップや fusianasan 等、<b>が含まれている名前欄
    COLOR_CHAR_AGE,         // 非sageのメール欄
    COLOR_CHAR_SELECTION,   // 選択範囲の文字
    COLOR_CHAR_HIGHLIGHT,   // ハイライトの文字
    COLOR_CHAR_BOOKMARK,    // ブックマーク
    COLOR_CHAR_LINK,        // リンク
    COLOR_CHAR_LINK_PUR,    // レス番号やIDなどの複数現れたリンク
    COLOR_CHAR_LINK_RED,    // レス番号やIDなどの多数現れたリンク

    COLOR_IMG_NOCACHE, // 画像のリンク(キャッシュ無)
    COLOR_IMG_CACHED,  // 画像のリンク(キャッシュ有)
    COLOR_IMG_LOADING, // 画像のリンク(ロード中)
    COLOR_IMG_ERR,     // 画像のリンク(エラー)

    COLOR_BACK,             // スレビューなど基本の背景
    COLOR_BACK_POPUP,       // ポップアップの背景
    COLOR_BACK_BBS,         // 板一覧の背景
    COLOR_BACK_BOARD,       // スレ一覧の背景
    COLOR_BACK_SELECTION,   // 選択範囲
    COLOR_BACK_HIGHLIGHT,   // ハイライト文字

    COLOR_SEPARATOR_NEW,    // 新着セパレータ

    COLOR_NUM
};

#endif
