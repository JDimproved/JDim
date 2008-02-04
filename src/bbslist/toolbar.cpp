// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "bbslistadmin.h"

#include "skeleton/compentry.h"

#include "command.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BBSLIST;


BBSListToolBar::BBSListToolBar() :
    SKELETON::ToolBar( BBSLIST::get_admin() ), m_enable_slot( true )
{
    m_hbox_label.pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 2 );
    m_hbox_label.pack_start( *get_button_close(), Gtk::PACK_SHRINK );

    m_combo.append_text( "板一覧" );
    m_combo.append_text( "お気に入り" );
    m_combo.signal_changed().connect( sigc::mem_fun( *this, &BBSListToolBar::slot_combo_changed ) );

    pack_start( m_hbox_label, Gtk::PACK_SHRINK );

    pack_buttons();
    get_entry_search()->add_mode( CONTROL::MODE_BBSLIST );
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

            case ITEM_SEARCHBOX:
                get_buttonbar().pack_start( *get_entry_search(), Gtk::PACK_EXPAND_WIDGET, 2 );
                break;

            case ITEM_SEARCH_NEXT:
                get_buttonbar().pack_start( *get_button_down_search(), Gtk::PACK_SHRINK );
                break;

            case ITEM_SEARCH_PREV:
                get_buttonbar().pack_start( *get_button_up_search(), Gtk::PACK_SHRINK );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    show_all_children();
}


void BBSListToolBar::set_combo( int page )
{
#ifdef _DEBUG
    std::cout << "BBSListToolBar::set_combo page = " << page << std::endl;
#endif

    m_enable_slot = false;
    m_combo.set_active( page );
    m_enable_slot = true;
}


// ラベルのコンボボックスの表示が変わった
void BBSListToolBar::slot_combo_changed()
{
    if( ! m_enable_slot ) return;

#ifdef _DEBUG
    std::cout << "BBSListToolBar::slot_combo_changed url = " << get_url() << std::endl
              << "combo = " << m_combo.get_active_row_number() << std::endl;
#endif

    switch( m_combo.get_active_row_number() ){

        case COMBO_BBSLIST:
            if( get_url() != URL_BBSLISTVIEW ) CORE::core_set_command( "switch_sidebar", URL_BBSLISTVIEW );
            break;

        case COMBO_FAVORITE:
            if( get_url() != URL_FAVORITEVIEW ) CORE::core_set_command( "switch_sidebar", URL_FAVORITEVIEW );
            break;
    }
}

