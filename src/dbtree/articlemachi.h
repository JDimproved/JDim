// ライセンス: GPL2

//
// まち型スレ情報クラス
//

#ifndef _ARTICLEMACHI_H
#define _ARTICLEMACHI_H

#include "articlebase.h"

namespace DBTREE
{
    class NodeTreeBase;

    class ArticleMachi : public ArticleBase
    {
      public:

        ArticleMachi( const std::string& datbase, const std::string& id, bool cached );
        ~ArticleMachi() noexcept;

        // 書き込みメッセージ変換
        const std::string create_write_message( const std::string& name, const std::string& mail,
                                                const std::string& msg ) override;

        // bbscgi のURL
        const std::string url_bbscgi() override;
        
        // subbbscgi のURL
        const std::string url_subbbscgi() override;

      private:
        
        // offlawモードなら更新チェック可能
        const bool enable_check_update() override;

        NodeTreeBase* create_nodetree() override;
    };
}

#endif
