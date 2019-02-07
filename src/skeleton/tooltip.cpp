// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tooltip.h"

#include "global.h"

using namespace SKELETON;

// ツールチップ
Tooltip::Tooltip()
    : PopupWinBase( POPUPWIN_DRAWFRAME ),
      m_counter( 0 ),
      m_min_width( 0 )
{
    // 背景色をテーマに合わせる
    set_name( "gtk-tooltips" );
    set_border_width( 4 );
    add( m_label );
    show_all_children();
}


// フォント
void Tooltip::modify_font_label( const std::string& fontname )
{
    Pango::FontDescription pfd( fontname );
    pfd.set_weight( Pango::WEIGHT_NORMAL );
#if GTKMM_CHECK_VERSION(3,0,0)
    m_label.override_font( pfd );
#else
    m_label.modify_font( pfd );
#endif
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
#if GTKMM_CHECK_VERSION(3,0,0)
    Gdk::Display::get_default()->get_device_manager()->get_client_pointer()->get_position( x_mouse, y_mouse );
#else
    Gdk::ModifierType mod;
    Gdk::Display::get_default()->get_pointer( x_mouse, y_mouse,  mod );
#endif

    // 一度画面外にshow()して幅を確定してから、もし m_min_width よりも
    // 幅が大きければマウスの位置に移動する
    move( -100, -100 );
    show();
    const int width = get_width();
    const int height = get_height();
    
#ifdef _DEBUG
    std::cout << "width / min_width = " << width << " / " << m_min_width << std::endl;
#endif
    if( width >= m_min_width ){

        // 画面外にはみださないように調整
        const int mrg = 4;
        const int width_desktop = get_screen()->get_width();
        if( x_mouse + width > width_desktop ) x_mouse = MAX( 0, width_desktop - width );

        move( x_mouse, y_mouse - height - mrg );
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
