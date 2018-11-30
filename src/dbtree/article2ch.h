// ライセンス: GPL2

//
// 2ch型スレ情報クラス
//

#ifndef _ARTICLE2ch_H
#define _ARTICLE2ch_H

#include "article2chcompati.h"

namespace DBTREE
{
    class Article2ch : public Article2chCompati
    {
      public:

        Article2ch( const std::string& datbase, const std::string& id, bool cached );
        ~Article2ch() noexcept;

        // 書き込みメッセージ変換
        const std::string create_write_message( const std::string& name, const std::string& mail,
                                                const std::string& msg ) override;

        // bbscgi のURL
        const std::string url_bbscgi() override;
        
        // subbbscgi のURL
        const std::string url_subbbscgi() override;

      protected:

        // dat落ちしたスレをロードするか
        const bool is_load_olddat() override;

      private:
        
        NodeTreeBase* create_nodetree() override;
    };
}

#endif
