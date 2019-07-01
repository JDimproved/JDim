// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarsearch.h"
#include "articleadmin.h"
#include "articleviewsearch.h"

#include "skeleton/compentry.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "compmanager.h"
#include "session.h"
#include "global.h"

using namespace ARTICLE;

SearchToolBar::SearchToolBar() :
    SKELETON::ToolBar( ARTICLE::get_admin() ),
    m_searchview( NULL ),
    m_check_bm( "しおり" )
{
    m_tool_bm.add( m_check_bm );
    m_tool_bm.set_expand( false );

    m_check_bm.signal_toggled().connect( sigc::mem_fun(*this, &SearchToolBar::slot_toggle_bm ) );

    // 検索バー    
    get_searchbar()->append( *get_tool_search( CORE::COMP_SEARCH_ARTICLE ) );
    get_searchbar()->append( m_tool_bm );
    get_searchbar()->append( *get_button_close_searchbar() );

    pack_buttons();
}


SearchToolBar::~SearchToolBar() noexcept = default;


//
// ボタンのパッキング
//
void SearchToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_search_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_NAME:
                pack_transparent_separator();
                get_buttonbar().append( *get_label() );
                pack_transparent_separator();
                break;

            case ITEM_SEARCH:
                get_buttonbar().append( *get_button_open_searchbar() );
                break;

            case ITEM_RELOAD:
                get_buttonbar().append( *get_button_reload() );
                set_tooltip( *get_button_reload(), "再検索 " + CONTROL::get_str_motions( CONTROL::Reload ) );
                break;

            case ITEM_STOPLOADING:
                get_buttonbar().append( *get_button_stop() );
                set_tooltip( *get_button_stop(), "検索中止 " + CONTROL::get_str_motions( CONTROL::StopLoading ) );
                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    set_relief();
    show_all_children();
}


//
// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
//
// virtual
void SearchToolBar::set_view( SKELETON::View * view )
{
    SKELETON::ToolBar::set_view( view );

    m_enable_slot = false;

    // ArticleViewSearch固有の情報をコピー
    m_searchview = dynamic_cast< ArticleViewSearch* >( view );
    if( m_searchview ){

        if( m_searchview->get_enable_bm() ){

            m_check_bm.set_sensitive( true );
            m_check_bm.set_active( m_searchview->get_bm() );
        }
        else{
            m_check_bm.set_sensitive( false );
            m_check_bm.set_active( false );
        }
    }

    m_enable_slot = true;
}


void SearchToolBar::slot_toggle_bm()
{
    if( ! m_enable_slot ) return;

    if( m_searchview ) m_searchview->set_bm( m_check_bm.get_active() );
}
