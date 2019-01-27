// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "boardviewlog.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "searchmanager.h"
#include "session.h"
#include "global.h"

using namespace BOARD;


BoardViewLog::BoardViewLog( const std::string& url )
    : BoardViewBase( url, ( url == URL_ALLLOG ) )
{
    set_writeable( false );
    set_load_subject_txt( false );

    CORE::get_search_manager()->sig_search_fin().connect( sigc::mem_fun( *this, &BoardViewLog::slot_search_fin ) );

#ifdef _DEBUG
    std::cout << "BoardViewLog::BoardViewLog : url = " << get_url() << std::endl;
#endif
}


BoardViewLog::~BoardViewLog()
{
#ifdef _DEBUG
    std::cout << "BoardViewLog::~BoardViewLog : url = " << get_url() << std::endl;
#endif
}


//
// 検索停止
//
void BoardViewLog::stop()
{
    CORE::get_search_manager()->stop( get_url() );
}


//
// リロード
//
void BoardViewLog::reload()
{
    if( CORE::get_search_manager()->is_searching() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "他の検索スレッドが実行中です" );
        mdiag.run();
        return;
    }

    show_view();
}



//
// ビュー表示
//
void BoardViewLog::show_view()
{
#ifdef _DEBUG
    std::cout << "BoardViewLog::show_view " << get_url() << std::endl;
#endif

    if( ! SESSION::is_booting() ){

        const std::string id = get_url();
        int searchmode = CORE::SEARCHMODE_LOG;
        if( get_url() == URL_ALLLOG ) searchmode = CORE::SEARCHMODE_ALLLOG;
        const bool mode_or = false;
        const bool bm = false;
        const bool calc_data = false;

        CORE::get_search_manager()->search( id, searchmode, get_url_board(),
                                            "", mode_or, bm, calc_data );
    }

    BoardViewBase::show_view();
}


//
// 検索終了
//
void BoardViewLog::slot_search_fin( const std::string& id )
{
    if( id != get_url() ) return;

#ifdef _DEBUG
    std::cout << "BoardViewLog::slot_search_fin size = " << CORE::get_search_manager()->get_list_article().size() << std::endl;
#endif

    const bool loading_fin = true;
    const std::vector< DBTREE::ArticleBase* >& list_article = CORE::get_search_manager()->get_list_article();
    update_view_impl( list_article, loading_fin );

    m_set_thread.clear();
    for( size_t i = 0; i < list_article.size(); ++i ){

        const DBTREE::ArticleBase* art = list_article[ i ];
        m_set_thread.insert( art->get_url() );
    }
}


void BoardViewLog::slot_abone_thread()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "ログ一覧ではあぼ〜ん出来ません" );
    mdiag.run();
    return;
}


//
// 板名更新
//
void BoardViewLog::update_boardname()
{
    std::string title;
    if( get_url() == URL_ALLLOG ) title = "[ 全ログ一覧 ]";
    else if( ! get_url_board().empty() ) title = "[ ログ一覧 ] - " + DBTREE::board_name( get_url_board() );

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
void BoardViewLog::update_item( const std::string& url, const std::string& id )
{
    // url が URL_ALLLOG の時は get_url_board() の戻り値は empty
    if( get_url() != URL_ALLLOG && get_url_board() != url ) return;

    if( CORE::get_search_manager()->is_searching( get_url() ) ) return;

    const std::string url_dat = DBTREE::url_datbase( url ) + id;

    Gtk::TreeModel::Row row;
    if( ! id.empty() && m_set_thread.find_if( url_dat ) ) row = get_row_from_url( url_dat );

#ifdef _DEBUG
    std::cout << "BoardViewLog::update_item " << get_url() << std::endl
              << "url = " << url << " id = " << id << " url_dat = " << url_dat << std::endl;
#endif

    if( id.empty() || row ) BoardViewBase::update_item( url, id );

    // もし row が無く、かつキャッシュがあるならば行を追加
    else if( ! id.empty() ){ 

        DBTREE::ArticleBase* art = DBTREE::get_article( url_dat );
        if( art && art->is_cached() ){

#ifdef _DEBUG
            std::cout << "prepend\n";
#endif

            unsorted_column();
            prepend_row( art, get_row_size() + 1 );
            m_set_thread.insert( art->get_url() );
            goto_top();
            update_status();
        }
    }
}


//
// デフォルトのソート状態
//
int BoardViewLog::get_default_sort_column()
{
    if( get_url() != URL_ALLLOG ) return BoardViewBase::get_default_sort_column();

    return COL_BOARD;
}

int BoardViewLog::get_default_view_sort_mode()
{
    if( get_url() != URL_ALLLOG ) return BoardViewBase::get_default_view_sort_mode();

    return SORTMODE_ASCEND;
}

int BoardViewLog::get_default_view_sort_pre_column()
{
    if( get_url() != URL_ALLLOG ) return BoardViewBase::get_default_view_sort_pre_column();

    return COL_ID;
}

int BoardViewLog::get_default_view_sort_pre_mode()
{
    if( get_url() != URL_ALLLOG ) return BoardViewBase::get_default_view_sort_pre_mode();

    return SORTMODE_ASCEND;
}
