// ライセンス: GPL2

//
// 2ch互換型スレ情報クラス
//

#ifndef _ARTICLE2CHCOMPATI_H
#define _ARTICLE2CHCOMPATI_H

#include "articlebase.h"

namespace DBTREE
{
    class Article2chCompati : public ArticleBase
    {
      public:

        Article2chCompati( const std::string& datbase, const std::string& id, bool cached );
        ~Article2chCompati() noexcept;

        // 書き込みメッセージ変換
        std::string create_write_message( const std::string& name, const std::string& mail,
                                          const std::string& msg ) override;

        // bbscgi のURL
        std::string url_bbscgi() override;
        
        // subbbscgi のURL
        std::string url_subbbscgi() override;

      private:
        
        NodeTreeBase* create_nodetree() override;
    };
}

#endif
