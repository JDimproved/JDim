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

        virtual const std::string get_url();
        virtual const std::string get_path();
        virtual const std::string get_charset();

        // ロード用データ作成
        virtual void create_loaderdata( JDLIB::LOADERDATA& data );

        // ロード後に呼び出される
        virtual void parse_data();
    };
}

#endif
