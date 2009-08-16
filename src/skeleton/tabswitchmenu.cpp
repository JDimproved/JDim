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
    : Gtk::Menu(), m_parentadmin( admin ), m_parentnote( notebook ), m_deactivated( true ), m_size( 0 )
{

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::TabSwitchMenu\n";
#endif 
}


TabSwitchMenu::~TabSwitchMenu()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::~TabSwitchMenu\n";
#endif 
}



void TabSwitchMenu::remove_items()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::remove_items\n";
#endif

    for( int i = 0; i < m_size; ++i ) remove( *m_vec_items[ i ] );
}


void TabSwitchMenu::append_items()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::append_items\n";
#endif

    m_size = m_parentnote->get_n_pages();

    if( (int) m_vec_items.size() < m_size ){

        for( int i = m_vec_items.size(); i < m_size ; ++ i ){

            Gtk::Image* image = Gtk::manage( new Gtk::Image() );
            m_vec_images.push_back( image );

            Gtk::Label* label = Gtk::manage( new Gtk::Label() );
            m_vec_labels.push_back( label );

            Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox() );
            hbox->set_spacing( SPACING_MENU );
            hbox->pack_start( *image, Gtk::PACK_SHRINK );
            hbox->pack_start( *label, Gtk::PACK_SHRINK );

            Gtk::MenuItem* item = Gtk::manage( new Gtk::MenuItem( *hbox ) );
            item->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *m_parentadmin, &Admin::set_current_page_focus ), i ) );
            m_vec_items.push_back( item );
        }
    }

    for( int i = 0; i < m_size; ++i ) append( *m_vec_items[ i ] );

    show_all_children();
    m_deactivated = false;
}


void TabSwitchMenu::update_labels()
{
    if( m_deactivated ) return;
    if( ! m_parentnote ) return;

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::update_labels\n";
#endif 

    for( int i = 0; i < m_size; ++ i ){

        std::string name = m_parentnote->get_tab_fulltext( i );
        if( name.empty() ) name = "???";

        const unsigned int maxsize = 50;
        m_vec_labels[ i ]->set_label( MISC::cut_str( name, maxsize ) );
    }
}


void TabSwitchMenu::update_icons()
{
    if( m_deactivated ) return;
    if( ! m_parentnote ) return;

#ifdef _DEBUG
    std::cout << "TabSwitchMenu::update_icons\n";
#endif 

    for( int i = 0; i < m_size; ++ i ){

        const int icon = m_parentnote->get_tabicon( i );
        if( icon != ICON::NONE && icon != ICON::NUM_ICONS ){
            m_vec_images[ i ]->set( ICON::get_icon( icon ) );
        }
    }
}


void TabSwitchMenu::deactivate()
{
#ifdef _DEBUG
    std::cout << "TabSwitchMenu::deactivate\n";
#endif

    m_deactivated = true;
}


void TabSwitchMenu::on_deactivate()
{
    deactivate();

    Gtk::Menu::on_deactivate();
}
