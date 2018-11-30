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

        RuleLoader( const std::string& url_boardbase );
        ~RuleLoader();

      protected:

        const std::string get_url() override;
        const std::string get_path() override;
        const std::string get_charset() override;

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;
    };
}

#endif
