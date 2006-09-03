// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "tablabel.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

using namespace SKELETON;

TabLabel::TabLabel( const std::string& url )
    : m_url( url ), m_id_icon( ICON::NUM_ICONS ), m_image( NULL )
{
#ifdef _DEBUG
    std::cout << "TabLabel::TabLabel " <<  m_url << std::endl;
#endif

    pack_end( m_label );

    show_all_children();
}


TabLabel::~TabLabel()
{
#ifdef _DEBUG
    std::cout << "TabLabel::~TabLabel " <<  m_fulltext << std::endl;
#endif

    if( m_image ) delete m_image;
}


void TabLabel::set_fulltext( const std::string& label )
{
    m_fulltext = label;
    m_label.set_text( label );
}


// アイコンセット
void TabLabel::set_id_icon( int id )
{
    if( m_id_icon == id ) return;

    if( !m_image ){
        m_image = new Gtk::Image();
        pack_end( *m_image );
        show_all_children();
    }

#ifdef _DEBUG
    std::cout << "TabLabel::set_icon " <<  m_fulltext << " id = " << id << std::endl;
#endif

    m_id_icon = id;

    m_image->set( ICON::get_icon( id ) );
}



const int TabLabel::get_tabwidth()
{
    const int iconsize = 16;
    const int mrg = 12;    

    int lng_label = m_label.get_layout()->get_pixel_ink_extents().get_width();

    return iconsize + lng_label + mrg;
}


// 伸縮
bool TabLabel::dec()
{
    if( m_label.get_text() == m_fulltext ) return false;

    int lng = m_label.get_text().length() +1;
    resize_tab( lng );

    return true;
}


bool TabLabel::inc()
{
    int lng = m_label.get_text().length() -1;
    if( lng < CONFIG::get_tab_min_str() ) return false;
    resize_tab( lng );

    return true;
}


// タブの文字列の文字数がlngになるようにリサイズする
void TabLabel::resize_tab( int lng )
{
    Glib::ustring ulabel( m_fulltext );
    ulabel.resize( lng );
    m_label.set_text( ulabel );

#ifdef _DEBUG
    std::cout << "TabLabel::resize_tab lng = " << lng << " " << m_label.get_text() << std::endl;
#endif

}
