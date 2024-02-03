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
        enum class Mode;

        std::unique_ptr<JDLIB::Iconv> m_iconv;
        Mode m_mode;

      public:

        NodeTree2chCompati( const std::string& url, const std::string& date_modified );
        ~NodeTree2chCompati() override;

      protected:

        void clear() override;
        void init_loading() override;
        const char* raw2dat( char* rawlines, int& byte ) override;

        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        void receive_finish() override;
    };
}

#endif
