// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "boardviewsidebar.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "history/historymanager.h"

#include "global.h"
#include "session.h"
#include "updatemanager.h"

#include <cstring>

using namespace BOARD;


BoardViewSidebar::BoardViewSidebar( const std::string& url, const bool set_history  )
    : BoardViewBase( url, true ),
      m_set_history( set_history )
{
    m_sidebar_url = url.substr( 0, url.find( SIDEBAR_SIGN ) );
    m_dirid = atoi( url.substr( url.find( SIDEBAR_SIGN ) + strlen( SIDEBAR_SIGN ) ).c_str() );

    set_writeable( false );
    set_load_subject_txt( false );

#ifdef _DEBUG
    std::cout << "BoardViewSidebar::BoardViewSidebar : sidebar_url = " << m_sidebar_url
              << " dirid = " << m_dirid << " url = " << get_url() << std::endl;
#endif
}


BoardViewSidebar::~BoardViewSidebar()
{
#ifdef _DEBUG
    std::cout << "BoardViewSidebar::~BoardViewSidebar : url = " << get_url() << std::endl;
#endif
}


//
// リロード
//
void BoardViewSidebar::reload()
{
    show_view();
}


//
// ビュー表示
//
void BoardViewSidebar::show_view()
{
#ifdef _DEBUG
    std::cout << "BoardViewSidebar::show_view " << get_url() << std::endl;
#endif

    BoardViewBase::show_view();

    std::vector< std::string > list_url;
    SESSION::get_sidebar_threads( m_sidebar_url, m_dirid, list_url );
    if( list_url.empty() ) return;

    std::vector< DBTREE::ArticleBase* > list_article;
    m_set_thread.clear();
    for( size_t i = 0; i < list_url.size(); ++i ){

        DBTREE::ArticleBase* art = DBTREE::get_article( list_url[ i ] );
        if( art ){
            list_article.push_back( art );
            m_set_thread.insert( art->get_url() );

            if( SESSION::is_online() ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( art->get_url() ), false );
        }
    }
    
    const bool loading_fin = true;
    update_view_impl( list_article, loading_fin );

    // 板の履歴に登録
    if( m_set_history ){

        HISTORY::append_history( URL_HISTBOARDVIEW,
                                 get_url(),
                                 get_title(), TYPE_VBOARD );
    }

    // 更新チェック
    if( SESSION::is_online() ) CORE::get_checkupdate_manager()->run();
}


void BoardViewSidebar::slot_abone_thread()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "お気に入り一覧ではあぼ〜ん出来ません" );
    mdiag.run();
    return;
}


//
// 板名更新
//
void BoardViewSidebar::update_boardname()
{
    const std::string title = SESSION::get_sidebar_dirname( m_sidebar_url, m_dirid );

    // ウィンドウタイトル表示
    set_title( title );
    BOARD::get_admin()->set_command( "set_title", get_url(), get_title() );

    // タブに名前をセット
    BOARD::get_admin()->set_command( "set_tablabel", get_url(), title );
}


//
// 特定の行だけの表示内容更新
//
// url : subject.txt のアドレス
// id : DAT の ID(拡張子付き), empty なら全ての行の表示内容を更新する
//
void BoardViewSidebar::update_item( const std::string& url, const std::string& id )
{
    const std::string url_dat = DBTREE::url_datbase( url ) + id;

    if( id.empty() || m_set_thread.find_if( url_dat ) ){

#ifdef _DEBUG
        std::cout << "BoardViewSidebar::update_item " << get_url() << std::endl
                  << "url = " << url << " id = " << id << " url_dat = " << url_dat << std::endl;
#endif

        BoardViewBase::update_item( url, id );
    }
}



//
// デフォルトのソート状態
//
int BoardViewSidebar::get_default_sort_column()
{
    return COL_ID;
}

int BoardViewSidebar::get_default_view_sort_mode()
{
    return SORTMODE_ASCEND;
}

int BoardViewSidebar::get_default_view_sort_pre_column()
{
    return COL_ID;
}

int BoardViewSidebar::get_default_view_sort_pre_mode()
{
    return SORTMODE_ASCEND;
}
