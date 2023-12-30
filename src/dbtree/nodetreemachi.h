// ライセンス: GPL2

//
// Machi型ノードツリー
//

#ifndef _NODETREEMACHI_H
#define _NODETREEMACHI_H

#include "nodetreebase.h"

#include <memory>
#include <string_view>


namespace JDLIB
{
    class Iconv;
    class Regex;
}


namespace DBTREE
{
    class NodeTreeMachi : public NodeTreeBase
    {
        std::unique_ptr<JDLIB::Regex> m_regex;
        std::unique_ptr<JDLIB::Iconv> m_iconv;
        std::string m_decoded_lines;
        std::string m_buffer;
        std::string m_buffer_for_200;  // HTTP200が来た時のdat落ち判定用

        std::string m_tmp_buffer;

        std::string m_subject_machi;

      public:

        NodeTreeMachi( const std::string& url, const std::string& date_modified );
        ~NodeTreeMachi() override;

      protected:

        void clear() override;
        void init_loading() override;
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        char* process_raw_lines( std::string& rawlines ) override;
        const char* raw2dat( char* rawlines, int& byte ) override;

        void receive_data( std::string_view buf ) override;
        void receive_finish() override;
    };
}

#endif
