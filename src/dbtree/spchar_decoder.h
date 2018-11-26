// ライセンス: GPL2

//
// 特殊HTML文字のデコード関数
//

#ifndef _SPCHAR_DECODER_H
#define _SPCHAR_DECODER_H

namespace DBTREE
{
    // 文字参照のデコード
    // in_char : 入力文字列, in_char[ 0 ] = '&' となっていること
    // n_in : 入力で使用した文字数が返る
    // out_char : 出力文字列
    // n_out : 出力した文字数が返る
    // only_check : チェックのみ実施 ( out_char は NULL でも可 )
    //
    // 戻り値 : node.h で定義したノード番号
    //
    int decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out, const bool only_check );
}

#endif
