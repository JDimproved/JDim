// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "historymenu.h"
#include "historysubmenu.h"
#include "global.h"
#include "cache.h"

using namespace CORE;


HistoryMenu::HistoryMenu()
    : Gtk::MenuItem( "_History", true )
{
    m_submenu = Gtk::manage( new CORE::HistorySubMenu( CACHE::path_xml_history() ) );
    set_submenu( *m_submenu );
    signal_activate().connect( sigc::mem_fun( *this, &HistoryMenu::slot_activate_menu ) );

    // セパレータ
    Gtk::MenuItem* item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_submenu->prepend( *item );

    // 板サブメニュー
    item = Gtk::manage( new Gtk::MenuItem( "板履歴" ) );
    m_submenu_board = Gtk::manage( new CORE::HistorySubMenu( CACHE::path_xml_history_board() ) );
    item->set_submenu( *m_submenu_board );
    m_submenu->prepend( *item );

    // セパレータ
    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_submenu->append( *item );

    // クリア
    item = Gtk::manage( new Gtk::MenuItem( "履歴クリア" ) );
    m_submenu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistoryMenu::slot_clear ) );
}


HistoryMenu::~HistoryMenu()
{
#ifdef _DEBUG
    std::cout << "HistoryMenu::~HistoryMenu\n";
#endif
}


void HistoryMenu::append( const std::string& url, const std::string& name, int type )
{
    if( type == TYPE_THREAD && m_submenu ) m_submenu->append_item( url, name, type );
    if( type == TYPE_BOARD && m_submenu_board ) m_submenu_board->append_item( url, name, type );
}


// activeになった時にラベルをセットする
void HistoryMenu::slot_activate_menu()
{
#ifdef _DEBUG
    std::cout << "HistoryMenu::slot_activate_menu\n";
#endif
    if( m_submenu ) m_submenu->set_menulabel();
    if( m_submenu_board ) m_submenu_board->set_menulabel();
}


// 履歴のクリア
void HistoryMenu::slot_clear()
{
    Gtk::MessageDialog mdiag( "履歴を全てクリアしますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
    if( mdiag.run() != Gtk::RESPONSE_OK ) return;
    
    if( m_submenu ) m_submenu->clear();
    if( m_submenu_board ) m_submenu_board->clear();
}
