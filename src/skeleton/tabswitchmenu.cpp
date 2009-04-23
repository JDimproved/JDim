// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabswitchmenu.h"
#include "dragnote.h"
#include "admin.h"

#include "jdlib/miscutil.h"

using namespace SKELETON;

TabSwitchMenu::TabSwitchMenu( DragableNoteBook* notebook, Admin* admin )
    : Gtk::Menu()
{

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::TabSwitchMenu\n";
#endif 

    const int pages = notebook->get_n_pages();
    for( int i = 0; i < pages; ++ i ){

        std::string name = notebook->get_tab_fulltext( i );
        if( name.empty() ) name = "???";

        const unsigned int maxsize = 50;
        Gtk::MenuItem* item = Gtk::manage( new Gtk::MenuItem( MISC::cut_str( name, maxsize ) ) );
        item->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *admin, &Admin::set_current_page ), i ) );

        append( *item );
    }

    show_all_children();
}


TabSwitchMenu::~TabSwitchMenu()
{

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::~TabSwitchMenu\n";
#endif 
}
 

