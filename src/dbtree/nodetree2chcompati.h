// ライセンス: GPL2

//
// 2ch互換型ノードツリー
//

#ifndef _NODETREE2CHCOMPATI_H
#define _NODETREE2CHCOMPATI_H

#include "nodetreebase.h"

#include <memory>


namespace JDLIB
{
    class Iconv;
}

namespace DBTREE
{
    class NodeTree2chCompati : public NodeTreeBase
    {
        std::unique_ptr<JDLIB::Iconv> m_iconv;

      public:

        NodeTree2chCompati( const std::string& url, const std::string& date_modified );
        ~NodeTree2chCompati();

      protected:

        void clear() override;
        void init_loading() override;
        const char* raw2dat( char* rawlines, int& byte ) override;

        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
    };
}

#endif
