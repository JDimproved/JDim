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
// only_check : チェックのみ実施 ( out_char は NULL でも可 )
//
// 戻り値 : node.h で定義したノード番号
//
int decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out, const bool only_check )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    int offset;
    const int lng = MISC::spchar_number_ln( in_char, offset );
    if( lng == -1 ) return DBTREE::NODE_NONE;

    if( only_check ) return ret;

    const int num = MISC::decode_spchar_number( in_char, offset, lng );

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

    n_in = offset + lng;
    if( in_char[n_in] == ';' ) n_in++; // 数値文字参照の終端「;」の場合は1文字削除
    
    if( out_char ) out_char[ n_out ] = '\0';

    return ret;
}    


//
// 文字参照のデコード
//
// in_char : 入力文字列, in_char[ 0 ] = '&' となっていること
// n_in : 入力で使用した文字数が返る
// out_char : 出力文字列
// n_out : 出力した文字数が返る
// only_check : チェックのみ実施 ( out_char は NULL でも可 )
//
// 戻り値 : node.h で定義したノード番号
//
int DBTREE::decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out, const bool only_check )
{
    // 1文字目が&以外の場合は出力しない
    if( in_char[ 0 ] != '&' ){
        n_in = n_out = 0;
        if( out_char ) out_char[ n_out ] = '\0';

        return DBTREE::NODE_NONE;
    }

    // 数字参照 &#数字;
    if( in_char[ 1 ] == '#' ) return decode_char_number( in_char, n_in, out_char, n_out, only_check );

    // 文字参照 -> ユニコード変換
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    int i = 0;
    for(;;){

        const int ucs = ucstbl[ i ].ucs;
        if( ! ucs ) break;
        if( in_char[ 1 ] == ucstbl[ i ].str[ 0 ] ){

            if( check_spchar( in_char +1, ucstbl[ i ].str ) ){

                if( only_check ) return ret;

                n_in = strlen( ucstbl[ i ].str ) +1;

                // zwnj, zwj, lrm, rlm は今のところ無視する(zwspにする)
                if( ucs >= UCS_ZWSP && ucs <= UCS_RLM ) ret = DBTREE::NODE_ZWSP;
                else n_out = MISC::ucs2toutf8( ucs, out_char );

                break;
            }
        }

        ++i;
    }

    if( !n_in ) ret = DBTREE::NODE_NONE;
    if( out_char ) out_char[ n_out ] = '\0';

    return ret;
}
