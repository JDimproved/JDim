// ライセンス: GPL2

#include "boardfactory.h"
#include "board2ch.h"
#include "board2chcompati.h"
#include "boardlocal.h"
#include "boardjbbs.h"
#include "boardmachi.h"

#include "type.h"


std::unique_ptr<DBTREE::BoardBase> DBTREE::BoardFactory( int type, const std::string& root,
                                                         const std::string& path_board, const std::string& name,
                                                         const std::string& basicauth )
{
    switch( type )
    {
        case TYPE_BOARD_2CH: return std::make_unique<Board2ch>( root, path_board, name );

        case TYPE_BOARD_2CH_COMPATI: return std::make_unique<Board2chCompati>( root, path_board, name, basicauth );

        case TYPE_BOARD_LOCAL: return std::make_unique<BoardLocal>( root, path_board, name );

        case TYPE_BOARD_JBBS: return std::make_unique<BoardJBBS>( root, path_board, name );

        case TYPE_BOARD_MACHI: return std::make_unique<BoardMachi>( root, path_board, name );

        default: return nullptr;
    }
}
