// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "view.h"

#include "global.h"

using namespace SKELETON;


View::View( const std::string& url, const std::string& arg1 ,const std::string& arg2 )
    : m_url( url ),
      m_status( std::string() ),
      m_enable_mg( false ),
      m_enable_autoreload( false ),
      m_autoreload_mode( AUTORELOAD_NOT ),
      m_popupmenu_shown( false )
{}


// クロック入力
// clock_in_always()はviewの種類に依らず常に呼び出されるので重い処理を含めてはいけない
void View::clock_in_always()
{
    // オートリロード
    if( inc_autoreload_counter() ) reload();
}


// オートリロードのカウンタをインクリメント
// 指定秒数を越えたら true を返す
bool View::inc_autoreload_counter()
{
    if( m_autoreload_mode == AUTORELOAD_NOT ) return false;

    ++m_autoreload_counter;

    if( m_autoreload_counter > m_autoreload_sec * 1000/TIMER_TIMEOUT ){
        reset_autoreload_counter();
        return true;
    }

    return false;
}


// オートリロードのモードを設定
void View::set_autoreload_mode( int mode, int sec )
{
    if( ! m_enable_autoreload ) return;

    m_autoreload_mode = mode;
    m_autoreload_sec = sec;
    m_autoreload_counter = 0;
}


// オートリロードのカウンタをリセット
void View::reset_autoreload_counter()
{
    m_autoreload_counter = 0;
    if( m_autoreload_mode == AUTORELOAD_ONCE ) m_autoreload_mode = AUTORELOAD_NOT;
}



// view 上にマウスポインタがあれば true
bool View::is_mouse_on_view()
{
    bool ret = false;

    int x,y;
    get_pointer( x, y );
    if( x <= get_width() && x >= 0 && y <= get_height() && y >= 0 ) ret = true;

    return ret;
}



// ポップアップメニュー表示
void View::show_popupmenu( const std::string& url, bool use_slot )
{
    // ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
    activate_act_before_popupmenu( url );

    Gtk::Menu* popupmenu = get_popupmenu( url );
    if( popupmenu ){

        popupmenu->signal_hide().connect( sigc::mem_fun( *this, &View::slot_hide_popupmenu ) ); 

        if( use_slot ) popupmenu->popup( sigc::mem_fun( *this, &View::slot_popup_menu_position ), 0, gtk_get_current_event_time() );
        else popupmenu->popup( 0, gtk_get_current_event_time() );

        m_popupmenu_shown = true;
    }
}


// ポップアップメニュー表示時に表示位置を決めるスロット
void View::slot_popup_menu_position( int& x, int& y, bool& push_in)
{
    // viewの左上の座標をセットする
    int x2, y2;
    get_window()->get_position( x, y );
    translate_coordinates( *dynamic_cast< Gtk::Widget* >( get_toplevel() ), 0, 0, x2, y2 );
    x += x2;
    y += y2;
    push_in = false;
}


//
// ポップアップメニューがhideしたときに呼ばれるslot
//
void View::slot_hide_popupmenu()
{
    m_popupmenu_shown = false;

    // もしviewがポップアップウィンドウ上にあって、かつ
    // メニューを消したときにマウスポインタが領域外にあれば自分自身をhide
    if( ! is_mouse_on_view() ) sig_hide_popup().emit();
}
