// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "articleadmin.h"
#include "articleviewbase.h"

#include "skeleton/compentry.h"

#include "dbtree/interface.h"

#include "command.h"
#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace ARTICLE;


ArticleToolBar::ArticleToolBar() :
    SKELETON::ToolBar( ARTICLE::get_admin() ),
    m_button_board( NULL ),

    m_button_drawout_and( Gtk::Stock::CUT ),
    m_button_drawout_or( Gtk::Stock::ADD ),
    m_button_clear_hl( Gtk::Stock::CLEAR )
{
    // 検索バー
    set_tooltip( m_button_drawout_and, CONTROL::get_label_motion( CONTROL::DrawOutAnd ) );
    set_tooltip( m_button_drawout_or, CONTROL::get_label_motion( CONTROL::DrawOutOr ) );
    set_tooltip( m_button_clear_hl, CONTROL::get_label_motion( CONTROL::HiLightOff ) );

    get_searchbar()->pack_start( *get_entry_search(), Gtk::PACK_EXPAND_WIDGET );
    get_searchbar()->pack_end( *get_button_close_searchbar(), Gtk::PACK_SHRINK );
    get_searchbar()->pack_end( m_button_clear_hl, Gtk::PACK_SHRINK );
    get_searchbar()->pack_end( m_button_drawout_or, Gtk::PACK_SHRINK );
    get_searchbar()->pack_end( m_button_drawout_and, Gtk::PACK_SHRINK );
    get_searchbar()->pack_end( *get_button_up_search(), Gtk::PACK_SHRINK );
    get_searchbar()->pack_end( *get_button_down_search(), Gtk::PACK_SHRINK );

    get_entry_search()->add_mode( CONTROL::MODE_COMMON );

    m_button_clear_hl.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_clear_highlight ) );
    m_button_drawout_or.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_or ) );
    m_button_drawout_and.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_and ) );

    pack_buttons();
}
        

//
// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
//
// virtual
void ArticleToolBar::set_view( SKELETON::View * view )
{
    SKELETON::ToolBar::set_view( view );

    // ArticleViewBase固有の情報をコピー
    ArticleViewBase* articleview = dynamic_cast< ArticleViewBase* >( view );
    if( articleview ){
        m_url_article = articleview->url_article();
        if( m_button_board ) m_button_board->set_label( articleview->get_label_board() );
    }
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
                get_buttonbar().pack_start( *get_button_write(), Gtk::PACK_SHRINK );
                break;

            case ITEM_OPENBOARD:
                if( ! m_button_board ){
                    m_button_board = Gtk::manage( new Gtk::Button() );
                    m_button_board->set_focus_on_click( false );
                    set_tooltip( *m_button_board, CONTROL::get_label_motion( CONTROL::OpenParentBoard ) );
                    m_button_board->signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_open_board ) );
                }
                get_buttonbar().pack_start( *m_button_board, Gtk::PACK_SHRINK );
                break;

            case ITEM_NAME:
                get_buttonbar().pack_start( *get_label(), Gtk::PACK_EXPAND_WIDGET, 4 );
                break;

            case ITEM_SEARCH:
                get_buttonbar().pack_start( *get_button_open_searchbar(), Gtk::PACK_SHRINK );
                break;

            case ITEM_RELOAD:
                get_buttonbar().pack_start( *get_button_reload(), Gtk::PACK_SHRINK );
                break;

            case ITEM_STOPLOADING:
                get_buttonbar().pack_start( *get_button_stop(), Gtk::PACK_SHRINK );
                break;

            case ITEM_FAVORITE:
                get_buttonbar().pack_start( *get_button_favorite(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motion( CONTROL::AppendFavorite )
                             + "...\n\nスレのタブをお気に入りに直接Ｄ＆Ｄしても登録可能" );
                break;

            case ITEM_DELETE:
                get_buttonbar().pack_start( *get_button_delete(), Gtk::PACK_SHRINK );
                break;

            case ITEM_QUIT:
                get_buttonbar().pack_start( *get_button_close(), Gtk::PACK_SHRINK );
                break;

            case ITEM_PREVVIEW:
                get_buttonbar().pack_start( *get_button_back(), Gtk::PACK_SHRINK );
                break;

            case ITEM_NEXTVIEW:
                get_buttonbar().pack_start( *get_button_forward(), Gtk::PACK_SHRINK );
                break;

            case ITEM_LOCK:
                get_buttonbar().pack_start( *get_button_lock(), Gtk::PACK_SHRINK );
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
    std::string query = get_entry_search()->get_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
}


//
// キーワード抽出 (OR)
//
void ArticleToolBar::slot_drawout_or()
{
    std::string query = get_entry_search()->get_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "true" );
}


//
// ハイライト解除
//
void ArticleToolBar::slot_clear_highlight()
{
    ARTICLE::get_admin()->set_command( "clear_highlight", get_url() );
}


//
// 板を開くボタン
//
void ArticleToolBar::slot_open_board()
{
    CORE::core_set_command( "open_board", DBTREE::url_subject( m_url_article ), "true",
                            "auto" // オートモードで開く
        );
}
