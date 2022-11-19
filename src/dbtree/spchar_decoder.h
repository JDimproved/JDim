// ライセンス: GPL2

//
// 特殊HTML文字のデコード関数
//

#ifndef _SPCHAR_DECODER_H
#define _SPCHAR_DECODER_H

#include "jdlib/span.h"


namespace DBTREE
{
    /**
     * @brief HTML 文字参照をUTF-8文字列にデコードする
     *
     * @param[in]  in_char  入力文字列, in_char[0] = '&' となっていること (not null)
     * @param[out] n_in     入力で使用した文字数が返る
     * @param[out] out_char 出力文字列 (長さ5以上)
     * @param[out] n_out    出力した文字数が返る
     * @return デコードした文字の種類( node.h で定義したノード番号 )
     */
    int decode_char( const char* in_char, int& n_in, JDLIB::span<char> out_char, int& n_out );
}

#endif
