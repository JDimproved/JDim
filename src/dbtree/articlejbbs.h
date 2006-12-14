// ライセンス: GPL2

//
// JBBS型スレ情報クラス
//

#ifndef _ARTICLEJBBS_H
#define _ARTICLEJBBS_H

#include "articlebase.h"

namespace DBTREE
{
    class NodeTreeBase;

    class ArticleJBBS : public ArticleBase
    {
      public:

        ArticleJBBS( const std::string& datbase, const std::string& id, bool cached );
        ~ArticleJBBS();

        // 書き込みメッセージ変換
        virtual const std::string create_write_message( const std::string& name, const std::string& mail, const std::string& msg );

        // bbscgi のURL
        virtual const std::string url_bbscgi();
        
        // subbbscgi のURL
        virtual const std::string url_subbbscgi();

      private:
        
        virtual NodeTreeBase* create_nodetree();
    };
}

#endif
