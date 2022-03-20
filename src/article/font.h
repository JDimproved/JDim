// ライセンス: GPL2

// 文字の幅とかを記録しておくデータベース

#ifndef _FONT_H
#define _FONT_H

namespace ARTICLE
{
    void init_font();

    // 登録された文字の幅を返す関数
    // code   : 入力文字 (コードポイント)
    // pre_char : ひとつ前の文字 ( 前の文字が全角の場合は 0 )
    // width  : 半角モードでの幅
    // width_wide : 全角モードでの幅
    // mode   : fontid.h で定義されているフォントのID
    // 戻り値 : 登録されていればtrue
    bool get_width_of_char( const char32_t code, const char pre_char, int& width, int& width_wide, const int mode );

    // 文字幅を登録する関数
    // width == -1 はフォント幅の取得に失敗した場合
    void set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode );
}

#endif
