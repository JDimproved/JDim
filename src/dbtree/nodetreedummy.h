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
        virtual ~NodeTreeDummy();

        // ダウンロードしない
        virtual void download_dat( const bool check_update ){}
    };
}

#endif
