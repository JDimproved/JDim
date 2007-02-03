// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vpaned.h"

using namespace SKELETON;

JDVPaned::JDVPaned()
    : Gtk::VPaned(),
      m_clicked( false ),
      m_drag( false )
{}


JDVPaned::~JDVPaned()
{}


int JDVPaned::get_position()
{
    if( ! m_pos ) m_pos = Gtk::VPaned::get_position();

#ifdef _DEBUG
    std::cout << "JDVPaned::get_position " << m_pos << std::endl;
#endif

    return m_pos;
}

void JDVPaned::set_position( int position )
{
    m_pos = position;
    Gtk::VPaned::set_position( position );
}

// unpack = true の時取り除く
void JDVPaned::add_remove1( bool unpack, Gtk::Widget& child )
{
    if( unpack ) remove( child );
    else add1( child );
}

// unpack = true の時取り除く
void JDVPaned::add_remove2( bool unpack, Gtk::Widget& child )
{
    if( unpack ) remove( child );
    else add2( child );
}


// ページ最大化切り替え
// page = 0 の時は元に戻す
void JDVPaned::toggle_maximize( int page )
{
    int maxpos = property_max_position();
    int pos = Gtk::VPaned::get_position();

#ifdef _DEBUG
    std::cout << "JDVPaned::toggle_maximize page = " << page << " current_pos = " << pos
              << " pos = " << get_position() << " maxpos = " << maxpos << std::endl;
#endif

    // 復元
    if( page == 0 && pos != get_position() ) Gtk::VPaned::set_position( get_position() );

    else if( page == 1 && pos < maxpos ){

        Gtk::VPaned::set_position( maxpos );
    }

    else if( page == 2 && pos > 0 ){

        Gtk::VPaned::set_position( 0 );
    }
}



bool JDVPaned::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDVPaned::on_button_press_event\n";
    std::cout << "x = " << event->x << " pos = " << m_pos << std::endl;
#endif

    m_clicked = true;
    m_drag = false;

    return Gtk::VPaned::on_button_press_event( event );
}


bool JDVPaned::on_button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDVPaned::on_butoon_release_event clicked = " << m_clicked << " drag = " << m_drag << std::endl;
#endif

    // 仕切りをドラッグした場合
    if( m_clicked && m_drag ) m_pos = Gtk::VPaned::get_position();
    m_clicked = false;
    m_drag = false;

    return Gtk::VPaned::on_button_release_event( event );
}



bool JDVPaned::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
//    std::cout << "JDVPaned::on_motion_notify_event\n";
#endif

    m_drag = true;

    return Gtk::VPaned::on_motion_notify_event( event );
}
