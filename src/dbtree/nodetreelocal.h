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

        explicit NodeTreeLocal( const std::string& url );
        ~NodeTreeLocal() override;

        // ダウンロードしない
        void download_dat( const bool check_update ) override {}
    };
}

#endif
