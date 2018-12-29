// ライセンス: GPL2

//
// ARTICLE::LayoutTree::append_html() で使用するダミーノードツリー
//

#ifndef _NODETREEDUMMY_H
#define _NODETREEDUMMY_H

#include "nodetreebase.h"

namespace DBTREE
{
    class NodeTreeDummy : public NodeTreeBase
    {
      public:

        NodeTreeDummy( const std::string& url );
        ~NodeTreeDummy();

        // ダウンロードしない
        void download_dat( const bool check_update ) override {}
    };
}

#endif
