// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "popupwin.h"

using namespace SKELETON;


PopupWin::PopupWin( Gtk::Widget* parent, SKELETON::View* view, const int mrg_x, const int mrg_y )
    : PopupWinBase( POPUPWIN_NOFRAME ),
      m_parent( parent ),
      m_view( view ),
      m_mrg_x( mrg_x ),
      m_mrg_y( mrg_y )
{
#ifdef _DEBUG
    std::cout << "PopupWin::PopupWin\n";
#endif

    m_view->sig_resize_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_resize_popup ) );
    add( *m_view );
    m_view->sig_hide_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_hide_popup ) );
    m_view->show_view();

    Gtk::Widget* toplevel = m_parent->get_toplevel();
#if GTKMM_CHECK_VERSION(2,18,0)
    const bool is_toplevel = toplevel->get_is_toplevel();
#else
    const bool is_toplevel = toplevel->is_toplevel();
#endif
    if( is_toplevel ) {
        set_transient_for( *dynamic_cast< Gtk::Window* >( toplevel ) );
    }
    slot_resize_popup();
}


PopupWin::~PopupWin()
{
    if( m_view ) delete m_view;
}


//
// ポップアップウィンドウの座標と幅と高さを計算して移動とリサイズ
//
void PopupWin::slot_resize_popup()
{
    if( ! m_view ) return;
    
    // マウス座標
    int x_mouse, y_mouse;
#if GTKMM_CHECK_VERSION(3,0,0)
    Gdk::Display::get_default()->get_device_manager()->get_client_pointer()->get_position( x_mouse, y_mouse );
#else
    Gdk::ModifierType mod;
    Gdk::Display::get_default()->get_pointer( x_mouse, y_mouse,  mod );
#endif

    // クライアントのサイズを取得
    const int width_client = m_view->width_client();
    const int height_client = m_view->height_client();

    const int width_desktop = m_parent->get_screen()->get_width();
    const int height_desktop = m_parent->get_screen()->get_height();

    // x 座標と幅
    const int width_popup = width_client;
    int x_popup;
    if( x_mouse + m_mrg_x + width_popup <= width_desktop ) x_popup = x_mouse + m_mrg_x;
    else x_popup = MAX( 0, width_desktop - width_popup );

    // y 座標と高さ
    int y_popup;  
    int height_popup;
    if( y_mouse - ( height_client + m_mrg_y ) >= 0 ){  // 上にスペースがある
        y_popup = y_mouse - height_client - m_mrg_y;
        height_popup = height_client;
    }
    else if( y_mouse + m_mrg_y + height_client <= height_desktop ){ // 下にスペースがある
        y_popup = y_mouse + m_mrg_y;        
        height_popup = height_client;
    }
    else if( m_view->get_popup_upside() || y_mouse > height_desktop/2 ){ // スペースは無いが上に表示
        y_popup = MAX( 0, y_mouse - ( height_client + m_mrg_y ) );
        height_popup = y_mouse - ( y_popup + m_mrg_y );
    }
    else{ // 下
        y_popup = y_mouse + m_mrg_y;        
        height_popup = height_desktop - y_popup;
    }

#ifdef _DEBUG
    std::cout << "PopupWin::slot_resize_popup : x = " << x_popup << " y = " << y_popup
              << " w = " << width_popup << " h = " << height_popup << std::endl;
#endif
    move( x_popup, y_popup );
    resize( width_popup,  height_popup );
    show_all();
}
