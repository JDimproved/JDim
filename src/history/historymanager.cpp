// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historymanager.h"
#include "historymenu.h"
#include "viewhistory.h"
#include "viewhistoryitem.h"

#include "dbtree/interface.h"
#include "jdlib/miscutil.h"

#include "xml/document.h"
#include "xml/tools.h"

#include "cache.h"
#include "session.h"
#include "sharedbuffer.h"
#include "command.h"
#include "global.h"


HISTORY::History_Manager* instance_history_manager = nullptr;


HISTORY::History_Manager* HISTORY::get_history_manager()
{
    if( ! instance_history_manager ) instance_history_manager = new History_Manager();
    assert( instance_history_manager );

    return instance_history_manager;
}


void HISTORY::delete_history_manager()
{
    if( instance_history_manager ) delete instance_history_manager;
    instance_history_manager = nullptr;
}


// url_history で指定した履歴に追加
void HISTORY::append_history( const std::string& url_history, const std::string& url, const std::string& name, const int type )
{
    get_history_manager()->append_history( url_history, url, name, type );
}

// url_history で指定した履歴の先頭を復元
void HISTORY::restore_history( const std::string& url_history )
{
    get_history_manager()->restore_history( url_history );
}


// url_history で指定した履歴を全クリア
void HISTORY::remove_allhistories( const std::string& url_history )
{
    get_history_manager()->remove_allhistories( url_history );
}


///////////////////////////////////////////////

using namespace HISTORY;

// XML ルート要素名
#define ROOT_NODE_NAME "viewhistory"


History_Manager::History_Manager()
{
#ifdef _DEBUG
    std::cout << "History_Manager::History_Manager\n";
#endif

    xml2viewhistory();
}


// 履歴メニュー取得
Gtk::MenuItem* History_Manager::get_menu_thread()
{
    if( ! m_menu_thread ) {
        m_menu_thread = std::make_unique<HistoryMenu>( URL_HISTTHREADVIEW,
                                                       std::string( ITEM_NAME_HISTVIEW ) + "(_T)" );
    }
    return m_menu_thread.get();
}


Gtk::MenuItem* History_Manager::get_menu_board()
{
    if( ! m_menu_board ) {
        m_menu_board = std::make_unique<HistoryMenu>( URL_HISTBOARDVIEW,
                                                      std::string( ITEM_NAME_HIST_BOARDVIEW ) + "(_B)" );
    }
    return m_menu_board.get();
}


Gtk::MenuItem* History_Manager::get_menu_close()
{
    if( ! m_menu_close ) {
        m_menu_close = std::make_unique<HistoryMenu>( URL_HISTCLOSEVIEW,
                                                      std::string( ITEM_NAME_HIST_CLOSEVIEW ) + "(_M)" );
    }
    return m_menu_close.get();
}

Gtk::MenuItem* History_Manager::get_menu_closeboard()
{
    if( ! m_menu_closeboard ) {
        m_menu_closeboard = std::make_unique<HistoryMenu>( URL_HISTCLOSEBOARDVIEW,
                                                           std::string( ITEM_NAME_HIST_CLOSEBOARDVIEW ) + "(_N)" );
    }
    return m_menu_closeboard.get();
}

Gtk::MenuItem* History_Manager::get_menu_closeimg()
{
    if( ! m_menu_closeimg ) {
        m_menu_closeimg = std::make_unique<HistoryMenu>( URL_HISTCLOSEIMGVIEW,
                                                         std::string( ITEM_NAME_HIST_CLOSEIMGVIEW ) + "(_I)" );
    }
    return m_menu_closeimg.get();
}


// url_history で指定した履歴に追加
void History_Manager::append_history( const std::string& url_history, const std::string& url, const std::string& name, const int type )
{
    if( SESSION::is_booting() ) return;

    CORE::DATA_INFO info;
    info.type = type;

    if( url_history == URL_HISTTHREADVIEW || url_history == URL_HISTCLOSEVIEW ){

        info.url = DBTREE::url_dat( url );
        info.name = MISC::to_plain( name );
    }
    if( url_history == URL_HISTBOARDVIEW || url_history == URL_HISTCLOSEBOARDVIEW ){

        if( type == TYPE_BOARD ){
            info.url = DBTREE::url_boardbase( url );
            info.name = DBTREE::board_name( info.url );
        }
        else{
            info.url = url;
            info.name = name;
        }
    }
    if( url_history == URL_HISTCLOSEIMGVIEW ){

        info.url = url;
        info.name = name;
    }


    CORE::DATA_INFO_LIST list_info;
    list_info.push_back( info );
    CORE::SBUF_set_list( list_info );

    CORE::core_set_command( "append_history", url_history );
    set_menulabel( url_history );
}


// url_history で指定した履歴の先頭を復元
void History_Manager::restore_history( const std::string& url_history )
{
    if( url_history == URL_HISTCLOSEVIEW && m_menu_close ) m_menu_close->restore_history();
    if( url_history == URL_HISTCLOSEBOARDVIEW && m_menu_closeboard ) m_menu_closeboard->restore_history();
    if( url_history == URL_HISTCLOSEIMGVIEW && m_menu_closeimg ) m_menu_closeimg->restore_history();
}


// url_history で指定した履歴を全クリア
void History_Manager::remove_allhistories( const std::string& url_history )
{
    CORE::core_set_command( "remove_allhistories", url_history );
}


// url_history で指定した履歴メニューのラベルを更新
void History_Manager::set_menulabel( const std::string& url_history )
{
    if( url_history == URL_HISTTHREADVIEW && m_menu_thread ) m_menu_thread->set_menulabel();
    if( url_history == URL_HISTBOARDVIEW && m_menu_board ) m_menu_board->set_menulabel();
    if( url_history == URL_HISTCLOSEVIEW && m_menu_close ) m_menu_close->set_menulabel();
    if( url_history == URL_HISTCLOSEBOARDVIEW && m_menu_closeboard ) m_menu_closeboard->set_menulabel();
    if( url_history == URL_HISTCLOSEIMGVIEW && m_menu_closeimg ) m_menu_closeimg->set_menulabel();
}


///////////////////////////////////////////////////////////////////////////////



//
// XMLを読み込んで View履歴に変換
//
void History_Manager::xml2viewhistory()
{
    std::string xml;
    CACHE::load_rawdata( CACHE::path_xml_history_view(), xml );

#ifdef _DEBUG
    std::cout << "History_Manager::xml2viewhistory\n";
    std::cout << "xml:\n" << xml << std::endl;
#endif
    if( xml.empty() ) return;

    const XML::Document document( xml );

    const XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME ) );

    for( const XML::Dom* subdir : *root ){

        if( subdir->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

        // viewhistory 作成
        const int type = XML::get_type( subdir->nodeName() );
        if( type != TYPE_DIR ) continue;

        const int top = atoi( subdir->getAttribute( "top" ).c_str() );
        const int cur = atoi( subdir->getAttribute( "cur" ).c_str() );
        const int end = atoi( subdir->getAttribute( "end" ).c_str() );

#ifdef _DEBUG
        std::cout << "\n---------------\nnew viewhistory\n"
                  << "top = " << top << std::endl
                  << "cur = " << cur << std::endl
                  << "end = " << end << std::endl;
#endif

        m_view_histories.emplace_back();
        ViewHistory& history = m_view_histories.back();

        // viewhistory に item を append
        for( const XML::Dom* histitem : *subdir ) {

            if( histitem->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

            const int item_type = XML::get_type( histitem->nodeName() );
            if( item_type != TYPE_HISTITEM ) continue;

            const std::string name = histitem->getAttribute( "name" );
            const std::string url = histitem->getAttribute( "url" );

#ifdef _DEBUG
            std::cout << "type = " << item_type << std::endl
                      << "name = " << name << std::endl
                      << "url = " << url << std::endl;
#endif
            history.append( url );
            history.replace_current_title( name );
        }

        history.set_top( top );
        history.set_cur( cur );
        history.set_end( end );
    }
}


//
// View履歴をXMLに変換して保存
//
void History_Manager::viewhistory2xml()
{
#ifdef _DEBUG
    std::cout << "History_Manager::viewhistory2xml\n";
#endif

    XML::Document document;

    // タブに表示されているViewのアドレスを取得
    std::set< std::string > taburls;
    const std::list< std::string >& article_urls = SESSION::get_article_URLs();
    const std::list< std::string >& board_urls = SESSION::get_board_URLs();

    for( const std::string& url : article_urls ) {
#ifdef _DEBUG
        std::cout << "insert " << url << std::endl;
#endif
        taburls.insert( url );
    }

    for( const std::string& url : board_urls ) {
#ifdef _DEBUG
        std::cout << "insert " << url << std::endl;
#endif
        taburls.insert( url );
    }

    // root 
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

    for( const ViewHistory& history : m_view_histories ) {

        const int size = history.get_size();
        const int top = history.get_top();
        const int cur = history.get_cur();
        const int end = history.get_end();

#ifdef _DEBUG
        std::cout << "\n---------------\nviewhistory\n"
                  << "size = "<< size << std::endl
                  << "top = " << top << std::endl
                  << "cur = " << cur << std::endl
                  << "end = " << end << std::endl
                  << "url = " << history.get_item( cur )->url << std::endl
                  << "title = " << history.get_item( cur )->title << std::endl;
#endif

        // タブに表示されていない履歴はXMLにしない
        if( taburls.find( history.get_item( cur )->url ) == taburls.end() ) continue;

#ifdef _DEBUG
        std::cout << "make xml\n";
#endif

        std::string node_name = XML::get_name( TYPE_DIR );

        XML::Dom* node = root->appendChild( XML::NODE_TYPE_ELEMENT, node_name );
        node->setAttribute( "top", top );
        node->setAttribute( "cur", cur );
        node->setAttribute( "end", end );

        if( ! size ) continue;

        node_name = XML::get_name( TYPE_HISTITEM );
        for( int i = 0; i < size; ++i ){

            XML::Dom* node_hist = node->appendChild( XML::NODE_TYPE_ELEMENT, node_name );
            node_hist->setAttribute( "name", history.get_item( i )->title );
            node_hist->setAttribute( "url", history.get_item( i )->url );
        }
    }

    std::string xml;
    if( root->hasChildNodes() ) xml = document.get_xml();

    if( ! xml.empty() ){

        CACHE::save_rawdata( CACHE::path_xml_history_view(), xml );

#ifdef _DEBUG
    std::cout << xml << std::endl;
#endif
    }
}



//
// View履歴取得
//
ViewHistory* History_Manager::get_viewhistory( const std::string& url )
{
    if( url.empty() ) return nullptr;

    // キャッシュ
    if( m_last_viewhistory && m_last_viewhistory->get_current_url() == url ) return m_last_viewhistory;

#ifdef _DEBUG
    std::cout << "History_Manager::get_view_history : " << url << std::endl
              << "size = " << m_view_histories.size() << std::endl;
#endif

    auto it = std::find_if( m_view_histories.begin(), m_view_histories.end(),
                            [&url]( auto& h ) { return h.get_current_url() == url; } );
    if( it != m_view_histories.end() ) {
#ifdef _DEBUG
        std::cout << "found\n";
#endif

        // NOTE: 挿入削除で参照が無効にならないコンテナが条件
        m_last_viewhistory = std::addressof( *it );
        return m_last_viewhistory;
    }

#ifdef _DEBUG
    std::cout << "not found\n";
#endif

    return nullptr;
}


//
// View履歴作成
//
void History_Manager::create_viewhistory( const std::string& url )
{
    ViewHistory* history = get_viewhistory( url );
    if( history ) return;

#ifdef _DEBUG
    std::cout << "History_Manager::create_viewhistory : " << url << std::endl;
#endif

    m_view_histories.emplace_back();
    m_view_histories.back().append( url );
}


//
// View履歴削除
//
void History_Manager::delete_viewhistory( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "History_Manager::delete_view_history : " << url << std::endl
              << "size = " << m_view_histories.size() << std::endl;
#endif

    auto it = std::find_if( m_view_histories.begin(), m_view_histories.end(),
                            [&url]( auto& h ) { return h.get_current_url() == url; } );
    if( it != m_view_histories.end() ) {
        m_view_histories.erase( it );
        m_last_viewhistory = nullptr;
    }
}


//
// 履歴全体で url_old を url_new に変更
//
void History_Manager::replace_url_viewhistory( const std::string& url_old, const std::string& url_new )
{
#ifdef _DEBUG
    std::cout << "History_Manager::replace_url_viewhistory\n"
              << "old = " << url_old << std::endl
              << "new = " << url_new << std::endl;
#endif

    for( auto& h : m_view_histories ) {
        h.replace_url( url_old, url_new );
    }
}


//
// View履歴タイトル更新
//
bool History_Manager::replace_current_title_viewhistory( const std::string& url, const std::string& title )
{
#ifdef _DEBUG
    std::cout << "History_Manager::replace_current_title_viewhistory\n"
              << "url = " << url   << std::endl
              << "new = " << title << std::endl;
#endif

    ViewHistory* history = get_viewhistory( url );
    if( !history ) return false;

    history->replace_current_title( title );
    return true;
}


static std::vector<ViewHistoryItem*> s_nullitems;


// item の取得
std::vector< ViewHistoryItem* >& History_Manager::get_items_back_viewhistory( const std::string& url, const int count )
{
    ViewHistory* history = get_viewhistory( url );
    if( !history ) return s_nullitems;

    return history->get_items_back( count );
}


std::vector< ViewHistoryItem* >& History_Manager::get_items_forward_viewhistory( const std::string& url, const int count )
{
    ViewHistory* history = get_viewhistory( url );
    if( !history ) return s_nullitems;

    return history->get_items_forward( count );
}


//
// View履歴追加
//
bool History_Manager::append_viewhistory( const std::string& url_current, const std::string& url_append )
{
#ifdef _DEBUG
    std::cout << "History_Manager::append_viewhistory" << std::endl
              << "currnet = " << url_current << std::endl
              << "append = " << url_append << std::endl;
#endif

    ViewHistory* history = get_viewhistory( url_current );
    if( !history ) return false;

    history->append( url_append );
    return true;
}


//
// View履歴「戻る」可能
//
bool History_Manager::can_back_viewhistory( const std::string& url, const int count )
{
    ViewHistory* history = get_viewhistory( url );
    if( ! history ) return false;

    return history->can_back( count );
}


//
// View履歴「進む」可能
//
bool History_Manager::can_forward_viewhistory( const std::string& url, const int count )
{
    ViewHistory* history = get_viewhistory( url );
    if( ! history ) return false;

    return history->can_forward( count );
}


//
// View履歴戻る
//
// exec = true のときは履歴の位置を変更する
// false の時はURLの取得のみ
//
const ViewHistoryItem* History_Manager::back_viewhistory( const std::string& url, const int count, const bool exec )
{
#ifdef _DEBUG
    std::cout << "History_Manager::back_viewhistory count = " << count
              << " exec = " << exec << " url = " << url << std::endl;
#endif

    ViewHistory* history = get_viewhistory( url );
    if( ! history ) return nullptr;

    return history->back( count, exec );
}


//
// View履歴進む
//
// exec = true のときは履歴の位置を変更する
// false の時はURLの取得のみ
//
const ViewHistoryItem* History_Manager::forward_viewhistory( const std::string& url, const int count, const bool exec )
{
#ifdef _DEBUG
    std::cout << "History_Manager::forward_viewhistory count = " << count
              << " exec = " << exec << " url = " << url << std::endl;
#endif

    ViewHistory* history = get_viewhistory( url );
    if( ! history ) return nullptr;

    return history->forward( count, exec );
}
