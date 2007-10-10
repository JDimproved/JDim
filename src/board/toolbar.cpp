// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "icons/iconmanager.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BOARD;


BoardToolBar::BoardToolBar( bool show_bar )
    : m_toolbar_shown( false ),
      m_button_close( Gtk::Stock::CLOSE ),
      m_button_reload( Gtk::Stock::REFRESH ),
      m_button_delete( Gtk::Stock::DELETE ),
      m_button_stop( Gtk::Stock::STOP ),
      m_button_favorite( Gtk::Stock::COPY ),
      m_button_up_search( Gtk::Stock::GO_UP ),
      m_button_down_search( Gtk::Stock::GO_DOWN ),
      m_button_new_article( ICON::WRITE )
{
    m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
    m_tooltip.set_tip( m_button_reload, CONTROL::get_label_motion( CONTROL::Reload ) );
    m_tooltip.set_tip( m_button_delete, CONTROL::get_label_motion( CONTROL::Delete ) );
    m_tooltip.set_tip( m_button_stop, CONTROL::get_label_motion( CONTROL::StopLoading ) );
    m_tooltip.set_tip( m_button_favorite, CONTROL::get_label_motion( CONTROL::AppendFavorite )
                       + "\n\nまたは板のタブか選択したスレをお気に入りに直接Ｄ＆Ｄする" );
    m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
    m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
    m_tooltip.set_tip( m_button_new_article, CONTROL::get_label_motion( CONTROL::NewArticle ) );
        
    int num = 0;
    for(;;){
        int item = SESSION::get_item_board_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_NEWARTICLE: m_buttonbar.pack_start( m_button_new_article, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCHBOX: m_buttonbar.pack_start( m_entry_search, Gtk::PACK_EXPAND_WIDGET ); break;
            case ITEM_SEARCH_NEXT: m_buttonbar.pack_start( m_button_down_search, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCH_PREV: m_buttonbar.pack_start( m_button_up_search, Gtk::PACK_SHRINK ); break;
            case ITEM_RELOAD: m_buttonbar.pack_start( m_button_reload, Gtk::PACK_SHRINK ); break;
            case ITEM_STOPLOADING: m_buttonbar.pack_start( m_button_stop, Gtk::PACK_SHRINK ); break;
            case ITEM_FAVORITE: m_buttonbar.pack_start( m_button_favorite, Gtk::PACK_SHRINK ); break;
            case ITEM_DELETE: m_buttonbar.pack_start( m_button_delete, Gtk::PACK_SHRINK ); break;
            case ITEM_QUIT: m_buttonbar.pack_start( m_button_close, Gtk::PACK_SHRINK ); break;
        }
        ++num;
    }

    m_entry_search.add_mode( CONTROL::MODE_BOARD );

    m_buttonbar.set_border_width( 1 );
    m_scrwin.add( m_buttonbar );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    set_size_request( 8 );
    if( show_bar ) show_toolbar();
}


// ツールバーを表示
void BoardToolBar::show_toolbar()
{
    if( ! m_toolbar_shown ){
        pack_start( m_scrwin, Gtk::PACK_SHRINK );
        show_all_children();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void BoardToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){
        remove( m_scrwin );
        show_all_children();
        m_toolbar_shown = false;
    }
}


// タブのロック
void BoardToolBar::lock()
{
    m_button_close.set_sensitive( false );
}


// タブのアンロック
void BoardToolBar::unlock()
{
    m_button_close.set_sensitive( true );
}
