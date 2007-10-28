// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "maintoolbar.h"

#include "config/globalconf.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace CORE;


MainToolBar::MainToolBar() :
    SKELETON::ToolBar(),
      m_button_go( Gtk::Stock::JUMP_TO ),

      m_button_bbslist( ICON::DIR ),
      m_button_favorite( ICON::FAVORITE ),
      m_button_board( ICON::BOARD ),
      m_button_thread( ICON::THREAD ),
      m_button_image( ICON::IMAGE )
{
    set_tooltip( m_button_go, ITEM_NAME_GO );

    set_tooltip( m_button_bbslist, std::string( ITEM_NAME_BBSLISTVIEW )
                 + "\n\nお気に入りに切替え " + CONTROL::get_motion( CONTROL::TabRight ) );
    set_tooltip( m_button_favorite, std::string( ITEM_NAME_FAVORITEVIEW )
                 + "\n\n板一覧に切替え " + CONTROL::get_motion( CONTROL::TabLeft ) );
    set_tooltip( m_button_board, std::string( ITEM_NAME_BOARDVIEW )
                 + "\n\n" + CONTROL::get_label_motion( CONTROL::ToggleArticle ) );
    set_tooltip( m_button_thread, std::string( ITEM_NAME_ARTICLEVIEW )
                 + "\n\n" + CONTROL::get_label_motion( CONTROL::ToggleArticle ) );
    set_tooltip( m_button_image, std::string( ITEM_NAME_IMAGEVIEW )
                 + "\n\nスレビューに切替 "
                 + CONTROL::get_motion( CONTROL::ToggleArticle ) + " , " + CONTROL::get_motion( CONTROL::Left ) );
        
    pack_buttons();
}

// ボタンのパッキング
void MainToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_main_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_BBSLISTVIEW: get_buttonbar().pack_start( m_button_bbslist, Gtk::PACK_SHRINK ); break;
            case ITEM_FAVORITEVIEW: get_buttonbar().pack_start( m_button_favorite, Gtk::PACK_SHRINK ); break;
            case ITEM_BOARDVIEW: get_buttonbar().pack_start( m_button_board, Gtk::PACK_SHRINK ); break;
            case ITEM_ARTICLEVIEW: get_buttonbar().pack_start( m_button_thread, Gtk::PACK_SHRINK ); break;
            case ITEM_IMAGEVIEW: if( CONFIG::get_use_image_view() ) get_buttonbar().pack_start( m_button_image, Gtk::PACK_SHRINK ); break;
            case ITEM_URL: get_buttonbar().pack_start( m_entry_url ); break;
            case ITEM_GO: get_buttonbar().pack_start( m_button_go, Gtk::PACK_SHRINK ); break;
            case ITEM_SEPARATOR: pack_separator(); break;
        }
        ++num;
    }
}    
