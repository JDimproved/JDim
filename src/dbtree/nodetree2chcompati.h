// ライセンス: GPL2

//
// 2ch互換型ノードツリー
//

#ifndef _NODETREE2CHCOMPATI_H
#define _NODETREE2CHCOMPATI_H

#include "nodetreebase.h"

namespace JDLIB
{
    class Iconv;
}

namespace DBTREE
{
    class NodeTree2chCompati : public NodeTreeBase
    {
        JDLIB::Iconv* m_iconv;

      public:

        NodeTree2chCompati( const std::string& url, const std::string& date_modified );
        virtual ~NodeTree2chCompati();

      protected:

        virtual void clear();
        virtual void init_loading();
        virtual char* process_raw_lines( char* rawlines );
        virtual const char* raw2dat( char* rawlines, int& byte );

        virtual void create_loaderdata( JDLIB::LOADERDATA& data );
    };
}

#endif
