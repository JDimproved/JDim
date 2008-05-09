// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "articleadmin.h"
#include "articleviewbase.h"

#include "skeleton/imgtoggletoolbutton.h"

#include "command.h"
#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace ARTICLE;


ArticleToolBar::ArticleToolBar() :
    SKELETON::ToolBar( ARTICLE::get_admin() ),
    m_enable_slot( true ),

    m_button_drawout_and( Gtk::Stock::CUT ),
    m_button_drawout_or( Gtk::Stock::ADD ),
    m_button_clear_hl( Gtk::Stock::CLEAR ),

    m_button_live_play_stop( NULL )
{
    // 検索バー
    set_tooltip( m_button_drawout_and, CONTROL::get_label_motion( CONTROL::DrawOutAnd ) );
    set_tooltip( m_button_drawout_or, CONTROL::get_label_motion( CONTROL::DrawOutOr ) );
    set_tooltip( m_button_clear_hl, CONTROL::get_label_motion( CONTROL::HiLightOff ) );

    get_searchbar()->append( *get_entry_search() );
    get_searchbar()->append( *get_button_down_search() );
    get_searchbar()->append( *get_button_up_search() );
    get_searchbar()->append( m_button_drawout_and );
    get_searchbar()->append( m_button_drawout_or );
    get_searchbar()->append( m_button_clear_hl );
    get_searchbar()->append( *get_button_close_searchbar() );

    m_button_clear_hl.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_clear_highlight ) );
    m_button_drawout_or.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_or ) );
    m_button_drawout_and.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_and ) );

    pack_buttons();
    add_search_mode( CONTROL::MODE_COMMON );
}
        

//
// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
//
// virtual
void ArticleToolBar::set_view( SKELETON::View * view )
{
    SKELETON::ToolBar::set_view( view );

    m_enable_slot = false;

    // ArticleViewBase固有の情報をコピー
    ArticleViewBase* articleview = dynamic_cast< ArticleViewBase* >( view );
    if( articleview ){

        m_url_article = articleview->url_article();

        if( m_button_live_play_stop ){
            bool sensitive = true;
            if( ! articleview->get_enable_live() ) sensitive = false;
            m_button_live_play_stop->set_sensitive( sensitive );

            if( articleview->get_live() ) m_button_live_play_stop->set_active( true );
            else m_button_live_play_stop->set_active( false );
        }
    }

    m_enable_slot = true;
}


//
// ボタンのパッキング
//
// virtual
void ArticleToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_article_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_WRITEMSG:
                get_buttonbar().append( *get_button_write() );
                break;

            case ITEM_OPENBOARD:
                get_buttonbar().append( *get_button_board() );
                break;

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
                break;

            case ITEM_STOPLOADING:
                get_buttonbar().append( *get_button_stop() );
                break;

            case ITEM_FAVORITE:
                get_buttonbar().append( *get_button_favorite() );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motion( CONTROL::AppendFavorite )
                             + "...\n\nスレのタブをお気に入りに直接Ｄ＆Ｄしても登録可能" );

                break;

            case ITEM_DELETE:
                get_buttonbar().append( *get_button_delete() );

                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                break;

            case ITEM_PREVVIEW:
                get_buttonbar().append( *get_button_back() );
                break;

            case ITEM_NEXTVIEW:
                get_buttonbar().append( *get_button_forward() );
                break;

            case ITEM_LOCK:
                get_buttonbar().append( *get_button_lock() );
                break;

            case ITEM_LIVE:
                if( ! m_button_live_play_stop ){
                    m_button_live_play_stop = Gtk::manage( new SKELETON::ImgToggleToolButton( Gtk::Stock::MEDIA_PLAY ) );
                    set_tooltip( *m_button_live_play_stop, CONTROL::get_label_motion( CONTROL::LiveStartStop ) );
                    m_button_live_play_stop->set_label( CONTROL::get_label( CONTROL::LiveStartStop ) );
                    m_button_live_play_stop->signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_live_play_stop ) );
                }
                get_buttonbar().append( *m_button_live_play_stop );

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
// キーワード抽出 (AND)
//
void ArticleToolBar::slot_drawout_and()
{
    if( ! m_enable_slot ) return;    

    std::string query = get_search_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
}


//
// キーワード抽出 (OR)
//
void ArticleToolBar::slot_drawout_or()
{
    if( ! m_enable_slot ) return;    

    std::string query = get_search_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "true" );
}


//
// ハイライト解除
//
void ArticleToolBar::slot_clear_highlight()
{
    if( ! m_enable_slot ) return;    

    ARTICLE::get_admin()->set_command( "clear_highlight", get_url() );
}


//
// 実況開始/停止
//
void ArticleToolBar::slot_live_play_stop()
{
    if( ! m_enable_slot ) return;    

    ARTICLE::get_admin()->set_command( "live_start_stop", get_url() );
}
