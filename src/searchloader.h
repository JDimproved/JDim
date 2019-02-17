// ライセンス: GPL2
//
// スレタイ検索ローダ
//

#ifndef _SEARCHLOADER_H
#define _SEARCHLOADER_H

#include "skeleton/textloader.h"

#include <string>

namespace CORE
{
    class SearchLoader : public SKELETON::TextLoader
    {
        // 検索が終了するとemitされる
        typedef sigc::signal< void > SIG_SEARCH_FIN;

        SIG_SEARCH_FIN m_sig_search_fin;

        std::string m_charset;
        std::string m_query;

      public:

        SearchLoader();
        ~SearchLoader();

        SIG_SEARCH_FIN sig_search_fin(){ return m_sig_search_fin; }

        void search( const std::string& query );

      protected:

        std::string get_url() override;
        std::string get_path() override { return {}; }
        std::string get_charset() override { return m_charset; }

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;
    };
}

#endif
