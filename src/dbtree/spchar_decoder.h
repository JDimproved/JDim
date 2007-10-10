// ライセンス: GPL2

//
// 特殊HTML文字のデコード関数
//

#ifndef _SPCHAR_DECODER_H
#define _SPCHAR_DECODER_H

namespace DBTREE
{
    // 文字参照のデコード
    int decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out );
}

#endif
