// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "panecontrol.h"

using namespace SKELETON;


PaneControl::PaneControl( Gtk::Paned& paned )
    : m_paned( paned ),
      m_click_fold( PANE_CLICK_NORMAL ),
      m_clicked( false ),
      m_drag( false ),
      m_mode( PANE_NORMAL )
{
    m_pre_size = -1;
}


PaneControl::~PaneControl()
{}


//
// クロック入力
//
void PaneControl::clock_in()
{
    // Gtk::Paned は configure_event()をキャッチ出来ないので
    // 応急処置としてタイマーの中でサイズが変更したか調べて
    // 変わっていたら仕切りの位置を調整する
    if( m_pre_size != get_size() ){

#ifdef _DEBUG
        int pos = m_paned.get_position();
        std::cout << "PaneControl::clock_in resize pos = " << pos
                  << " presize = " << m_pre_size << " size = " << get_size() << std::endl;
#endif

        if( m_mode == PANE_MAX_PAGE1 ) m_paned.set_position( get_size() );
        else if( m_mode == PANE_MAX_PAGE2 ) m_paned.set_position( 0 );
        else set_position( m_pos );

        m_pre_size = get_size();
    }
}


int PaneControl::get_position()
{
    if( ! m_pos ) m_pos = get_size() - m_paned.get_position();

#ifdef _DEBUG
    std::cout << "PaneControl::get_position " << m_pos << std::endl;
#endif

    return m_pos;
}

void PaneControl::set_position( int position )
{
#ifdef _DEBUG
    std::cout << "PaneControl::set_position " << position
              << " size = " << get_size() << std::endl;
#endif

    m_pos = position;
    m_paned.set_position( get_size() - position );
}


//
// ページ最大化切り替え
//
void PaneControl::set_mode( int mode )
{
    if( mode == m_mode ) return;

    m_mode = mode;
    int pos = m_paned.get_position();
    int size = get_size();

#ifdef _DEBUG
    std::cout << "PaneControl::set_mode = " << mode
              << " size = " << size
              << " current_pos = " << pos
              << " pos = " << get_position() << std::endl;
#endif

    // 復元
    if( mode == PANE_NORMAL && pos != size - get_position() ){

        m_paned.set_position( size - get_position() );
    }

    else if( mode == PANE_MAX_PAGE1 ){

        m_paned.set_position( size );
    }

    else if( mode == PANE_MAX_PAGE2 && pos > 0 ){

        m_paned.set_position( 0 );
    }

    m_sig_pane_modechanged.emit( m_mode );
}


// unpack = true の時取り除く
void PaneControl::add_remove1( bool unpack, Gtk::Widget& child )
{
    if( unpack ) m_paned.remove( child );
    else m_paned.add1( child );
}


// unpack = true の時取り除く
void PaneControl::add_remove2( bool unpack, Gtk::Widget& child )
{
    if( unpack ) m_paned.remove( child );
    else m_paned.add2( child );
}


void PaneControl::button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "PaneControl::_button_press_event\n";
    std::cout << "x = " << event->x << " pos = " << m_pos << std::endl;
#endif
    
    m_clicked = true;
    m_drag = false;
}


void PaneControl::button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "PaneControl::butoon_release_event clicked = " << m_clicked << " drag = " << m_drag << std::endl;
#endif

    // 仕切りをクリックしたら折りたたむ
    if( m_click_fold != PANE_CLICK_NORMAL && m_clicked && ! m_drag ){

        if( m_click_fold == PANE_CLICK_FOLD_PAGE1 ){

            if( m_mode == PANE_NORMAL ) set_mode( PANE_MAX_PAGE2 );
            else set_mode( PANE_NORMAL );
        }
        else if( m_click_fold == PANE_CLICK_FOLD_PAGE2 ){

            if( m_mode == PANE_NORMAL ) set_mode( PANE_MAX_PAGE1 );
            else set_mode( PANE_NORMAL );
        }
    }

    // 仕切りをドラッグした場合
    else if( m_clicked && m_drag ){
        m_mode = PANE_NORMAL;
        m_pos = get_size() - m_paned.get_position();

#ifdef _DEBUG
        std::cout << "pos = " << m_pos << std::endl;
#endif
    }

    m_clicked = false;
    m_drag = false;
}



void PaneControl::motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
//    std::cout << "PaneControl::motion_notify_event\n";
#endif

    m_drag = true;
}
