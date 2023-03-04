// ライセンス: GPL2
//
// 板のフロントページのローダー
//

#ifndef JDIM_FRONTLOADER_H
#define JDIM_FRONTLOADER_H

#include "skeleton/textloader.h"

#include <string>

namespace JDLIB
{
    class LOADERDATA;
}

namespace DBTREE
{
    class FrontLoader : public SKELETON::TextLoader
    {
        std::string m_url_boadbase;

      public:

        explicit FrontLoader( const std::string& url_boardbase );
        ~FrontLoader() = default;

      protected:

        std::string get_url() const override { return m_url_boadbase; }
        std::string get_path() const override { return {}; } // キャッシュには保存しない

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;

      private:

        void receive_cookies() override;
    };
}

#endif // JDIM_FRONTLOADER_H
