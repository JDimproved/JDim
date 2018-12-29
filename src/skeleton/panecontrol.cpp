// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "panecontrol.h"

using namespace SKELETON;

enum
{
    PANE_PAGE_MINSIZE = 32
};

PaneControl::PaneControl( Gtk::Paned& paned, int fixmode )
    : m_paned( paned ),
      m_click_fold( PANE_CLICK_NORMAL ),
      m_clicked( false ),
      m_drag( false ),
      m_on_paned( false ),
      m_fixmode( fixmode ),
      m_mode( PANE_NORMAL )
{
    m_pre_size = -1;
}


PaneControl::~PaneControl() noexcept
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

        update_position();
        m_pre_size = get_size();
    }
}


//
// モードにしたがってセパレータの位置を更新
//
void PaneControl::update_position()
{
#ifdef _DEBUG
    std::cout << "PaneControl::update_position fixmode = " << m_fixmode
              << " mode = " << m_mode << " pos = " << m_pos << " size = " << get_size() << std::endl;
#endif

    int pos = m_paned.get_position();

    // PAGE1を最大化
    if( m_mode == PANE_MAX_PAGE1 ) m_paned.set_position( get_size() );

    // PAGE2を最大化
    else if( m_mode == PANE_MAX_PAGE2 && pos > 0 ) m_paned.set_position( 0 );

    // 通常状態
    else if( m_mode == PANE_NORMAL ){

        // ウィンドウをリサイズしたときの処理
        // m_fixmode の値によりPAGE1とPAGE2のどちらのサイズを固定するか判断する
        int newpos = 0;

        if( m_fixmode == PANE_FIXSIZE_PAGE1 && pos != m_pos ){
            newpos = MIN( m_pos, get_size() - PANE_PAGE_MINSIZE );
        }
        else if( m_fixmode == PANE_FIXSIZE_PAGE2 && pos != get_size() - m_pos ){
            newpos = MAX( get_size() - m_pos, PANE_PAGE_MINSIZE );
        }

        if( newpos ) m_paned.set_position( newpos );

#ifdef _DEBUG
        std::cout << "newpos = " << newpos << " pos = " << m_paned.get_position() << std::endl;
#endif
    }
}


int PaneControl::get_position()
{
    if( ! m_pos ){
        if( m_fixmode == PANE_FIXSIZE_PAGE1 ) m_pos = m_paned.get_position();
        else if( m_fixmode == PANE_FIXSIZE_PAGE2 ) m_pos = get_size() - m_paned.get_position();
    }

#ifdef _DEBUG
    std::cout << "PaneControl::get_position " << m_pos << std::endl;
#endif

    return m_pos;
}


void PaneControl::set_position( int position )
{
    if( position == m_pos ) return;

#ifdef _DEBUG
    std::cout << "PaneControl::set_position position = " << position
              << " size = " << get_size() << std::endl;
#endif

    m_pos = position;
    update_position();
}


//
// ページ最大化切り替え
//
void PaneControl::set_mode( int mode )
{
    if( mode == m_mode ) return;

#ifdef _DEBUG
    int pos = m_paned.get_position();
    int size = get_size();

    std::cout << "PaneControl::set_mode = " << mode
              << " size = " << size
              << " current_pos = " << pos
              << " pos = " << get_position() << std::endl;
#endif

    m_mode = mode;
    update_position();
    m_sig_pane_modechanged.emit( m_mode );
}


// unpack = true の時取り除く
void PaneControl::add_remove1( bool unpack, Gtk::Widget& child )
{
    if( unpack ) m_paned.remove( child );

    // paned.ccg Revision 2 , Tue Jan 21 13:41:59 2003 UTC をハック
    // pack1 を SHRINK、pack2 を EXPAND にしないとリサイズしたときに
    // 仕切りがバタつく
    else m_paned.pack1( child, Gtk::SHRINK );
}


// unpack = true の時取り除く
void PaneControl::add_remove2( bool unpack, Gtk::Widget& child )
{
    if( unpack ) m_paned.remove( child );
    else m_paned.pack2( child, Gtk::EXPAND );
}


void PaneControl::button_press_event( GdkEventButton* event )
{
    m_clicked = is_separater_clicked( event ); // 仕切りをクリックしたかチェック
    m_drag = false;

#ifdef _DEBUG
    std::cout << "PaneControl::button_press_event"
              << " x = " << event->x << " y = " << event->y << " pos = " << m_pos << " current_pos = " << m_paned.get_position()
              << " click = " << m_clicked << std::endl;
#endif
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

        if( m_mode == PANE_NORMAL ){

            if( m_fixmode == PANE_FIXSIZE_PAGE1 ) m_pos = m_paned.get_position();
            else if( m_fixmode == PANE_FIXSIZE_PAGE2 ) m_pos = get_size() - m_paned.get_position();

#ifdef _DEBUG
            std::cout << "new pos = " << m_pos << std::endl;
#endif

            set_mode( PANE_NORMAL );
        }
        else if( m_mode == PANE_MAX_PAGE1 ) m_paned.set_position( get_size() );
        else if( m_mode == PANE_MAX_PAGE2 ) m_paned.set_position( 0 );
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


void PaneControl::enter_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "PaneControl::enter_notify_event\n";
#endif

    m_on_paned = true;
}


void PaneControl::leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "PaneControl::leave_notify_event\n";
#endif

    m_on_paned = false;
}
