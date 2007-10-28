// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BBSLIST;


BBSListToolBar::BBSListToolBar() :
    SKELETON::ToolBar(),
    m_toolbar_shown( false ),
    m_button_up_search( Gtk::Stock::GO_UP ),
    m_button_down_search( Gtk::Stock::GO_DOWN )
{
    // 検索バー
    set_tooltip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
    set_tooltip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
    m_hbox_label.pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 2 );
    m_hbox_label.pack_start( get_close_button(), Gtk::PACK_SHRINK );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );

    // ラベルバー
    m_combo.append_text( "板一覧" );
    m_combo.append_text( "お気に入り" );
    set_tooltip( get_close_button(), CONTROL::get_label_motion( CONTROL::Quit ) );
    pack_buttons();
    m_entry_search.add_mode( CONTROL::MODE_BBSLIST );
}


//
// ボタンのパッキング
//
void BBSListToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_sidebar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_SEARCHBOX: get_buttonbar().pack_start( m_entry_search, Gtk::PACK_EXPAND_WIDGET, 2 ); break;
            case ITEM_SEARCH_NEXT: get_buttonbar().pack_start( m_button_down_search, Gtk::PACK_SHRINK ); break;
            case ITEM_SEARCH_PREV: get_buttonbar().pack_start( m_button_up_search, Gtk::PACK_SHRINK ); break;
            case ITEM_SEPARATOR: pack_separator(); break;
        }
        ++num;
    }
}

void BBSListToolBar::remove_label()
{
    remove( m_hbox_label );
}


