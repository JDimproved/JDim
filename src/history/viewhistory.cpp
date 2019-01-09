// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "viewhistory.h"

using namespace HISTORY;

enum
{
    MAX_LOCAL_HISTORY = 20  // 履歴保持数
};


ViewHistory::ViewHistory()
   :  m_history_top( 0 ), m_history_current( 0 ), m_history_end( 0 )
{
#ifdef _DEBUG
    std::cout << "ViewHistory::ViewHistory\n";
#endif

    m_items.push_back( new ViewHistoryItem() );
}


ViewHistory::~ViewHistory()
{
#ifdef _DEBUG
    std::cout << "ViewHistory::~ViewHistory\n";
#endif

    for( size_t i = 0; i < m_items.size(); ++i ) delete m_items[ i ];
}


void ViewHistory::set_top( const int top )
{
    if( m_items.size() < MAX_LOCAL_HISTORY && top > m_history_top ) return;

    m_history_top = top;

#ifdef _DEBUG
    std::cout << "ViewHistory::set_top" << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif
}


void ViewHistory::set_cur( const int cur )
{
    if( m_items.size() < MAX_LOCAL_HISTORY && cur > m_history_top ) m_history_current = m_history_top;
    else  m_history_current = cur;

#ifdef _DEBUG
    std::cout << "ViewHistory::set_cur" << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif
}


void ViewHistory::set_end( const int end )
{
    m_history_end = end;

#ifdef _DEBUG
    std::cout << "ViewHistory::set_end" << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif
}


const std::string& ViewHistory::get_current_url()
{
#ifdef _DEBUG
    std::cout << "ViewHistory::get_current_url" << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
    assert( m_items[ m_history_current ] != NULL );
#endif

    return m_items[ m_history_current ]->url;
}


const std::string& ViewHistory::get_current_title()
{
    return m_items[ m_history_current ]->title;
}


//
// URL更新
//
void ViewHistory::replace_current_url( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ViewHistory::replace_current_url\n"
              << "old = " << m_items[ m_history_current ]->url << std::endl
              << "new = " << url << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif

    m_items[ m_history_current ]->url = url;
}

void ViewHistory::replace_url( const std::string& url_old, const std::string& url_new )
{
#ifdef _DEBUG
    std::cout << "ViewHistory::replace_url\n"
              << "old = " << url_old << std::endl
              << "new = " << url_new << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif

    int size = m_items.size();
    for( int i = 0; i < size; ++i ){
        if( m_items[ i ]->url == url_old ){
            m_items[ i ]->url = url_new;
#ifdef _DEBUG
            std::cout << "replaced\n";
#endif
        }
    }
}


//
// タイトル更新
//
void ViewHistory::replace_current_title( const std::string& title )
{
#ifdef _DEBUG
    std::cout << "ViewHistory::replace_current_title\n"
              << "old = " << m_items[ m_history_current ]->title << std::endl
              << "new = " << title << std::endl
              << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl;
#endif

    m_items[ m_history_current ]->title = title;
}


// item の取得
std::vector< ViewHistoryItem* >& ViewHistory::get_items_back( const int count )
{
    static std::vector< ViewHistoryItem* > items;
    items.clear();

    if( count <= 0 ) return items;

    int tmp_current = m_history_current;

    for( int i = 0; i < count; ++i ){
        if( tmp_current == m_history_end ) break;
        tmp_current = ( tmp_current + MAX_LOCAL_HISTORY - 1 ) % MAX_LOCAL_HISTORY;
        items.push_back( m_items[ tmp_current ] );
    }

    return items;
}


std::vector< ViewHistoryItem* >& ViewHistory::get_items_forward( const int count )
{
    static std::vector< ViewHistoryItem* > items;
    items.clear();

    if( count <= 0 ) return items;

    int tmp_current = m_history_current;

    for( int i = 0; i < count; ++i ){
        if( tmp_current == m_history_top ) break;
        tmp_current = ( tmp_current + 1 ) % MAX_LOCAL_HISTORY;
        items.push_back( m_items[ tmp_current ] );
    }

    return items;
}


//
// 「戻る」可能
//
bool ViewHistory::can_back( const int count )
{
    if( count <= 0 ) return false;

    int tmp_current = m_history_current;

    for( int i = 0; i < count; ++i ){
        if( tmp_current == m_history_end ) return false;
        tmp_current = ( tmp_current + MAX_LOCAL_HISTORY - 1 ) % MAX_LOCAL_HISTORY;
    }

    return true;
}


//
// 「進む」可能
//
bool ViewHistory::can_forward( const int count )
{
    if( count <= 0 ) return false;

    int tmp_current = m_history_current;

    for( int i = 0; i < count; ++i ){
        if( tmp_current == m_history_top ) return false;
        tmp_current = ( tmp_current + 1 ) % MAX_LOCAL_HISTORY;
    }

    return true;
}


// ローカル履歴追加
void ViewHistory::append( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "-------------\nViewhHstory::append : " << url << std::endl;
#endif

    // 一番最初の呼び出し
    if( m_items.size() == 1 && m_items[ 0 ]->url.empty() ) m_items[ 0 ]->url = url;

    else{

        if( get_current_url() == url ) return;

        // 既に登録されていないか確認
        bool exist = false;
        int tmp_current = m_history_current;
        for( int i = 0; i < MAX_LOCAL_HISTORY; ++i ){
            if( m_items[ tmp_current ]->url == url ){
                exist = true;
                break;
            }
            if( tmp_current == m_history_end ) break;
            tmp_current = ( tmp_current + MAX_LOCAL_HISTORY - 1 ) % MAX_LOCAL_HISTORY;
        }

        // 登録されている場合は前に詰める
        if( exist ){

#ifdef _DEBUG
            std::cout << "exist\n";
#endif

            ViewHistoryItem* item = m_items[ tmp_current ];

            for( int i = 0; i < MAX_LOCAL_HISTORY; ++i ){
                if( tmp_current == m_history_current ) break;
                int next = ( tmp_current + 1 ) % MAX_LOCAL_HISTORY;
#ifdef _DEBUG
                std::cout << "current = " << tmp_current << " next = " << next << std::endl;
#endif
                m_items[ tmp_current ] = m_items[ next ];
                tmp_current = next;
            }

            m_items[ tmp_current ] = item;
            m_history_top = m_history_current;
        }

        // 新しく追加
        else{

#ifdef _DEBUG
            std::cout << "append\n";
#endif

            bool pback = ( m_items.size() < MAX_LOCAL_HISTORY
                           && m_history_top == ( int )m_items.size() -1
                           && m_history_top == m_history_current
                );

            m_history_current = ( m_history_current + 1) % MAX_LOCAL_HISTORY;
    
            if( pback ){
#ifdef _DEBUG
                std::cout << "push_back\n";
#endif
                ViewHistoryItem *item = new ViewHistoryItem();
                item->url = url;
                m_items.push_back( item );
            }
            else m_items[ m_history_current ]->url = url;

            m_history_top = m_history_current;
            if( m_history_top == m_history_end ) m_history_end = ( m_history_end + 1 ) % MAX_LOCAL_HISTORY;
        }
    }

#ifdef _DEBUG
    std::cout << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl
              << "cur = " << get_current_url() << std::endl
              << "title = " << get_current_title() << std::endl;

    assert( get_current_url() == url );
#endif
}


//
// 戻る
//
// exec = true のときは履歴の位置を変更する
// false の時はURLの取得のみ
//
const ViewHistoryItem* ViewHistory::back( const int count, const bool exec )
{
    return back_forward( true, count, exec );
}


//
// 進む
//
// exec = true のときは履歴の位置を変更する
// false の時はURLの取得のみ
//
const ViewHistoryItem* ViewHistory::forward( const int count, const bool exec )
{
    return back_forward( false, count, exec );
}


//
// 戻る / 進む
//
const ViewHistoryItem* ViewHistory::back_forward( const bool back, const int count, const bool exec )
{
#ifdef _DEBUG
    std::cout << "ViewHistory::back_forward count = " << count << " back = " << back << " exec = " << exec << std::endl;
#endif

    int tmp_current = m_history_current;

    if( back ){
        if( ! can_back( count ) ) return NULL;
        tmp_current = ( tmp_current + MAX_LOCAL_HISTORY - count ) % MAX_LOCAL_HISTORY;
    }
    else{
        if( ! can_forward( count ) )  return NULL;
        tmp_current = ( tmp_current + count ) % MAX_LOCAL_HISTORY;
    }

    // 更新
    if( exec ) m_history_current = tmp_current;

#ifdef _DEBUG
    std::cout << "size = " << m_items.size() << std::endl
              << "top = " << m_history_top << std::endl
              << "cur = " << m_history_current << std::endl
              << "end = " << m_history_end << std::endl
              << "ret.url = " << m_items[ tmp_current ]->url << std::endl
              << "ret.title = " << m_items[ tmp_current ]->title << std::endl;
#endif

    return m_items[ tmp_current ];
}


