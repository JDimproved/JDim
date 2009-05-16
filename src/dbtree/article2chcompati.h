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
        virtual ~Article2chCompati();

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
