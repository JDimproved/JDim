// ライセンス: 最新のGPL

//
// 特殊HTML文字のデコード関数
//

#ifndef _SPCHAR_DECODER_H
#define _SPCHAR_DECODER_H

namespace DBTREE
{
    int decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out );
    int decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out );
}

#endif
