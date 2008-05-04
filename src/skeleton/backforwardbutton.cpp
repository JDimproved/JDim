// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "backforwardbutton.h"

#include "history/historymanager.h"
#include "history/viewhistoryitem.h"

using namespace SKELETON;

BackForwardButton::BackForwardButton( const std::string& url, const bool back )
    : SKELETON::MenuButton( back ? Gtk::Stock::GO_BACK : Gtk::Stock::GO_FORWARD ),
      m_url( url ),
      m_back( back )
{}


void BackForwardButton::set_url( const std::string& url )
{
    m_url = url;

#ifdef _DEBUG
    std::cout << "BackForwardButton::set_url back = " << m_back << " url = " << m_url << std::endl;
#endif

    if( m_back ){
        if( HISTORY::get_history_manager()->can_back_viewhistory( m_url, 1 ) ) set_sensitive( true );
        else set_sensitive( false );
    }
    else{
        if( HISTORY::get_history_manager()->can_forward_viewhistory( m_url, 1 ) ) set_sensitive( true );
        else set_sensitive( false );
    }
}


//
// ポップアップメニュー表示
//
// virtual
void BackForwardButton::show_popupmenu()
{
#ifdef _DEBUG
    std::cout << "BackForwardButton::show_popupmenu back = " << m_back << " url = " << m_url << std::endl;
#endif
    std::vector< std::string > items;

    // 戻る更新
    if( m_back ){
        std::vector< HISTORY::ViewHistoryItem* >& histitems
        = HISTORY::get_history_manager()->get_items_back_viewhistory( m_url, MAX_MENU_SIZE );
        std::vector< HISTORY::ViewHistoryItem* >::iterator it = histitems.begin();
        for( ; it != histitems.end(); ++it ) items.push_back( (*it)->title );
    }
    else{
        std::vector< HISTORY::ViewHistoryItem* >& histitems
        = HISTORY::get_history_manager()->get_items_forward_viewhistory( m_url, MAX_MENU_SIZE );
        std::vector< HISTORY::ViewHistoryItem* >::iterator it = histitems.begin();
        for( ; it != histitems.end(); ++it ) items.push_back( (*it)->title );
    }

    append_menu( items );
    SKELETON::MenuButton::show_popupmenu();
}
