// ライセンス: GPL2

//
// 2ch型ノードツリー
//

#ifndef _NODETREE2ch_H
#define _NODETREE2ch_H

#include "nodetree2chcompati.h"

namespace DBTREE
{
    class NodeTree2ch : public NodeTree2chCompati
    {
        std::string m_org_url;  // 移転前のオリジナルURL
        bool m_use_offlaw; // offlawを使用する
        
      public:

        NodeTree2ch( const std::string& url, const std::string& org_url, const std::string& date_modified );
        ~NodeTree2ch();

      protected:

        virtual void create_loaderdata( JDLIB::LOADERDATA& data );

      private:

        virtual void receive_finish();
    };
}

#endif
