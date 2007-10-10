// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BBSLIST;


BBSListtToolBar::BBSListtToolBar( bool show_bar ) :
    m_toolbar_shown( false ),
    m_button_close( Gtk::Stock::CLOSE ),
    m_button_up_search( Gtk::Stock::GO_UP ),
    m_button_down_search( Gtk::Stock::GO_DOWN )
{
    // ラベルバー
    m_combo.append_text( "板一覧" );
    m_combo.append_text( "お気に入り" );

    m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
    m_hbox_label.pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 2 );
    m_hbox_label.pack_start( m_button_close, Gtk::PACK_SHRINK );

    // 検索バー
    m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
    m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );

    // ボタンやラベルのバー
    int num = 0;
    for(;;){
        int item = SESSION::get_item_sidebar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_SEARCHBOX: m_hbox_search.pack_start( m_entry_search, Gtk::PACK_EXPAND_WIDGET, 2 ); break;
            case ITEM_SEARCH_NEXT: m_hbox_search.pack_start( m_button_down_search, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCH_PREV: m_hbox_search.pack_start( m_button_up_search, Gtk::PACK_SHRINK ); break;
        }
        ++num;
    }

    m_entry_search.add_mode( CONTROL::MODE_BBSLIST );

    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    if( show_bar ) show_toolbar();
}


// ツールバーを表示
void BBSListtToolBar::show_toolbar()
{
    if( ! m_toolbar_shown ){
        pack_start( m_hbox_search, Gtk::PACK_SHRINK );
        show_all_children();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void BBSListtToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){
        remove( m_hbox_search );
        show_all_children();
        m_toolbar_shown = false;
    }
}


void BBSListtToolBar::remove_label()
{
    remove( m_hbox_label );
}


