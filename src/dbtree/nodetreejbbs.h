// ライセンス: GPL2

//
// JBBS型ノードツリー
//

#ifndef _NODETREEJBBS_H
#define _NODETREEJBBS_H

#include "nodetreebase.h"

namespace JDLIB
{
    class Iconv;
}


namespace DBTREE
{
    class NodeTreeJBBS : public NodeTreeBase
    {
        JDLIB::Iconv* m_iconv;
        char* m_decoded_lines;
        
      public:

        NodeTreeJBBS( const std::string& url, const std::string& date_modified );
        ~NodeTreeJBBS();

      protected:

        void clear() override;
        void init_loading() override;
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        const char* raw2dat( char* rawlines, int& byte ) override;
    };
}

#endif
