// ライセンス: 最新のGPL

// 文字の幅とかを記録しておくデータベース

#ifndef _FONT_H
#define _FONT_H

namespace ARTICLE
{
    void init_font();
    int utf8toucs2( const char* utfstr, int& byte );
    int get_width_of_char( const char* utfstr, int& byte, int mode );
    void set_width_of_char( const char* utfstr, int& byte, int mode, int width );
}

#endif
