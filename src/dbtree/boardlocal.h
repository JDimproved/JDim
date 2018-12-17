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
        ~BoardLocal() noexcept;

        // url がこの板のものかどうか
        bool equal( const std::string& url ) override;

        std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ) override;
        std::string url_readcgi( const std::string& url, int num_from, int num_to ) override;

        void download_subject( const std::string& url_update_view, const bool ) override;

        // 板情報の読み書きをキャンセル
        void read_info() override {}
        void save_info() override {}

        // キャッシュサーチをキャンセル
        void search_cache( std::vector< ArticleBase* >&, const std::string&, const bool, const bool,
                           const bool& ) override {}

        // datファイルのインポート
        std::string import_dat( const std::string& filename ) override;

      private:

        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;

        void append_all_article_in_cache() override {}

        void load_rule_setting() override {}
        void download_rule_setting() override {}
    };
}

#endif
