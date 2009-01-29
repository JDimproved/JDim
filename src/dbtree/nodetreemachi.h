// ライセンス: GPL2

//
// Machi型ノードツリー
//

#ifndef _NODETREEMACHI_H
#define _NODETREEMACHI_H

#include "nodetreebase.h"

namespace JDLIB
{
    class Iconv;
    class Regex;
}


namespace DBTREE
{
    class NodeTreeMachi : public NodeTreeBase
    {
        JDLIB::Regex* m_regex;
        JDLIB::Iconv* m_iconv;
        char* m_decoded_lines;
        char* m_buffer;

        std::string m_tmp_buffer;

      public:

        NodeTreeMachi( const std::string& url, const std::string& date_modified );
        ~NodeTreeMachi();

      protected:

        virtual void clear();
        virtual void init_loading();
        virtual void create_loaderdata( JDLIB::LOADERDATA& data );
        virtual char* process_raw_lines( char* rawlines );
        virtual const char* raw2dat( char* rawlines, int& byte );
    };
}

#endif
