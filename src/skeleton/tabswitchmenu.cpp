// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabswitchmenu.h"
#include "dragnote.h"
#include "admin.h"

#include "jdlib/miscutil.h"

#include "icons/iconmanager.h"

enum
{
    SPACING_MENU = 3, // アイコンと項目名の間のスペース
};


using namespace SKELETON;

TabSwitchMenu::TabSwitchMenu( DragableNoteBook* notebook, Admin* admin )
    : Gtk::Menu(), m_parentnote( notebook ), m_deactivated( false )
{

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::TabSwitchMenu\n";
#endif 

    if( ! m_parentnote ) return;

    const int pages = m_parentnote->get_n_pages();
    for( int i = 0; i < pages; ++ i ){

        std::string name = m_parentnote->get_tab_fulltext( i );
        if( name.empty() ) name = "???";

        Gtk::Image* image = Gtk::manage( new Gtk::Image() );
        m_vec_images.push_back( image );

        const unsigned int maxsize = 50;
        Gtk::Label* label = Gtk::manage( new Gtk::Label( MISC::cut_str( name, maxsize ) ) );

        Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox() );
        hbox->set_spacing( SPACING_MENU );
        hbox->pack_start( *image, Gtk::PACK_SHRINK );
        hbox->pack_start( *label, Gtk::PACK_SHRINK );

        Gtk::MenuItem* item = Gtk::manage( new Gtk::MenuItem( *hbox ) );
        item->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *admin, &Admin::set_current_page_focus ), i ) );

        append( *item );
    }

    update_icons();
    show_all_children();
}


TabSwitchMenu::~TabSwitchMenu()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::~TabSwitchMenu\n";
#endif 
}
 

void TabSwitchMenu::update_icons()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::update_icons\n";
#endif 

    if( m_deactivated ) return;
    if( ! m_parentnote ) return;

    const size_t pages = m_parentnote->get_n_pages();
    if( pages != m_vec_images.size() ) return;
    for( size_t i = 0; i < pages; ++ i ){

        const int icon = m_parentnote->get_tabicon( i );
        if( icon != ICON::NONE && icon != ICON::NUM_ICONS ){
            m_vec_images[ i ]->set( ICON::get_icon( icon ) );
        }
    }
}


void TabSwitchMenu::on_deactivate()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::on_deactivate\n";
#endif

    m_deactivated = true;

    Gtk::Menu::on_deactivate();
}
