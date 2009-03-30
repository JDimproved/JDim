// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "spchar_decoder.h"
#include "spchar_tbl.h"
#include "node.h"

#include "jdlib/miscutil.h"

#include <string.h>
#include <stdlib.h>


bool check_spchar( const char* n_in, const char* spchar )
{
    int i = 0;
    while( spchar[ i ] != '\0' ){

        if( n_in[ i ] != spchar[ i ] ) return false;
        ++i;
    }

    return true;
}


//
// ユニコード文字参照  &#数字;
//
// in_char: 入力文字列、in_char[1] == "#" であること
// n_in : 入力で使用した文字数が返る
// out_char : 出力文字列
// n_out : 出力した文字数が返る
//
// 戻り値 : node.h で定義したノード番号
//
const int decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    int lng = 0;
    char str_num[ 16 ];
    int offset = 2;

    n_in = n_out = 0;

    // offset == 2 なら 10 進数、3 なら16進数
    if( in_char[ offset ] == 'x' || in_char[ offset ] == 'X' ) offset += 1;

    // 桁数取得(最大4桁)
    if( in_char[ offset ] == ';' ) return DBTREE::NODE_NONE;
    else if( in_char[ offset +1 ] == ';' ) lng = 1;
    else if( in_char[ offset +2 ] == ';' ) lng = 2;
    else if( in_char[ offset +3 ] == ';' ) lng = 3;
    else if( in_char[ offset +4 ] == ';' ) lng = 4;
    else if( in_char[ offset +5 ] == ';' ) lng = 5;
    else return DBTREE::NODE_NONE;

    // デコード可能かチェック

    // 10 進数
    if( offset == 2 ){

        for( int i = 0; i < lng; ++i ){
            if( in_char[ offset + i ] < '0' || in_char[ offset + i ] > '9' ) return DBTREE::NODE_NONE;
        }
    }

    // 16 進数
    else{

        for( int i = 0; i < lng; ++i ){
            if(
                ! (
                    ( in_char[ offset + i ] >= '0' && in_char[ offset + i ] <= '9' )
                    || ( in_char[ offset + i ] >= 'a' && in_char[ offset + i ] <= 'f' )
                    || ( in_char[ offset + i ] >= 'A' && in_char[ offset + i ] <= 'F' )
                    )
                ) return DBTREE::NODE_NONE;
        }
    }

    memcpy( str_num, in_char + offset, lng );
    str_num[ lng ] = '\0';

#ifdef _DEBUG
    std::cout << "decode_char_number offset = " << offset
              << " lng = " << lng
              << " str = " << str_num << std::endl;
#endif

    int num = 0;

    if( offset == 2 ) num = atoi( str_num );
    else num = strtol( str_num, NULL, 16 );

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

    n_in = offset + lng + 1;

    return ret;
}    


//
// 文字参照のデコード
//
// in_char : 入力文字列, in_char[ 0 ] = '&' となっていること
// n_in : 入力で使用した文字数が返る
// out_char : 出力文字列
// n_out : 出力した文字数が返る
//
// 戻り値 : node.h で定義したノード番号
//
const int DBTREE::decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out )
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
