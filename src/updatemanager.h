// ライセンス: GPL2

//
// 更新チェッククラス
//

#ifndef _UPDATEMANAGER_H
#define _UPDATEMANAGER_H

#include <list>
#include <string>

namespace CORE
{
    class CheckUpdate_Manager
    {
        std::list< std::string > m_list_urls;
        std::list< std::string > m_list_urls_updated;

        bool m_running;
        int m_total;
        bool m_open;

      public:

        CheckUpdate_Manager();
        virtual ~CheckUpdate_Manager();

        void run( bool open );
        void stop();

        void push_back( const std::string& url );
        void pop_front();
    };

    ///////////////////////////////////////
    // インターフェース

    CheckUpdate_Manager* get_checkupdate_manager();
    void delete_checkupdate_manager();
}

#endif
