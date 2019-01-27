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
{
    m_list_open.clear();
}


CheckUpdate_Manager::~CheckUpdate_Manager() noexcept
{
    assert( ! m_list_item.size() );
}


void CheckUpdate_Manager::run()
{
    if( m_running ){

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::run failed items = " << m_list_item.size() << std::endl;
#endif
        return;
    }

    m_total = m_list_item.size();
    if( ! m_total ) return;

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::run total = " << m_total << std::endl;
#endif

    m_running = true;
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

    // ArticleBase::slot_load_finished()経由でpop_front()が呼び出されて m_running が false になる
    DBTREE::article_stop_load( m_url_checking );
}


//
// 更新チェックする板やスレをセットする
//
// open == true なら更新チェック終了時に url を開く( url が更新可能状態なら )
//
void CheckUpdate_Manager::push_back( const std::string& url, const bool open )
{
    if( m_running ) return;

#ifdef _DEBUG
    std::cout << "CheckUpdate_Manager::push_back "
              << " open = " << open
              << " url = " << url << std::endl;
#endif

    std::list< std::string > urllist;

    // urlが板のアドレスかスレのアドレスか判断する
    int num_from, num_to;
    std::string num_str;
    const std::string url_dat = DBTREE::url_dat( url, num_from, num_to, num_str );
    const std::string url_subject = DBTREE::url_subject( url );

    // スレ
    if( ! url_dat.empty() ){
#ifdef _DEBUG
        std::cout << "type = dat\n"
                  << DBTREE::article_subject( url ) << std::endl;
#endif
        urllist.push_back( url );
    }

    // 板
    else if( ! url_subject.empty() ){
#ifdef _DEBUG
        std::cout << "type = board\n"
                  << url_subject << std::endl;
#endif
        urllist = DBTREE::board_get_check_update_articles( url );
    }

    else return;

    if( open ) m_list_open.push_back( url );
    if( ! urllist.size() ) return;

    // リストの先頭にあるスレから更新チェックをしていき、もし更新されていたらグループに属する
    // 残りのスレの更新チェックをキャンセルする
    CheckItem item;
    item.urllist = urllist;
    m_list_item.push_back( item );

#ifdef _DEBUG
    std::cout << "size = " << m_list_item.size() << std::endl;

    std::list< std::string >::const_iterator it = urllist.begin();
    for( ; it != urllist.end(); ++it ) std::cout << ( *it ) << std::endl;
#endif
}


// 次のスレをチェック
void CheckUpdate_Manager::pop_front()
{
    if( ! m_running ) return;

    // チェック完了
    if( ! m_list_item.size() ){

        m_running = false;

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front end size = " << m_list_item.size() << std::endl;
#endif

        // 更新したスレを開く
        if( m_list_open.size() ){

            std::string urls_article = std::string();
            std::string urls_board = std::string();

            std::list< std::string >::const_iterator it = m_list_open.begin();
            for( ; it != m_list_open.end(); ++it ){

                const std::string& url = ( *it );

                // urlが板のアドレスかスレのアドレスか判断する
                int num_from, num_to;
                std::string num_str;
                const std::string url_dat = DBTREE::url_dat( url, num_from, num_to, num_str );
                const std::string url_subject = DBTREE::url_subject( url );

                // スレ
                if( ! url_dat.empty() ){

                    if( DBTREE::article_status( url ) & STATUS_UPDATE ) urls_article += url + " ";
                }

                // 板
                else if( ! url_subject.empty() ){

                    if( DBTREE::board_status( url ) & STATUS_UPDATE ) urls_board += url + " ";
                }
            }
            
#ifdef _DEBUG
            std::cout << "open urls_article = " << urls_article << std::endl;
            std::cout << "open urls_board = " << urls_board << std::endl;
#endif

            if( ! urls_article.empty() ) CORE::core_set_command( "open_article_list", std::string(), urls_article );
            if( ! urls_board.empty() ) CORE::core_set_command( "open_board_list", std::string(), urls_board );

            m_list_open.clear();
        }

        CORE::core_set_command( "set_info", "", "更新チェック完了");
    }

    // チェック実行
    else{

        CORE::core_set_command( "set_info", "",
                                "更新チェック中 (" + MISC::itostr( m_total - m_list_item.size() ) + "/" + MISC::itostr( m_total ) +")" );

        // チェックした結果、更新されていたら
        // グループの残りのスレのチェックをキャンセルして次のグループへ
        if( ! m_url_checking.empty()
            && ( DBTREE::article_status( m_url_checking ) & STATUS_UPDATE ) ){

            m_list_item.pop_front();
            m_url_checking = std::string();
            pop_front();
            return;
        }

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

#ifdef _DEBUG
        std::cout << "CheckUpdate_Manager::pop_front download url = " << m_url_checking << " size = " << m_list_item.size() << std::endl;
#endif

        // 更新チェックが終わったらローダからpop_front()がコールバックされる
        const bool check_update = true;
        DBTREE::article_download_dat( m_url_checking, check_update );

        if( ! DBTREE::article_is_checking_update( m_url_checking ) ){

#ifdef _DEBUG
            std::cout << "skipped\n";
#endif
            m_url_checking = std::string();
            pop_front();
            return;
        }

#ifdef _DEBUG
        std::cout << "started\n";
#endif
    }
}
