// ライセンス: GPL2

#include "boardfactory.h"
#include "board2ch.h"
#include "board2chcompati.h"
#include "boardlocal.h"
#include "boardjbbs.h"
#include "boardmachi.h"

#include "type.h"

DBTREE::BoardBase* DBTREE::BoardFactory( int type, const std::string& root, const std::string& path_board, const std::string& name,
                                         const std::string& basicauth )
{
    switch( type )
    {
        case TYPE_BOARD_2CH: return new DBTREE::Board2ch( root, path_board, name );

        case TYPE_BOARD_2CH_COMPATI: return new DBTREE::Board2chCompati( root, path_board, name, basicauth );

        case TYPE_BOARD_LOCAL: return new DBTREE::BoardLocal( root, path_board, name );

        case TYPE_BOARD_JBBS:return  new DBTREE::BoardJBBS( root, path_board, name );

        case TYPE_BOARD_MACHI:return  new DBTREE::BoardMachi( root, path_board, name );

        default: return NULL;
    }
}
