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
        virtual const bool equal( const std::string& datbase, const std::string& id );

        // キャッシュの削除をしない
        virtual void delete_cache( const bool cache_only ){}

        // 情報ファイルを保存しない
        virtual void save_info( const bool force ){}

        // ダウンロードしない
        virtual void download_dat( const bool check_update ){}

      private:
        
        virtual NodeTreeBase* create_nodetree();
    };
}

#endif
