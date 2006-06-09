
// ライセンス: 最新のGPL

//
// 2ch 互換型板
//

#ifndef _BOARD2CHCOMPATI_H
#define _BOARD2CHCOMPATI_H

#include "boardbase.h"

namespace DBTREE
{
    class Board2chCompati : public BoardBase
    {
      public:

        Board2chCompati( const std::string& root, const std::string& path_board, const std::string& name );

        // 書き込み用クッキー
        virtual const std::string cookie_for_write();

        // 新スレ作成用のメッセージ変換
        virtual const std::string create_newarticle_message( const std::string& subject,
                                                             const std::string& name, const std::string& mail, const std::string& msg );
        // 新スレ作成用のbbscgi のURL
        virtual const std::string url_bbscgi_new();
        
        // 新スレ作成用のsubbbscgi のURL
        virtual const std::string url_subbbscgi_new();

      private:

        virtual bool is_valid( const std::string& filename );

        virtual ArticleBase* append_article( const std::string& id, bool cached );
        virtual void parse_subject( const char* str_subject_txt );
    };
}

#endif
