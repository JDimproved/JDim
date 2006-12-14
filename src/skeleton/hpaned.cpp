// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "hpaned.h"

#include "command.h"

using namespace SKELETON;

JDHPaned::JDHPaned()
    : Gtk::HPaned(),
      m_clicked( false ),
      m_drag( false ),
      m_pos( 0 )
{}


JDHPaned::~JDHPaned()
{}


int JDHPaned::get_position()
{
    if( ! m_pos ) m_pos = Gtk::HPaned::get_position();

#ifdef _DEBUG
    std::cout << "JDHPaned::get_position " << m_pos << std::endl;
#endif

    return m_pos;
}


void JDHPaned::set_position( int position )
{
    m_pos = position;
    Gtk::HPaned::set_position( position );
}


//
// 左ペーン表示/表示切り替え
//
void JDHPaned::show_hide_leftpane()
{
    int pos = Gtk::HPaned::get_position();

    // 復元
    if( pos == 0 ){
#ifdef _DEBUG
        std::cout << "restore!\n";
#endif
        Gtk::HPaned::set_position( get_position() );

        m_sig_show_hide_leftpane.emit( true );
    }

    // 折りたたみ
    else{
#ifdef _DEBUG
        std::cout << "fold!\n";
#endif
        m_pos = pos;
        Gtk::HPaned::set_position( 0 );

        m_sig_show_hide_leftpane.emit( false );
    }

    // タブ幅調整
    CORE::core_set_command( "adjust_tabwidth" );
}



bool JDHPaned::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDHPaned::on_button_press_event\n";
    std::cout << "x = " << event->x << " pos = " << m_pos << std::endl;
#endif

    // 仕切りをクリックしたかチェック
    if( event->type == GDK_BUTTON_PRESS && event->button == 1
        && event->x >= 0 && event->x <= 8
        ) m_clicked = true;
    m_drag = false;

    return Gtk::HPaned::on_button_press_event( event );
}

bool JDHPaned::on_button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "JDHPaned::on_butoon_release_event clicked = " << m_clicked << std::endl;
#endif

    bool ret = Gtk::HPaned::on_button_release_event( event );

    // 仕切りをクリックしたら折りたたむ
    if( m_clicked && ! m_drag ) show_hide_leftpane();

    // ドラッグした場合
    else if( m_clicked && m_drag ){

        int pos = Gtk::HPaned::get_position();

        if( pos ){
            m_pos = pos;
            m_sig_show_hide_leftpane.emit( true );
        }
        else m_sig_show_hide_leftpane.emit( false );
    }

    m_clicked = false;

    return ret;
}


bool JDHPaned::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
//    std::cout << "JDHPaned::on_motion_notify_event\n";
#endif

    m_drag = true;

    return Gtk::HPaned::on_motion_notify_event( event );
}
