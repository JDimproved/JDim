// ライセンス: GPL2

#include "sharedbuffer.h"

std::list< CORE::DATA_INFO > shared_infolist;


int CORE::SBUF_size()
{
    return shared_infolist.size();
}


void CORE:: SBUF_clear_info()
{
    shared_infolist.clear();
}


void CORE::SBUF_append( const DATA_INFO& info )
{
    shared_infolist.push_back( info );
}


const std::list< CORE::DATA_INFO >& CORE::SBUF_infolist()
{
    return shared_infolist;
}
