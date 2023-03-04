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

      public:

        explicit RuleLoader( const std::string& url_boardbase );
        ~RuleLoader();

      protected:

        std::string get_url() const override;
        std::string get_path() const override;

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;

      private:

        void receive_cookies() override;
    };
}

#endif
