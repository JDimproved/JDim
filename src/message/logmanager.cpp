// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "logmanager.h"
#include "logitem.h"

#include "config/globalconf.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "cache.h"
#include "session.h"

#include <sstream>
#include <sys/time.h>
#include <cstring>


enum
{
    WRITE_TIMEOUT = 120  // 書き込んでからこの秒数を過ぎても書き込んだレスを判定できなかったらロストとする
};


MESSAGE::Log_Manager* instance_log_manager = NULL;

MESSAGE::Log_Manager* MESSAGE::get_log_manager()
{
    if( ! instance_log_manager ) instance_log_manager = new MESSAGE::Log_Manager();
    assert( instance_log_manager );

    return instance_log_manager;
}


void MESSAGE::delete_log_manager()
{
    if( instance_log_manager ) delete instance_log_manager;
    instance_log_manager = NULL;
}



///////////////////////////////////////////////

using namespace MESSAGE;


Log_Manager::Log_Manager()
{
#ifdef _DEBUG
    std::cout << "MESSAGE::Log_Manager\n";
#endif
}


Log_Manager::~Log_Manager()
{
#ifdef _DEBUG
    std::cout << "MESSAGE::~Log_Manager\n";
#endif

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){
#ifdef _DEBUG
        std::cout << "url = " << (*it)->url << std::endl;
        std::cout << "subject = " << (*it)->subject << std::endl;
        std::cout << "msg = " << (*it)->msg << std::endl;
#endif
        delete *it;
    }
}



const bool Log_Manager::has_items( const std::string& url )
{
    if( ! m_logitems.size() ) return false;

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ) if( (*it)->url == url ) return true;

    return false;
}


void Log_Manager::remove_items( const std::string& url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    if( ! m_logitems.size() ) return;

#ifdef _DEBUG
    std::cout << "Log_Manager::remove_items url = " << url << std::endl
              << "size = " << m_logitems.size() << std::endl;
#endif 

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){

        if( (*it)->url == url ){

            const time_t elapsed = tv.tv_sec - (*it)->time_write;

#ifdef _DEBUG
            std::cout << "elapsed = " << elapsed << std::endl;
#endif 

            // removeフラグが立っているか、時間切れの場合は削除
            if( (*it)->remove || elapsed > WRITE_TIMEOUT ){

#ifdef _DEBUG
                std::cout << "removed url = " << (*it)->url << std::endl;
#endif
                delete (*it);
                m_logitems.erase( it );
                it = m_logitems.begin();
            }
        }
    }

#ifdef _DEBUG
    std::cout << "-> size = " << m_logitems.size() << std::endl;
#endif 
}



// messageが自分の書き込んだものかチェックする ( 高速版 )
const bool Log_Manager::check_write_fast( const std::string& url, const std::string& msg )
{
    if( ! m_logitems.size() ) return false;

#ifdef _DEBUG
    std::cout << "Log_Manager::check_write_fast url = " << url << std::endl;
#endif

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){

        if( (*it)->url != url ) continue;

        // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
        // 改行のみの行を削除しないようにするため
        std::list< std::string > msg_lines = MISC::get_lines( MISC::replace_str( MISC::remove_spaces( msg ), "\n", " \n" ) );

#ifdef _DEBUG
        std::cout << "lines = " << msg_lines.size() << " : " << (*it)->msg_lines.size() << std::endl;
#endif

        if( msg_lines.size() != (*it)->msg_lines.size() ) continue;

        std::list< std::string >::iterator it_msg = msg_lines.begin();
        std::list< std::string >::iterator it_item = (*it)->msg_lines.begin();
        for( ; it_msg != msg_lines.end() ; ++it_msg, ++it_item ){
#ifdef _DEBUG
            std::cout << (*it_msg) << " | " << (*it_item) << std::endl;
#endif
            if( MISC::remove_spaces( (*it_msg) ) != MISC::remove_spaces( (*it_item ) ) ) break;
        }
        if( it_msg != msg_lines.end() ) continue;

#ifdef _DEBUG
        std::cout << "!! hit !!\n";
#endif

        (*it)->remove = true;

        return true;

    }

    return false;
}



void Log_Manager::save( const std::string& url,
                        const std::string& subject,  const std::string& msg, const std::string& name, const std::string& mail )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    m_logitems.push_back( new LogItem( url, subject, MISC::remove_spaces( msg ), tv.tv_sec ) );

#ifdef _DEBUG
    std::cout << "Log_Manager::save\n";
    std::cout << "url = " << url << std::endl;
    std::cout << "subject = " << subject << std::endl;
    std::cout << "msg = " << msg << std::endl;
#endif

    if( ! CONFIG::get_save_postlog() ) return;

    // 実況中の時は保存しない
    if( SESSION::is_live( url ) ) return;

    const std::string date = MISC::timettostr( tv.tv_sec );

    std::stringstream ss;
    ss << "---------------" << std::endl
       << url << std::endl
       << "[ " << DBTREE::board_name( url ) << " ] " << subject << std::endl
       << "名前：" << name << " [" << mail << "]：" << date << std::endl
       << msg << std::endl;

#ifdef _DEBUG
    std::cout << ss.str() << std::endl;
#endif 

    CACHE::save_rawdata( CACHE::path_postlog(), ss.str(), true );
}
