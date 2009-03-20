// ライセンス: GPL2

//
// ローカルファイル用ノードツリー
//

#ifndef _NODETREELOCAL_H
#define _NODETREELOCAL_H

#include "nodetree2chcompati.h"

namespace DBTREE
{
    class NodeTreeLocal : public NodeTree2chCompati
    {
      public:

        NodeTreeLocal( const std::string& url );
        virtual ~NodeTreeLocal();

        // ダウンロードしない
        virtual void download_dat( const bool check_update ){}
    };
}

#endif
