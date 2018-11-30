// ライセンス: GPL2

//
// 2ch型ノードツリー
//

#ifndef _NODETREE2ch_H
#define _NODETREE2ch_H

#include "nodetree2chcompati.h"
#include <ctime>

namespace DBTREE
{
    class NodeTree2ch : public NodeTree2chCompati
    {
        std::string m_org_url;  // 移転前のオリジナルURL
        time_t m_since_time; // スレが立った時刻
        int m_mode; // 読み込みモード
        
      public:

        NodeTree2ch( const std::string& url, const std::string& org_url,
                     const std::string& date_modified, time_t since_time );
        ~NodeTree2ch();

      protected:

        char* process_raw_lines( char* rawlines ) override;

        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

      private:

        void receive_finish() override;
    };
}

#endif
