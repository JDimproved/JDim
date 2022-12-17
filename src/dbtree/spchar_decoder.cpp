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


/**
 * @brief HTMLの数値文字参照 `&#数字;` をUTF-8文字列にデコードする
 *
 * @param[in]  in_char  入力文字列、in_char[1] == '#' であること (not null)
 * @param[out] n_in     入力で使用した文字数が返る
 * @param[out] out_char 出力文字列 (長さ5以上)
 * @param[out] n_out    出力した文字数が返る
 * @return デコードした文字の種類( node.h で定義したノード番号 )
 */
int decode_char_number( const char* in_char, int& n_in, JDLIB::span<char> out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    int offset;
    const int lng = MISC::spchar_number_ln( in_char, offset );
    if( lng == -1 ) return DBTREE::NODE_NONE;

    const char32_t uch = MISC::decode_spchar_number( in_char, offset, lng );

    switch( uch ){

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
            n_out = MISC::utf32toutf8( uch, out_char.data() );
            if( ! n_out ) return DBTREE::NODE_NONE;
    }

    n_in = offset + lng;
    if( in_char[n_in] == ';' ) n_in++; // 数値文字参照の終端「;」の場合は1文字削除

    out_char[ n_out ] = '\0';

    return ret;
}


/**
 * @brief HTML 文字参照をUTF-8文字列にデコードする
 *
 * @param[in]  in_char  入力文字列, in_char[0] = '&' となっていること (not null)
 * @param[out] n_in     入力で使用した文字数が返る
 * @param[out] out_char 出力文字列 (長さ7以上)
 * @param[out] n_out    出力した文字数が返る
 * @return デコードした文字の種類( node.h で定義したノード番号 )
 */
int DBTREE::decode_char( const char* in_char, int& n_in, JDLIB::span<char> out_char, int& n_out )
{
    assert( out_char.size() >= 7 );

    // 1文字目が&以外の場合は出力しない
    if( in_char[ 0 ] != '&' ){
        n_in = n_out = 0;
        out_char[ n_out ] = '\0';

        return DBTREE::NODE_NONE;
    }

    // 数字参照 &#数字;
    if( in_char[ 1 ] == '#' ) return decode_char_number( in_char, n_in, out_char, n_out );

    // 文字実体参照 &名前;
    return DBTREE::decode_char_name( in_char, n_in, out_char, n_out );
}


/**
 * @brief HTML 文字実体参照をUTF-8文字列にデコードする
 *
 * @param[in]  in_char  入力文字列, in_char[0] = '&' となっていること (not null)
 * @param[out] n_in     入力で使用した文字数が返る
 * @param[out] out_char 出力文字列 (長さ7以上)
 * @param[out] n_out    出力した文字数が返る
 * @return デコードした文字の種類( node.h で定義したノード番号 )
 */
int DBTREE::decode_char_name( const char* in_char, int& n_in, JDLIB::span<char> out_char, int& n_out )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    const char ch = in_char[ 1 ];
    JDLIB::span<const UCSTBL> tbl;

    if( ch >= 'a' && ch <= 'z' ) tbl = ucstbl_lower[ ch - 'a' ];
    else if( ch >= 'A' && ch <= 'Z' ) tbl = ucstbl_upper[ ch - 'A' ];
    else return DBTREE::NODE_NONE;

    for( const auto& [entity, utf8] : tbl ) {
        const int order = std::strncmp( in_char + 1, entity.data(), entity.size() );
        if( order == 0 ) {

            n_in = static_cast<int>( entity.size() ) + 1; // 先頭の '&' の分を+1する

            // U+200B, U+200C(zwnj), U+200D(zwj), U+200E(lrm), U+200F(rlm)
            // は今のところ空文字列にする(zwsp扱いにする)
            if( utf8[0] == '\xE2' && utf8[1] == '\x80' && '\x8B' <= utf8[2] && utf8[2] <= '\x8F' ) {
                ret = DBTREE::NODE_ZWSP;
            }
            else {
                n_out = static_cast<int>( utf8.size() );
                utf8.copy( out_char.data(), utf8.size() );
            }

            break;
        }
        // NOTE: UCSTBL の配列は entity の辞書順になっている
        // in_char+1 が entity より小さくなったら検索を終了していい
        else if( order < 0 ) break;
    }

    if( !n_in ) ret = DBTREE::NODE_NONE;
    out_char[ n_out ] = '\0';

    return ret;
}
