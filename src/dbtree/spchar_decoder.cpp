// ライセンス: 最新のGPL

#include "spchar_decoder.h"
#include "node.h"

#include "jdlib/miscutil.h"

#include <string.h>
#include <stdlib.h>


//
// &〜みたいな特殊文字をデコードする
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

    // gt
    if( in_char[ 1 ] == 'g' && in_char[ 2 ] == 't' && in_char[ 3 ] == ';' ){
        n_in = 4;
        n_out = 1;
        out_char[ 0 ] = '>';
    }

    // lt
    else if( in_char[ 1 ] == 'l' && in_char[ 2 ] == 't' && in_char[ 3 ] == ';' ){
        n_in = 4; 
        n_out = 1;
        out_char[ 0 ] = '<';
    }
    
    // amp
    else if( in_char[ 1 ] == 'a' && in_char[ 2 ] == 'm' && in_char[ 3 ] == 'p' && in_char[ 4 ] == ';' ){
        n_in = 5; 
        n_out = 1;
        out_char[ 0 ] = '&';
    }

    // quot
    else if( in_char[ 1 ] == 'q' && in_char[ 2 ] == 'u' && in_char[ 3 ] == 'o' && in_char[ 4 ] == 't' && in_char[ 5 ] == ';' ){
        n_in = 6; 
        n_out = 1;
        out_char[ 0 ] = '"';
    }

    // hearts
    else if( in_char[ 1 ] == 'h' && in_char[ 2 ] == 'e' && in_char[ 3 ] == 'a' && in_char[ 4 ] == 'r'
        && in_char[ 5 ] == 't' && in_char[ 6 ] == 's' && in_char[ 7 ] == ';' ){
        n_in = 8;
        n_out = MISC::ucs2utf8( 9829, out_char );
    }

    // スペース

    // nbsp
    else if( in_char[ 1 ] == 'n' && in_char[ 2 ] == 'b'
             && in_char[ 3 ] == 's' && in_char[ 4 ] == 'p' && in_char[ 5 ] == ';' ){
        n_in = 6; 
        n_out = 1;
        out_char[ 0 ] = ' ';
    }

    // zwsp
    else if( in_char[ 1 ] == 'z' && in_char[ 2 ] == 'w' && in_char[ 3 ] == 's' && in_char[ 4 ] == 'p'
             && in_char[ 5 ] == ';' ){
        n_in = 6;
        n_out = 0;
        ret = DBTREE::NODE_ZWSP;
    }

    // thinsp
    else if( in_char[ 1 ] == 't' && in_char[ 2 ] == 'h' && in_char[ 3 ] == 'i' && in_char[ 4 ] == 'n'
             && in_char[ 5 ] == 's' && in_char[ 6 ] == 'p' && in_char[ 7 ] == ';' ){
        n_in = 8;
        n_out = 0;
        ret = DBTREE::NODE_THINSP;
    }

    // ensp
    else if( in_char[ 1 ] == 'e' && in_char[ 2 ] == 'n' && in_char[ 3 ] == 's' && in_char[ 4 ] == 'p'
             && in_char[ 5 ] == ';' ){
        n_in = 6;
        n_out = 0;
        ret = DBTREE::NODE_ENSP;
    }

    // emsp
    else if( in_char[ 1 ] == 'e' && in_char[ 2 ] == 'm' && in_char[ 3 ] == 's' && in_char[ 4 ] == 'p'
             && in_char[ 5 ] == ';' ){
        n_in = 6;
        n_out = 0;
        ret = DBTREE::NODE_EMSP;
    }

    // zwnj, zwj, lrm, rlm は今のところ無視
    else if( in_char[ 1 ] == 'z' && in_char[ 2 ] == 'w' && in_char[ 3 ] == 'n' && in_char[ 4 ] == 'j'
             && in_char[ 5 ] == ';' ){
         n_in = 6; 
         n_out = 0;
         ret = DBTREE::NODE_ZWSP;
    }    
    else if( in_char[ 1 ] == 'z' && in_char[ 2 ] == 'w' && in_char[ 3 ] == 'j' && in_char[ 4 ] == ';' ){
         n_in = 5; 
         n_out = 0;
         ret = DBTREE::NODE_ZWSP;
    }    
    else if( in_char[ 1 ] == 'l' && in_char[ 2 ] == 'r' && in_char[ 3 ] == 'm' && in_char[ 4 ] == ';' ){
         n_in = 5; 
         n_out = 0;
         ret = DBTREE::NODE_ZWSP;
    }    
    else if( in_char[ 1 ] == 'r' && in_char[ 2 ] == 'l' && in_char[ 3 ] == 'm' && in_char[ 4 ] == ';' ){
         n_in = 5; 
         n_out = 0;
         ret = DBTREE::NODE_ZWSP;
    }    

    // 数字参照 &#数字;
    else if( in_char[ 1 ] == '#' ) ret = decode_char_number( in_char, n_in, out_char, n_out );

    else ret = NODE_NONE;

    out_char[ n_out ] = '\0';
    return ret;
}



//
// 数字参照  &#数字;
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

        case 8194:
            ret = DBTREE::NODE_ENSP;
            break;

        case 8195:
            ret = DBTREE::NODE_EMSP;
            break;

        case 8201:
            ret = DBTREE::NODE_THINSP;
            break;

        case 8202:
            ret = DBTREE::NODE_HAIRSP;
            break;

        case 8203:
            ret = DBTREE::NODE_ZWSP;
            break;

            //zwnj,zwj,lrm,rlm は今のところ無視
        case 8204:
        case 8205:
        case 8206:
        case 8207:
            ret = DBTREE::NODE_ZWSP;
            break;

        default:
            n_out = MISC::ucs2utf8( num, out_char );
            if( ! n_out ) return NODE_NONE;
    }

    n_in = 3 + lng;

    return ret;
}    
                                                                
