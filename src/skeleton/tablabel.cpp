// ライセンス: 最新のGPL

#define _DEBUG
#include "jddebug.h"

#include "tablabel.h"

using namespace SKELETON;

TabLabel::TabLabel( const std::string& url )
    : m_url( url )
{
    pack_start( m_label );
    show_all_children();
}


TabLabel::~TabLabel()
{
#ifdef _DEBUG
    std::cout << "TabLabel::~TabLabel " <<  m_fulltext << std::endl;
#endif
}


void TabLabel::set_fulltext( const std::string& label )
{
    m_fulltext = label;
    m_label.set_text( label );
}


const int TabLabel::get_tabwidth()
{
    const int mrg = 10;    

    int lng_label = m_label.get_layout()->get_pixel_ink_extents().get_width();

    return lng_label +mrg;
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
    if( lng <= 0 ) return false;
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
