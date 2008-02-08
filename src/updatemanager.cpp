// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "updatemanager.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "command.h"
#include "global.h"

#include <algorithm>

CORE::CheckUpdate_Manager* instance_checkupdate_manager = NULL;


CORE::CheckUpdate_Manager* CORE::get_checkupdate_manager()
{
    if( ! instance_checkupdate_manager ) instance_checkupdate_manager = new CheckUpdate_Manager();
    assert( instance_checkupdate_manager );

    return instance_checkupdate_manager;
}


void CORE::delete_checkupdate_manager()
{
    if( instance_checkupdate_manager ) delete instance_checkupdate_manager;
    instance_checkupdate_manager = NULL;
}


///////////////////////////////////////////////

using namespace CORE;

CheckUpdate_Manager::CheckUpdate_Manager()
    : m_running( false )
{}


CheckUpdate_Manager::~CheckUpdate_Manager()
{
    assert( ! m_list_urls.size() );
}


void CheckUpdate_Manager::run( bool open )
{
    if( m_running ) return;

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::run open = " << open << std::endl;
#endif

    m_running = true;
    m_open = open;
    m_total = m_list_urls.size();
    m_list_urls_updated.clear();
    pop_front();
}


void CheckUpdate_Manager::stop()
{
    if( ! m_running ) return;

    std::string url = m_list_urls_updated.back();
    m_list_urls.clear();

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::stop running = " << url << std::endl;
#endif

    DBTREE::article_stop_load( url );
}


void CheckUpdate_Manager::push_back( const std::string& url )
{
    if( m_running ) return;

    if( std::find( m_list_urls.begin(), m_list_urls.end(), url ) == m_list_urls.end() ){

        m_list_urls.push_back( url );

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::push_back url = " << url << " size = " << m_list_urls.size() << std::endl;
#endif
    }
}


void CheckUpdate_Manager::pop_front()
{
    if( ! m_running ) return;

    // チェック完了
    if( ! m_list_urls.size() ){
        m_running = false;

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front stop\n";
#endif

        // 更新したスレを開く
        if( m_open ){

            std::string urls;
            std::list< std::string >::iterator it = m_list_urls_updated.begin();
            for( ; it != m_list_urls_updated.end(); ++it ){

                if( DBTREE::article_status( *it ) & STATUS_UPDATE ){
                    if( ! urls.empty() ) urls += " ";
                    urls += (*it);
                }
            }
            
#ifdef _DEBUG
            std::cout << "urls = " << urls << std::endl;
#endif

            if( ! urls.empty() ) CORE::core_set_command( "open_article_list", std::string(), urls );
        }

        CORE::core_set_command( "set_info", "", "更新チェック完了");
    }

    // チェック実行
    else{

        CORE::core_set_command( "set_info", "",
                                "更新チェック中 (" + MISC::itostr( m_total - m_list_urls.size() ) + "/" + MISC::itostr( m_total ) +")" );

        std::string url = m_list_urls.front();
        m_list_urls.pop_front();
        m_list_urls_updated.push_back( url );

        if( DBTREE::article_is_loading( url ) ){
            pop_front();
            return;
        }

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front url = " << url << " size = " << m_list_urls.size() << std::endl;
#endif

        // 更新チェックが終わったらローダからCheckUpdate_Manager::pop_front()が
        // コールバックされる
        DBTREE::article_download_dat( url, true );

        if( ! DBTREE::article_is_loading( url ) ){
            pop_front();
            return;
        }
    }
}
