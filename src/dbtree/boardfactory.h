// ライセンス: 最新のGPL

//
// BoardBaseのファクトリー
//

#ifndef _BOARDFACTORY_H
#define _BOARDFACTORY_H

#include <string>

namespace DBTREE
{
    class BoardBase;

    DBTREE::BoardBase* BoardFactory( int type, const std::string& root, const std::string& path_board, const std::string& name );
}


#endif
