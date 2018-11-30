// ライセンス: GPL2

//
// ローカルファイル用スレ情報クラス
//

#ifndef _ARTICLELOCAL_H
#define _ARTICLELOCAL_H

#include "article2chcompati.h"

namespace DBTREE
{
    class ArticleLocal : public Article2chCompati
    {
      public:

        ArticleLocal( const std::string& datbase, const std::string& id );
        ~ArticleLocal();

        // ID がこのスレのものかどうか
        const bool equal( const std::string& datbase, const std::string& id ) override;

        // キャッシュの削除をしない
        void delete_cache( const bool cache_only ) override {}

        // 情報ファイルを保存しない
        void save_info( const bool force ) override {}

        // ダウンロードしない
        void download_dat( const bool check_update ) override {}

      private:
        
        NodeTreeBase* create_nodetree() override;
    };
}

#endif
