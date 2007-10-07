// ライセンス: GPL2

//
// ログ、スレタイ検索クラス
//

#ifndef _SEARCHMANAGER_H
#define _SEARCHMANAGER_H

#include "skeleton/dispatchable.h"

#include <gtkmm.h>
#include <pthread.h>

#include <string>
#include <list>

namespace CORE
{
    class SearchLoader;

    struct SEARCHDATA
    {
        std::string url_readcgi;
        std::string boardname;
        std::string subject;
        int num;
    };

    class Search_Manager : public SKELETON::Dispatchable
    {
        typedef sigc::signal< void > SIG_SEARCH_FIN;

        SIG_SEARCH_FIN m_sig_search_fin;

        pthread_t m_thread;

        std::string m_id;
        std::string m_url;
        std::string m_query;
        bool m_mode_or;
        bool m_searchall;

        std::list< SEARCHDATA > m_list_data;

        // 検索実行中
        bool m_searching;

        bool m_stop;

        // スレタイ検索ローダ
        SearchLoader* m_searchloader;

      public:

        Search_Manager();
        virtual ~Search_Manager();

        SIG_SEARCH_FIN sig_search_fin(){ return m_sig_search_fin; }

        const bool is_searching() const { return m_searching; }
        const std::string& get_id() const { return m_id; }
        const std::list< SEARCHDATA >& get_list_data() const { return m_list_data; }

        // ログ検索
        bool search( const std::string& id, const std::string& url, const std::string& query,
                     bool mode_or, bool searchall );

        // スレタイ検索
        bool search_title( const std::string& id, const std::string& query );

        void stop();

      private:
        static void* launcher( void* );
        void thread_search();
        virtual void callback_dispatch();
        void search_fin();
        void search_fin_title();
    };

    ///////////////////////////////////////
    // インターフェース

    Search_Manager* get_search_manager();
    void delete_search_manager();
}


#endif
