// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "spchar_decoder.h"
#include "spchar_tbl.h"
#include "node.h"

#include "jdlib/misccharcode.h"
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
int decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    int offset;
    const int lng = MISC::spchar_number_ln( in_char, offset );
    if( lng == -1 ) return DBTREE::NODE_NONE;

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

        // U+2028 LINE SEPARATOR を描画処理に渡すと改行が乱れるため空白に置き換える (webブラウザと同じ挙動)
        case CP_LINE_SEPARATOR:
            out_char[0] = ' ';
            n_out = 1;
            break;

        default:
            n_out = MISC::utf32toutf8( num, out_char );
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
//
// 戻り値 : node.h で定義したノード番号
//
int DBTREE::decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out )
{
    // 1文字目が&以外の場合は出力しない
    if( in_char[ 0 ] != '&' ){
        n_in = n_out = 0;
        if( out_char ) out_char[ n_out ] = '\0';

        return DBTREE::NODE_NONE;
    }

    // 数字参照 &#数字;
    if( in_char[ 1 ] == '#' ) return decode_char_number( in_char, n_in, out_char, n_out );

    // 文字参照 -> ユニコード変換
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    for( const UCSTBL& t : ucstbl ) {

        const int ucs = t.ucs;
        if( ! ucs ) break;

        if( in_char[1] == t.str[0]
                && check_spchar( in_char + 1, t.str ) ) {

            n_in = std::strlen( t.str ) + 1;

            // zwnj, zwj, lrm, rlm は今のところ無視する(zwspにする)
            if( ucs >= UCS_ZWSP && ucs <= UCS_RLM ) ret = DBTREE::NODE_ZWSP;
            else n_out = MISC::utf32toutf8( ucs, out_char );

            break;
        }
    }

    if( !n_in ) ret = DBTREE::NODE_NONE;
    if( out_char ) out_char[ n_out ] = '\0';

    return ret;
}
