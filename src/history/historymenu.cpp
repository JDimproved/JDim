// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historymenu.h"
#include "historysubmenu.h"
#include "global.h"
#include "cache.h"

using namespace HISTORY;


HistoryMenuBase::HistoryMenuBase( const std::string& label )
    : Gtk::MenuItem( label, true )
{}


HistoryMenuBase::~HistoryMenuBase()
{
#ifdef _DEBUG
    std::cout << "HistoryMenuBase::~HistoryMenuBase\n";
#endif
}


void HistoryMenuBase::setup( HistorySubMenu* submenu )
{
    m_submenu = submenu;
    set_submenu( *m_submenu );
    signal_activate().connect( sigc::mem_fun( *this, &HistoryMenuBase::slot_activate_menu ) );

    // セパレータ
    Gtk::MenuItem* item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_submenu->prepend( *item );

    // クリア
    Gtk::Menu* menu = Gtk::manage( new Gtk::Menu() );
    item = Gtk::manage( new Gtk::MenuItem( "クリアする(_C)", true ) );
    menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistoryMenuBase::slot_clear ) ); 

    item = Gtk::manage( new Gtk::MenuItem( "履歴クリア(_C)", true ) );
    item->set_submenu( *menu );
    m_submenu->prepend( *item );
}



void HistoryMenuBase::append( const std::string& url, const std::string& name, int type )
{
    if( m_submenu ) m_submenu->append_item( url, name, type );
}


void HistoryMenuBase::update()
{
    if( m_submenu ) m_submenu->update();
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
    : HistoryMenuBase( "スレ履歴(_T)" )
{
    // ファイルが存在しなければ入力を旧ファイル名にする
    std::string file_in = CACHE::path_xml_history();
    if( CACHE::file_exists( file_in ) != CACHE::EXIST_FILE ) file_in = CACHE::path_xml_history_old();

    const std::string file_out = CACHE::path_xml_history();

    setup( Gtk::manage( new HistorySubMenu( file_in, file_out ) ) );
}



HistoryMenuBoard::HistoryMenuBoard()
    : HistoryMenuBase( "板履歴(_B)" )
{
    // ファイルが存在しなければ入力を旧ファイル名にする
    std::string file_in = CACHE::path_xml_history_board();
    if( CACHE::file_exists( file_in ) != CACHE::EXIST_FILE ) file_in = CACHE::path_xml_history_board_old();

    const std::string file_out = CACHE::path_xml_history_board();

    setup( Gtk::manage( new HistorySubMenu( file_in, file_out ) ) );
}



HistoryMenuClose::HistoryMenuClose()
    : HistoryMenuBase( "最近閉じたスレ(_M)" )
{
    // ファイルが存在しなければ入力を旧ファイル名にする
    std::string file_in = CACHE::path_xml_history_close();
    if( CACHE::file_exists( file_in ) != CACHE::EXIST_FILE ) file_in = CACHE::path_xml_history_close_old();

    const std::string file_out = CACHE::path_xml_history_close();

    setup( Gtk::manage( new HistorySubMenu( file_in, file_out ) ) );
}
