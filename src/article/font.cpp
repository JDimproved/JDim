// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "jdlib/miscutil.h"

#include "global.h"

unsigned char* width_of_char[ FONT_NUM ];


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
// mode   : global.h で定義されてるフォントのID
//
// 戻り値 : 文字幅(ピクセル値)
// 
int ARTICLE::get_width_of_char( const char* utfstr, int& byte, int mode )
{
    if( width_of_char[ mode ]  == NULL ){
        width_of_char[ mode ] = ( unsigned char* ) malloc( 65536 + 16 );
        memset( width_of_char[ mode ], 0, 65536 );
    }

    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return 0;

    return width_of_char[ mode ][ ucs2 ];
}




//
// 文字幅をセットする関数
//
void ARTICLE::set_width_of_char( const char* utfstr, int& byte, int mode, int width )
{    
    int ucs2 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return;

    width_of_char[ mode ][ ucs2 ] = width;    
}
