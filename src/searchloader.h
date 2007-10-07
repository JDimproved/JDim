// ライセンス: GPL2
//
// スレタイ検索ロード
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

        virtual const std::string get_url();
        virtual const std::string get_path(){ return std::string(); }
        virtual const std::string get_charset(){ return m_charset; }

        // ロード用データ作成
        virtual void create_loaderdata( JDLIB::LOADERDATA& data );

        // ロード後に呼び出される
        virtual void parse_data();
    };
}

#endif
