// ライセンス: GPL2

//
// BoardBaseのファクトリー
//

#ifndef _BOARDFACTORY_H
#define _BOARDFACTORY_H

#include <memory>
#include <string>


namespace DBTREE
{
    class BoardBase;

    std::unique_ptr<DBTREE::BoardBase> BoardFactory( int type, const std::string& root, const std::string& path_board,
                                                     const std::string& name, const std::string& basicauth );
}


#endif
