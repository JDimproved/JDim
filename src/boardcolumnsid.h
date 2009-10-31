// スレ一覧の列IDと並び替えモード
#ifndef _BOARDCOLUMNS_ID_H
#define _BOARDCOLUMNS_ID_H

// 列ID
enum
{
    COL_MARK = 0,
    COL_ID,
    COL_BOARD,
    COL_SUBJECT,
    COL_RES,
    COL_STR_LOAD,
    COL_STR_NEW,
    COL_SINCE,
    COL_WRITE,
    COL_SPEED,

    COL_VISIBLE_END,
            
    // 以下は不可視
    COL_MARK_VAL = COL_VISIBLE_END,
    COL_DRAWBG,
    COL_NEW,
    COL_WRITE_T,
    COL_ARTICLE,
            
    COL_NUM_COL
};

// 並び替えのモード
enum
{
    SORTMODE_ASCEND = 0,
    SORTMODE_DESCEND,

    SORTMODE_MARK1, // 通常
    SORTMODE_MARK2, // 新着をキャッシュの上に。後は通常
    SORTMODE_MARK3,  // 新着を一番上に。後は通常 
    SORTMODE_MARK4,  // 反転

    SORTMODE_NUM
};

#endif
