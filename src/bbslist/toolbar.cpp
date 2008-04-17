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
    SKELETON::ToolBar( BBSLIST::get_admin() )
{
    set_tooltip( m_button_toggle, "板一覧とお気に入りの切り替え" );

    std::vector< std::string > menu;
    menu.push_back( "板一覧" );
    menu.push_back( "お気に入り" );
    m_button_toggle.append_menu( menu );
    m_button_toggle.signal_selected().connect( sigc::mem_fun(*this, &BBSListToolBar::slot_toggle ) );

    m_hbox_label.pack_start( *get_label(), Gtk::PACK_EXPAND_WIDGET, 4 );
    m_hbox_label.pack_start( m_button_toggle, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( *get_button_close(), Gtk::PACK_SHRINK );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );

    pack_buttons();
    get_entry_search()->add_mode( CONTROL::MODE_BBSLIST );
}


//
// ボタンのパッキング
//
// virtual
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

    set_relief();
    show_all_children();
}


void BBSListToolBar::slot_toggle( int i )
{
#ifdef _DEBUG 	 
     std::cout << "BBSListToolBar::slot_toggle = " << get_url() << " i = " << i << std::endl;
#endif 	 
  	 
     switch( i ){
  	 
         case 0:
             if( get_url() != URL_BBSLISTVIEW ) CORE::core_set_command( "switch_sidebar", URL_BBSLISTVIEW ); 	 
             break; 	 
  	 
         case 1:
             if( get_url() != URL_FAVORITEVIEW ) CORE::core_set_command( "switch_sidebar", URL_FAVORITEVIEW ); 	 
             break; 	 
     }
}
