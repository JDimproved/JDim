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
        const std::string create_write_message( const std::string& name, const std::string& mail,
                                                const std::string& msg ) override;

        // bbscgi のURL
        const std::string url_bbscgi() override;
        
        // subbbscgi のURL
        const std::string url_subbbscgi() override;

      private:
        
        // 更新チェック不可能
        const bool enable_check_update() override { return false; }

        NodeTreeBase* create_nodetree() override;
    };
}

#endif
