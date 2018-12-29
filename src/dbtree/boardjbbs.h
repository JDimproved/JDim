// ライセンス: GPL2

//
// JBBS 型板
//

#ifndef _BOARDJBBS_H
#define _BOARDJBBS_H

#include "boardbase.h"

namespace DBTREE
{
    class BoardJBBS : public BoardBase
    {
      public:

        BoardJBBS( const std::string& root, const std::string& path_board,const std::string& name );

        const std::string url_datpath() override;

        // 新スレ作成用のメッセージ変換
        const std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                                     const std::string& mail, const std::string& msg ) override;

        // 新スレ作成用のbbscgi のURL
        const std::string url_bbscgi_new() override;
        
        // 新スレ作成用のsubbbscgi のURL
        const std::string url_subbbscgi_new() override;

      private:

        const bool is_valid( const std::string& filename ) override;
        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;
        void parse_subject( const char* str_subject_txt ) override;
        void regist_article( const bool is_online ) override;
    };
}

#endif
