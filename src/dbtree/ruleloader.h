// ライセンス: GPL2
//
// 2chのローカルルールのローダ
//

#ifndef _RULELOADER_H
#define _RULELOADER_H

#include "skeleton/textloader.h"

#include <string>

namespace JDLIB
{
    class LOADERDATA;
}

namespace DBTREE
{
    class RuleLoader : public SKELETON::TextLoader
    {
        std::string m_url_boadbase;
        // スレ本文とエンコーディングが異なる板があるため指定可能にする
        // 文字列の寿命は呼び出し元が責任を持つこと
        const char* m_override_charset;

      public:

        explicit RuleLoader( const std::string& url_boardbase, const char* override_charset = nullptr );
        ~RuleLoader();

      protected:

        std::string get_url() const override;
        std::string get_path() const override;
        std::string get_charset() const override;

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;

      private:

        void receive_cookies() override;
    };
}

#endif
