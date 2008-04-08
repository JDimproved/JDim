// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "bbslistadmin.h"

#include "skeleton/compentry.h"

#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace BBSLIST;


BBSListToolBar::BBSListToolBar() :
    SKELETON::ToolBar( BBSLIST::get_admin() )
{
    // TODO : class arrowbutton を作る
    Gtk::Arrow* arrow =  Gtk::manage( new Gtk::Arrow( Gtk::ARROW_DOWN, Gtk::SHADOW_NONE ) );
    m_button_toggle.add( *arrow );
    m_button_toggle.set_focus_on_click( false );
    m_button_toggle.set_relief( Gtk:: RELIEF_NONE );
    set_tooltip( m_button_toggle, "板一覧とお気に入りの切り替え(未実装)" );

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
