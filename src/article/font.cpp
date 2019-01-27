// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "jdlib/miscutil.h"

#include "fontid.h"
#include "config/globalconf.h"

#include <cstdlib>
#include <cstring>


struct WIDTH_DATA
{
    // 半角モードの時の幅
    unsigned int *width;

    // 全角モードの時の幅
    unsigned int width_wide;
};


static WIDTH_DATA* width_of_char[ FONT_NUM ];
static bool strict_of_char = false;

enum
{
    UCS2_MAX = 1114111
};


//
// 初期化
//
void ARTICLE::init_font()
{
    // スレビューで文字幅の近似を厳密にするか
    strict_of_char = CONFIG::get_strict_char_width();

    for( int i = 0; i< FONT_NUM ; i++ ){

        if( width_of_char[ i ]  ){

            for( int j = 0; j < UCS2_MAX; ++j ){
                
                if( width_of_char[ i ][ j ].width ) delete width_of_char[ i ][ j ].width;
            }
            delete width_of_char[ i ];
        }

        width_of_char[ i ] = NULL;
    }
}



//
// 登録された文字の幅を返す関数
//
// utfstr : 入力文字 (UTF-8)
// byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
// pre_char : ひとつ前の文字 ( 前の文字が全角の場合は 0 )
// width  : 半角モードでの幅
// width_wide : 全角モードでの幅
// mode   : fontid.h で定義されているフォントのID
// 戻り値 : 登録されていればtrue
// 
bool ARTICLE::get_width_of_char( const char* utfstr, int& byte, const char pre_char, int& width, int& width_wide, const int mode )
{
    byte = 0;
    width = 0;
    width_wide = 0;

    if( ! width_of_char[ mode ] ){
        const int size = sizeof( WIDTH_DATA ) * UCS2_MAX;

        width_of_char[ mode ] = ( WIDTH_DATA* ) malloc( size );
        memset( width_of_char[ mode ], 0, size );
    }

    const int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( byte && ucs2 < UCS2_MAX ){

        // 全角モードの幅
        width_wide = width_of_char[ mode ][ ucs2 ].width_wide;

        // 半角モードの幅
        width = width_wide;

        // 厳密に求める場合
        if( byte == 1 && strict_of_char ){

            if( ! width_of_char[ mode ][ ucs2 ].width ){
                const int size = sizeof( unsigned int ) * 128;

                width_of_char[ mode ][ ucs2 ].width = ( unsigned int* ) malloc( size );
                memset( width_of_char[ mode ][ ucs2 ].width, 0, size );
            }

            const int pre_char_num = ( const int ) pre_char;
            if( pre_char_num < 128 ) width = width_of_char[ mode ][ ucs2 ].width[ pre_char_num ];
        }
    }

    if( width && width_wide ) return true;
    else if( width == -1 ){ // フォント幅の取得に失敗した場合
        width = width_wide = 0;
        return true;
    }

    return false;
}



//
// 文字幅を登録する関数
//
// width == -1 はフォント幅の取得に失敗した場合
//
void ARTICLE::set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode )
{    
    const int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return;
    if( ucs2 >= UCS2_MAX ) return;

    // 半角モードの幅を厳密に求める場合
    if( byte == 1 && strict_of_char ){

        const int pre_char_num = ( const int ) pre_char;
        if( pre_char_num < 128 ) width_of_char[ mode ][ ucs2 ].width[ pre_char_num ] = width;
    }

    // 全角モードの幅
    width_of_char[ mode ][ ucs2 ].width_wide = width_wide;
}
