// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historymenu.h"
#include "historysubmenu.h"

using namespace HISTORY;


HistoryMenu::HistoryMenu( const std::string& url_history, const std::string& label )
    : Gtk::MenuItem( label, true ),
      m_activate( false )
{
    m_submenu = Gtk::manage( new HistorySubMenu( url_history ) );
    set_submenu( *m_submenu );
    signal_activate().connect( sigc::mem_fun( *this, &HistoryMenu::slot_activate_menu ) );

    signal_deselect().connect( sigc::mem_fun( *this, &HistoryMenu::slot_deactivate_menu ) );
}


HistoryMenu::~HistoryMenu() noexcept = default;


void HistoryMenu::restore_history()
{
    m_submenu->restore_history();
}


// ラベルをセットする
void HistoryMenu::set_menulabel()
{
    if( ! m_activate ) return;
    if( ! m_submenu ) return;

#ifdef _DEBUG
    std::cout << "HistoryMenu::set_menulabel\n";
#endif

    m_submenu->set_menulabel();
}


// activeになった
void HistoryMenu::slot_activate_menu()
{
#ifdef _DEBUG
    std::cout << "HistoryMenu::slot_activate_menu\n";
#endif

    m_activate = true;
    set_menulabel();
}


// メニューが deactive になった
void HistoryMenu::slot_deactivate_menu()
{
#ifdef _DEBUG
    std::cout << "HistoryMenu::slot_deactivate_menu\n";
#endif

    m_activate = false;
}
