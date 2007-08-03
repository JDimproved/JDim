// ライセンス: GPL2
//
// ユニコード(ucs2) <-> 文字参照変換テーブル
//

#ifndef _SPCHAR_TBL_H
#define _SPCHAR_TBL_H

struct UCSTBL
{
    int ucs;
    char str[ 256 ];
};


UCSTBL ucstbl[] = {

    { 34, "quot;" },
    { 38, "amp;" },
    { 60, "lt;" },
    { 62, "gt;" },

    { 32, "nbsp;" },
//    { 160, "nbsp;" }, // 正しくはこちら
    { 164, "curren;" },
    { 165, "yen;" },
    { 169, "copy;" },
    { 174, "reg;" },
    { 191, "iquest;" },

    { 8482, "trade;" },
    { 8194, "ensp;" },
    { 8195, "emsp;" },
    { 8201, "thinsp;" },
    { 8202, "hairsp;" },
    { 8203, "zwsp;" },
    { 8204, "zwnj;" },
    { 8205, "zwj;" },
    { 8206, "lrm;" },
    { 8207, "rlm;" },
    { 8212, "mdash;" },
    { 8776, "asymp;" },

    { 9824, "spades;" },
    { 9827, "clubs;" },
    { 9829, "hearts;" },
    { 9830, "diams;" },

    { 0, "" } // 終端
};


enum
{
    UCS_ZWSP    = 8203,
    UCS_ZWNJ    = 8204,
    UCS_ZWJ     = 8205,
    UCS_LRM     = 8206,
    UCS_RLM     = 8207,
};


#endif
