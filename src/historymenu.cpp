// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "historymenu.h"
#include "historysubmenu.h"
#include "global.h"
#include "cache.h"

using namespace CORE;


HistoryMenuBase::HistoryMenuBase( const std::string& label )
    : Gtk::MenuItem( label, true )
{}


HistoryMenuBase::~HistoryMenuBase()
{
#ifdef _DEBUG
    std::cout << "HistoryMenuBase::~HistoryMenuBase\n";
#endif
}


void HistoryMenuBase::setup( CORE::HistorySubMenu* submenu )
{
    m_submenu = submenu;
    set_submenu( *m_submenu );
    signal_activate().connect( sigc::mem_fun( *this, &HistoryMenuBase::slot_activate_menu ) );

    // セパレータ
    Gtk::MenuItem* item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_submenu->prepend( *item );

    // クリア
    Gtk::Menu* menu = Gtk::manage( new Gtk::Menu() );
    item = Gtk::manage( new Gtk::MenuItem( "クリアする" ) );
    menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistoryMenuBase::slot_clear ) ); 

    item = Gtk::manage( new Gtk::MenuItem( "履歴クリア" ) );
    item->set_submenu( *menu );
    m_submenu->prepend( *item );
}



void HistoryMenuBase::append( const std::string& url, const std::string& name, int type )
{
    if( m_submenu ) m_submenu->append_item( url, name, type );
}


// activeになった時にラベルをセットする
void HistoryMenuBase::slot_activate_menu()
{
#ifdef _DEBUG
    std::cout << "HistoryMenuBase::slot_activate_menu\n";
#endif
    if( m_submenu ) m_submenu->set_menulabel();
}


// 履歴のクリア
void HistoryMenuBase::slot_clear()
{
    if( m_submenu ) m_submenu->clear();
}


///////////////////////////////////////////////////


HistoryMenuThread::HistoryMenuThread()
    : CORE::HistoryMenuBase( "スレ履歴(_T)" )
{
    setup( Gtk::manage( new CORE::HistorySubMenu( CACHE::path_xml_history() ) ) );
}



HistoryMenuBoard::HistoryMenuBoard()
    : CORE::HistoryMenuBase( "板履歴(_B)" )
{
    setup( Gtk::manage( new CORE::HistorySubMenu( CACHE::path_xml_history_board() ) ) );
}
