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
        char* m_buffer_for_200;  // HTTP200が来た時のdat落ち判定用

        std::string m_tmp_buffer;

        std::string m_subject_machi;

      public:

        NodeTreeMachi( const std::string& url, const std::string& date_modified );
        ~NodeTreeMachi();

      protected:

        void clear() override;
        void init_loading() override;
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        char* process_raw_lines( char* rawlines ) override;
        const char* raw2dat( char* rawlines, int& byte ) override;

        void receive_data( const char* data, size_t size ) override;
        void receive_finish() override;
    };
}

#endif
