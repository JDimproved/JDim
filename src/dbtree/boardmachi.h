// ライセンス: GPL2

//
// まち 型板
//

#ifndef _BOARDMACHI_H
#define _BOARDMACHI_H

#include "boardbase.h"

namespace DBTREE
{
    class BoardMachi : public BoardBase
    {
      public:

        BoardMachi( const std::string& root, const std::string& path_board,const std::string& name );
        ~BoardMachi() noexcept override = default;

        // url がこの板のものかどうか
        bool equal( const std::string& url ) const override;

        // スレの url を dat型のurlに変換して出力
        // (例) "http://hoge.machi.to/bbs/read.cgi?BBS=board&KEY=12345&START=12&END=15"" のとき
        // 戻り値 : "http://hoge.machi.to/bbs/read.cgi?BBS=board&KEY=12345", num_from = 12, num_to = 15, num_str = 12-15
        std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ) override;

        std::string url_datpath() const override;

        // 新スレ作成時の書き込みメッセージ作成
        std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                               const std::string& mail, const std::string& msg,
                                               const bool utf8_post ) override;

        // 新スレ作成用のbbscgi のURL
        std::string url_bbscgi_new() const override;

        // 新スレ作成用のsubbbscgi のURL
        std::string url_subbbscgi_new() const override;

      private:

        bool is_valid( const std::string& filename ) const override;
        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;
        void parse_subject( const char* str_subject_txt ) override;
        void regist_article( const bool is_online ) override;
    };
}

#endif
