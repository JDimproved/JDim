// ライセンス: GPL2

//
// ローカルファイル用仮想板
//

#ifndef _BOARDLOCAL_H
#define _BOARDLOCAL_H

#include "board2chcompati.h"

namespace DBTREE
{
    class BoardLocal : public Board2chCompati
    {
      public:

        BoardLocal( const std::string& root, const std::string& path_board, const std::string& name );
        virtual ~BoardLocal();

        // url がこの板のものかどうか
        virtual bool equal( const std::string& url );

        virtual const std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ); 
        virtual const std::string url_readcgi( const std::string& url, int num_from, int num_to );

        virtual void download_subject( const std::string& url_update_view, const bool );

        // 板情報の読み書きをキャンセル
        virtual void read_info(){}
        virtual void save_info(){}

        // キャッシュサーチをキャンセル
        virtual void search_cache( std::vector< ArticleBase* >&, const std::string&,
                                   const bool, const bool, const bool& ) {}

        // datファイルのインポート
        virtual const std::string import_dat( const std::string& filename );

      private:

        virtual ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached );

        virtual void append_all_article_in_cache(){}

        virtual void load_rule_setting(){}
        virtual void download_rule_setting(){}
    };
}

#endif
