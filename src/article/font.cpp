// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "jdlib/miscutil.h"

#include "global.h"


struct WIDTH_DATA
{
    unsigned int width;
    unsigned int width_wide;
};


WIDTH_DATA* width_of_char[ FONT_NUM ];


//
// 初期化
//
void ARTICLE::init_font()
{
    for( int i = 0; i< FONT_NUM ; i++ ){
        if( width_of_char[ i ]  ) delete width_of_char[ i ];
        width_of_char[ i ] = NULL;
    }
}



//
// 文字の幅を返す関数
//
// utfstr : 入力文字 (UTF-8)
// byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 を入れて返す
// width  : 半角モードでの幅
// width_wide : 全角モードでの幅
// mode   : global.h で定義されてるフォントのID
//
// 
void ARTICLE::get_width_of_char( const char* utfstr, int& byte, int& width, int& width_wide, const int mode )
{
    byte = 0;
    width = 0;
    width_wide = 0;

    if( width_of_char[ mode ]  == NULL ){
        width_of_char[ mode ] = ( WIDTH_DATA* ) malloc( sizeof( WIDTH_DATA ) * ( 65536 + 16 ) );
        memset( width_of_char[ mode ], 0, sizeof( WIDTH_DATA ) * 65536 );
    }

    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( byte ){

        width = width_of_char[ mode ][ ucs2 ].width; 
        width_wide = width_of_char[ mode ][ ucs2 ].width_wide;
    }
}




//
// 文字幅をセットする関数
//
void ARTICLE::set_width_of_char( const char* utfstr, int& byte, const int width, const int width_wide, const int mode )
{    
    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return;

    width_of_char[ mode ][ ucs2 ].width = width;    
    width_of_char[ mode ][ ucs2 ].width_wide = width_wide;
}
