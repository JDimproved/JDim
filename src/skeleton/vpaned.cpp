// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vpaned.h"

using namespace SKELETON;


enum
{
    MAXMODE_NORMAL = 0,
    MAXMODE_PAGE1,
    MAXMODE_PAGE2
};


JDVPaned::JDVPaned()
    : Gtk::VPaned(),
      m_clicked( false ),
      m_drag( false ),
      m_mode( MAXMODE_NORMAL )
{
    m_pre_height = get_height();
}


JDVPaned::~JDVPaned()
{}


//
// クロック入力
//
void JDVPaned::clock_in()
{
    // Gtk::Paned は configure_event()をキャッチ出来ないので
    // 応急処置としてタイマーの中でサイズが変更したか調べて
    // 変わっていたら仕切りの位置を調整する
    if( m_pre_height != get_height() ){

#ifdef _DEBUG
        int pos = Gtk::VPaned::get_position();
        std::cout << "JDVPande::resize pos = " << pos
                  << " preheight = " << m_pre_height << " height = " << get_height() << std::endl;
#endif

        if( m_mode == MAXMODE_PAGE1 ) Gtk::VPaned::set_position( get_height() );

        m_pre_height = get_height();
    }
}


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
    int pos = Gtk::VPaned::get_position();

#ifdef _DEBUG
    std::cout << "JDVPaned::toggle_maximize page = " << page << " current_pos = " << pos
              << " pos = " << get_position() << std::endl;
#endif

    // 復元
    if( page == 0 && m_mode != MAXMODE_NORMAL && pos != get_position() ){

        m_mode = MAXMODE_NORMAL;
        Gtk::VPaned::set_position( get_position() );
    }

    else if( page == 1 && m_mode != MAXMODE_PAGE1 ){

        m_mode = MAXMODE_PAGE1;
        Gtk::VPaned::set_position( get_height() );
    }

    else if( page == 2 && m_mode != MAXMODE_PAGE2 && pos > 0 ){

        m_mode = MAXMODE_PAGE2;
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
    if( m_clicked && m_drag ){
        m_mode = MAXMODE_NORMAL;
        m_pos = Gtk::VPaned::get_position();
    }

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
