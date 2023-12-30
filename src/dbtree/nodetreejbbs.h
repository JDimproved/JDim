// ライセンス: GPL2

//
// JBBS型ノードツリー
//

#ifndef _NODETREEJBBS_H
#define _NODETREEJBBS_H

#include "nodetreebase.h"

#include <memory>
#include <string>


namespace JDLIB
{
    class Iconv;
}


namespace DBTREE
{
    class NodeTreeJBBS : public NodeTreeBase
    {
        std::unique_ptr<JDLIB::Iconv> m_iconv;
        std::string m_decoded_lines;

      public:

        NodeTreeJBBS( const std::string& url, const std::string& date_modified );
        ~NodeTreeJBBS() override;

      protected:

        void clear() override;
        void init_loading() override;
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        const char* raw2dat( char* rawlines, int& byte ) override;
    };
}

#endif
