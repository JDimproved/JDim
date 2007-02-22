// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "jdlib/miscutil.h"

#include "fontid.h"
#include "config/globalconf.h"


struct WIDTH_DATA
{
    // 半角モードの時の幅
    unsigned int *width;

    // 全角モードの時の幅
    unsigned int width_wide;
};


WIDTH_DATA* width_of_char[ FONT_NUM ];

#define UCS2_MAX 65536


//
// 初期化
//
void ARTICLE::init_font()
{
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
// 文字の幅を返す関数
//
// utfstr : 入力文字 (UTF-8)
// byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 を入れて返す
// pre_char : ひとつ前の文字 ( 前の文字が全角の場合は 0 )
// width  : 半角モードでの幅
// width_wide : 全角モードでの幅
// mode   : global.h で定義されてるフォントのID
//
// 
void ARTICLE::get_width_of_char( const char* utfstr, int& byte, const char pre_char, int& width, int& width_wide, const int mode )
{
    byte = 0;
    width = 0;
    width_wide = 0;

    if( width_of_char[ mode ]  == NULL ){
        width_of_char[ mode ] = ( WIDTH_DATA* ) malloc( sizeof( WIDTH_DATA ) * ( UCS2_MAX + 16 ) );
        memset( width_of_char[ mode ], 0, sizeof( WIDTH_DATA ) * UCS2_MAX );
    }

    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( byte ){

        // 全角モードの幅
        width_wide = width_of_char[ mode ][ ucs2 ].width_wide;

        // 半角モードの幅
        width = width_wide;

        // 厳密に求める場合
        if( byte == 1 && CONFIG::get_strict_char_width() ){  

            if( ! width_of_char[ mode ][ ucs2 ].width ){

                const int size = 128;
                width_of_char[ mode ][ ucs2 ].width = ( unsigned int* ) malloc( sizeof( unsigned int ) * ( size + 16 ) );
                memset( width_of_char[ mode ][ ucs2 ].width, 0, sizeof( unsigned int ) * size );
            }

            const int pre_char_num = ( const int ) pre_char;
            if( pre_char_num < 128 ) width = width_of_char[ mode ][ ucs2 ].width[ pre_char_num ];
        }
    }
}



//
// 文字幅をセットする関数
//
void ARTICLE::set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode )
{    
    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return;

    // 半角モードの幅を厳密に求める場合
    if( byte == 1 && CONFIG::get_strict_char_width() ){  

        const int pre_char_num = ( const int ) pre_char;
        if( pre_char_num < 128 ) width_of_char[ mode ][ ucs2 ].width[ pre_char_num ] = width;
    }

    // 全角モードの幅
    width_of_char[ mode ][ ucs2 ].width_wide = width_wide;
}
