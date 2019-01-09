// ライセンス: GPL2

// 文字の幅とかを記録しておくデータベース

#ifndef _FONT_H
#define _FONT_H

namespace ARTICLE
{
    void init_font();

    // 登録された文字の幅を返す関数
    // utfstr : 入力文字 (UTF-8)
    // byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
    // pre_char : ひとつ前の文字 ( 前の文字が全角の場合は 0 )
    // width  : 半角モードでの幅
    // width_wide : 全角モードでの幅
    // mode   : fontid.h で定義されているフォントのID
    // 戻り値 : 登録されていればtrue
    bool get_width_of_char( const char* utfstr, int& byte, const char pre_char, int& width, int& width_wide, const int mode );

    // 文字幅を登録する関数
    // width == -1 はフォント幅の取得に失敗した場合
    void set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode );
}

#endif
