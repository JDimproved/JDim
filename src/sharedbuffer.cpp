// ライセンス: GPL2

#include "sharedbuffer.h"

using namespace CORE;

CORE::DATA_INFO_LIST shared_infolist;

int CORE::SBUF_size()
{
    return shared_infolist.size();
}

void CORE:: SBUF_clear_info()
{
    shared_infolist.clear();
}

void CORE::SBUF_set_list( const CORE::DATA_INFO_LIST& list_info )
{
    shared_infolist = list_info;
}

CORE::DATA_INFO_LIST& CORE::SBUF_list_info()
{
    return shared_infolist;
}
