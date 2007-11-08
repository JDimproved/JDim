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


BoardToolBar::BoardToolBar() :
    SKELETON::ToolBar(),
      m_button_reload( Gtk::Stock::REFRESH ),
      m_button_delete( Gtk::Stock::DELETE ),
      m_button_stop( Gtk::Stock::STOP ),
      m_button_favorite( Gtk::Stock::COPY ),
      m_button_up_search( Gtk::Stock::GO_UP ),
      m_button_down_search( Gtk::Stock::GO_DOWN ),
      m_button_new_article( ICON::WRITE )
{
    set_tooltip( get_close_button(), CONTROL::get_label_motion( CONTROL::Quit ) );
    set_tooltip( m_button_reload, CONTROL::get_label_motion( CONTROL::Reload ) );
    set_tooltip( m_button_delete, CONTROL::get_label_motion( CONTROL::Delete ) );
    set_tooltip( m_button_stop, CONTROL::get_label_motion( CONTROL::StopLoading ) );
    set_tooltip( m_button_favorite, CONTROL::get_label_motion( CONTROL::AppendFavorite )
                       + "\n\nスレ一覧のタブか選択したスレをお気に入りに直接Ｄ＆Ｄしても登録可能" );
    set_tooltip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
    set_tooltip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
    set_tooltip( m_button_new_article, CONTROL::get_label_motion( CONTROL::NewArticle ) );
        
    m_entry_search.add_mode( CONTROL::MODE_BOARD );

    pack_buttons();
}

// ボタンのパッキング
void BoardToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_board_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_NEWARTICLE: get_buttonbar().pack_start( m_button_new_article, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCHBOX: get_buttonbar().pack_start( m_entry_search, Gtk::PACK_EXPAND_WIDGET ); break;
            case ITEM_SEARCH_NEXT: get_buttonbar().pack_start( m_button_down_search, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCH_PREV: get_buttonbar().pack_start( m_button_up_search, Gtk::PACK_SHRINK ); break;
            case ITEM_RELOAD: get_buttonbar().pack_start( m_button_reload, Gtk::PACK_SHRINK ); break;
            case ITEM_STOPLOADING: get_buttonbar().pack_start( m_button_stop, Gtk::PACK_SHRINK ); break;
            case ITEM_FAVORITE: get_buttonbar().pack_start( m_button_favorite, Gtk::PACK_SHRINK ); break;
            case ITEM_DELETE: get_buttonbar().pack_start( m_button_delete, Gtk::PACK_SHRINK ); break;
            case ITEM_QUIT: get_buttonbar().pack_start( get_close_button(), Gtk::PACK_SHRINK ); break;
            case ITEM_SEPARATOR: pack_separator(); break;
        }
        ++num;
    }

    show_all_children();
}    
