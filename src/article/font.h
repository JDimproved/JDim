// ライセンス: GPL2

// 文字の幅とかを記録しておくデータベース

#ifndef _FONT_H
#define _FONT_H

namespace ARTICLE
{
    void init_font();
    int utf8toucs2( const char* utfstr, int& byte );
    void get_width_of_char( const char* utfstr, int& byte, const char pre_char, int& width, int& width_wide, const int mode );
    void set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode );
}

#endif
