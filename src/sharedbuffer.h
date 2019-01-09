// ライセンス: GPL2

//
// 共有バッファ
//
// クラス間で情報をやり取りするときに使う
//

#ifndef _SHAREDBUFFER_H
#define _SHAREDBUFFER_H

#include "type.h"
#include "data_info.h"

namespace CORE
{
    int SBUF_size();
    void SBUF_clear_info();
    void SBUF_set_list( const CORE::DATA_INFO_LIST& list_info );
    CORE::DATA_INFO_LIST& SBUF_list_info();
}

#endif
