// ライセンス: GPL2

//
// 書き込みログの管理クラス
//

#ifndef _LOGMANAGER_H
#define _LOGMANAGER_H

#include <list>

namespace MESSAGE
{
    class LogItem;

    class Log_Manager
    {
        std::list< LogItem* > m_logitems;

      public:

        Log_Manager();
        virtual ~Log_Manager();

        const bool has_items( const std::string& url );
        void remove_items( const std::string& url );

        // messageが自分の書き込んだものかチェックする ( 高速版 )
        const bool check_write_fast( const std::string& url, const std::string& msg );

        void save( const std::string& url,
                   const std::string& subject,  const std::string& msg, const std::string& name, const std::string& mail );

    };

    ///////////////////////////////////////
    // インターフェース

    Log_Manager* get_log_manager();
    void delete_log_manager();
}

#endif
