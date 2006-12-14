// ライセンス: GPL2

#include "spchar_decoder.h"
#include "spchar_tbl.h"
#include "node.h"

#include "jdlib/miscutil.h"

#include <string.h>
#include <stdlib.h>


bool DBTREE::check_spchar( const char* n_in, const char* spchar )
{
    int i = 0;
    while( spchar[ i ] != '\0' ){

        if( n_in[ i ] != spchar[ i ] ) return false;
        ++i;
    }

    return true;
}



//
// 文字参照のデコード
//
// in_char : 入力文字列, in_char[ 0 ] = '&' となっていること
// out_char : 出力文字列
// n_in : 入力で使用した文字数
// n_out : 出力文字数
//
// 戻り値 : node.h で定義したノード番号
//
int DBTREE::decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    // 数字参照 &#数字;
    if( in_char[ 1 ] == '#' ) ret = decode_char_number( in_char, n_in, out_char, n_out );

    // 文字参照 -> ユニコード変換
    else{

        int i = 0;
        for(;;){

            int ucs = ucstbl[ i ].ucs;
            if( ! ucs ) break;
            if( check_spchar( in_char +1, ucstbl[ i ].str ) ){

                n_in = strlen( ucstbl[ i ].str ) +1;

                // zwnj, zwj, lrm, rlm は今のところ無視する(zwspにする)
                if( ucs >= UCS_ZWSP && ucs <= UCS_RLM ) ret = DBTREE::NODE_ZWSP;
                else n_out = MISC::ucs2toutf8( ucs, out_char );

                break;
            }
            ++i;
        }
    }

    if( !n_in ) ret = NODE_NONE;
    out_char[ n_out ] = '\0';
    return ret;
}



//
// ユニコード文字参照  &#数字;
//
// in_char[1] == "#" であること
//
int DBTREE::decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    int lng = 0;
    char str_num[ 16 ];

    n_in = n_out = 0;

    // 桁数取得(最大4桁)
    if( in_char[ 2 ] == ';' ) return NODE_NONE;
    else if( in_char[ 3 ] == ';' ) lng = 1;
    else if( in_char[ 4 ] == ';' ) lng = 2;
    else if( in_char[ 5 ] == ';' ) lng = 3;
    else if( in_char[ 6 ] == ';' ) lng = 4;
    else if( in_char[ 7 ] == ';' ) lng = 5;
    else return NODE_NONE;

    // 全て数字かどうかチェック
    for( int i = 0; i < lng; ++i ) if( in_char[ 2 + i ] < '0' || in_char[ 2 + i ] > '9' ) return NODE_NONE;

    memcpy( str_num, in_char + 2, lng );
    str_num[ lng ] = '\0';

    int num = atoi( str_num );

    switch( num ){

        //zwnj,zwj,lrm,rlm は今のところ無視(zwspにする)
        case UCS_ZWSP:
        case UCS_ZWNJ:
        case UCS_ZWJ:
        case UCS_LRM:
        case UCS_RLM:
            ret = DBTREE::NODE_ZWSP;
            break;

        default:
            n_out = MISC::ucs2toutf8( num, out_char );
            if( ! n_out ) return DBTREE::NODE_NONE;
    }

    n_in = 3 + lng;

    return ret;
}    
                                                                
