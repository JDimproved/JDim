// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "tooltip.h"

#include "global.h"

using namespace SKELETON;

// ツールチップ
Tooltip::Tooltip()
    : Gtk::Window( Gtk::WINDOW_POPUP ),
      m_counter( 0 ),
      m_min_width( 0 )
{
    // 背景色をテーマに合わせる
    set_name( "gtk-tooltips" );
    set_border_width( 4 );
    add( m_label );
    show_all_children();
}


void Tooltip::on_realize()
{
    Gtk::Window::on_realize();

    Glib::RefPtr< Gdk::Window > window = get_window();
    m_gc = Gdk::GC::create( window );    
}



bool Tooltip::on_expose_event( GdkEventExpose* event )
{
    Gtk::Window::on_expose_event( event );

    // 黒枠を描く
    Glib::RefPtr< Gdk::Window > window = get_window();
    m_gc->set_foreground( Gdk::Color( "black" ) );
    window->draw_rectangle( m_gc, false, 0, 0, get_width()-1, get_height()-1 );
    return true;    
}


//
// クロック入力
//
void Tooltip::clock_in()
{
    const int popup_time = 500 // msec
    / TIMER_TIMEOUT ; 

    // カウンターが一杯になったらツールチップ表示
    ++m_counter;
    if( m_counter == popup_time ) show_tooltip();
}




void Tooltip::set_text( const std::string& text )
{
    if( m_label.get_text() == text ) return;

#ifdef _DEBUG
    std::cout << "Tooltip::set_text" << text << std::endl;
#endif

    hide_tooltip();
    resize( 1, 1 );
    m_label.set_text( text );
    m_counter = 0;
}



//
// 表示
//
void Tooltip::show_tooltip()
{
    if( m_label.get_text().empty() ) return;

#ifdef _DEBUG
    std::cout << "Tooltip::show_tooltip" << m_label.get_text() << std::endl;
#endif

    int x_mouse, y_mouse;
    Gdk::ModifierType mod;
    Gdk::Display::get_default()->get_pointer( x_mouse, y_mouse,  mod );

    // 一度画面外にshow()して幅を確定してから、もし m_min_width よりも
    // 幅が大きければマウスの位置に移動する
    move( -100, -100 );
    show();
    int width = get_width();
    
#ifdef _DEBUG
    std::cout << "width / min_width = " << width << " / " << m_min_width << std::endl;
#endif
    if( width >= m_min_width ){

        // 画面外にはみださないように調整
        const int mrg = 30;
        int width_desktop = get_screen()->get_width();
        if( x_mouse + width > width_desktop ) x_mouse = MAX( 0, width_desktop - width );

        move( x_mouse, y_mouse - mrg );
    }
}


void Tooltip::hide_tooltip()
{
#ifdef _DEBUG
    std::cout << "Tooltip::hide_tooltip\n";
#endif

    m_counter = 10000;
    hide();
    m_label.set_text( std::string() );
}
