// タイプ

#ifndef _TYPE_H
#define _TYPE_H

enum 
{
    // 板のタイプ
    TYPE_BOARD_2CH = 0,
    TYPE_BOARD_2CH_COMPATI,  // 2ch 互換
    TYPE_BOARD_JBBS,         // したらば
    TYPE_BOARD_MACHI,        // まち
    TYPE_BOARD_UNKNOWN,

    // その他一般的なデータタイプ
    TYPE_BOARD,
    TYPE_THREAD,
    TYPE_THREAD_UPDATE,
    TYPE_THREAD_OLD,
    TYPE_IMAGE,
    TYPE_DIR,
    TYPE_DIR_END, // お気に入りの追加の時にサブディレクトリの終了の意味で使う
    TYPE_COMMENT,
    TYPE_LINK,
    TYPE_AA,
    TYPE_HISTITEM,
    TYPE_USRCMD,
    TYPE_SEPARATOR,

    TYPE_UNKNOWN
};

#endif
