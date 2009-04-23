// ライセンス: GPL2

#define _DEBUG
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
    assert( ! m_list_item.size() );
}


void CheckUpdate_Manager::run()
{
    if( m_running ) return;

    m_total = m_list_item.size();
    if( ! m_total ) return;

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::run total = " << m_total << std::endl;
#endif

    m_running = true;
    m_list_open.clear();
    m_url_checking = std::string();

    pop_front();
}


void CheckUpdate_Manager::stop()
{
    if( ! m_running ) return;

    m_list_item.clear();

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::stop running = " << m_url_checking << std::endl;
#endif

    DBTREE::article_stop_load( m_url_checking );
}


//
// 更新チェックするスレのグループをセットする
//
// リストの先頭にあるスレから更新チェックをしていき、もし更新されていたらグループに属する
// 残りのスレの更新チェックをキャンセルする
//
// open : true なら更新しているスレをタブで開く
//
void CheckUpdate_Manager::push_back_group( const std::list< std::string >& urllist, const bool open )
{
    if( m_running ) return;
    if( ! urllist.size() ) return;

    CheckItem item;
    item.urllist = urllist;
    item.open = open;

    m_list_item.push_back( item );

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::push_back_group "<< " open = " << open << " size = " << m_list_item.size() << std::endl;
    std::list< std::string >::const_iterator it = urllist.begin();
    for( ; it != urllist.end(); ++it ) std::cout << ( *it ) << std::endl;
#endif
}


// 簡易版
void CheckUpdate_Manager::push_back( const std::string& url, const bool open )
{
    if( m_running ) return;

    std::list< std::string > urllist;
    urllist.push_back( url );

    push_back_group( urllist, open );
}


// 次のスレをチェック
void CheckUpdate_Manager::pop_front()
{
    if( ! m_running ) return;

    // チェック完了
    if( ! m_list_item.size() ){

        m_running = false;

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front end\n";
#endif

        // 更新したスレを開く
        if( m_list_open.size() ){

            std::string urls = std::string();
            std::list< std::string >::const_iterator it = m_list_open.begin();
            for( ; it != m_list_open.end(); ++it ){

                if( DBTREE::article_status( *it ) & STATUS_UPDATE ){
                    if( ! urls.empty() ) urls += " ";
                    urls += ( *it );
                }
            }
            
#ifdef _DEBUG
            std::cout << "open urls = " << urls << std::endl;
#endif

            if( ! urls.empty() ) CORE::core_set_command( "open_article_list", std::string(), urls );
        }

        CORE::core_set_command( "set_info", "", "更新チェック完了");
    }

    // チェック実行
    else{

        CORE::core_set_command( "set_info", "",
                                "更新チェック中 (" + MISC::itostr( m_total - m_list_item.size() ) + "/" + MISC::itostr( m_total ) +")" );

        // チェックした結果、更新されていた
        if( ! m_url_checking.empty() && DBTREE::article_status( m_url_checking ) & STATUS_UPDATE ){

            // グループの残りのスレのチェックをキャンセル
            m_list_item.pop_front();
            m_url_checking = std::string();
            pop_front();
            return;
        }

        // pop
        CheckItem& item = m_list_item.front();

        if( ! item.urllist.size() ){

            // 次のグループへ
            m_list_item.pop_front();
            m_url_checking = std::string();
            pop_front();
            return;
        }

        m_url_checking = item.urllist.front();
        item.urllist.pop_front();

        if( DBTREE::article_is_loading( m_url_checking ) ){
            m_url_checking = std::string();
            pop_front();
            return;
        }

        if( item.open ) m_list_open.push_back( m_url_checking );

        if( DBTREE::article_status( m_url_checking ) & STATUS_UPDATE ){
            m_url_checking = std::string();
            pop_front();
            return;
        }

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front download url = " << m_url_checking << " size = " << m_list_item.size() << std::endl;
#endif

        // 更新チェックが終わったらローダからCheckUpdate_Manager::pop_front()が
        // コールバックされる
        const bool check_update = true;
        DBTREE::article_download_dat( m_url_checking, check_update );

        if( ! DBTREE::article_is_loading( m_url_checking ) ){
            m_url_checking = std::string();
            pop_front();
            return;
        }
    }
}
