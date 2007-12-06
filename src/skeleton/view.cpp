// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "view.h"
#include "toolbar.h"

#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace SKELETON;

enum
{
    KEYJUMP_TIMEOUT = 1000
};


View::View( const std::string& url, const std::string& arg1 ,const std::string& arg2 )
    : m_url( url ),
      m_status( std::string() ),
      m_enable_mg( false ),
      m_enable_autoreload( false ),
      m_autoreload_mode( AUTORELOAD_NOT ),
      m_keyjump_counter( 0 ),
      m_keyjump_num( 0 ),
      m_lockable( true ),
      m_locked( false ),
      m_toolbar( NULL )
{}


//
// host の更新
//
// 移転があったときなどにadminから呼ばれる
//
void View::update_host( const std::string& host )
{
#ifdef _DEBUG
    std::string old_url = m_url;
#endif

    m_url = host + m_url.substr( MISC::get_hostname( m_url ).length() );

#ifdef _DEBUG
    if( ! old_url.empty() ) std::cout << "View::update_host from "  << old_url
                                     << " to " << m_url << std::endl;
#endif
}


// クロック入力
// clock_in_always()はviewの種類に依らず常に呼び出されるので重い処理を含めてはいけない
void View::clock_in_always()
{
    // オートリロード
    if( inc_autoreload_counter() ) reload();

    // キーボード数字入力ジャンプ
    if( inc_keyjump_counter() ){
        goto_num( m_keyjump_num );
        reset_keyjump_counter();
    }
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


// 数字入力ジャンプカウンタのインクリメント
// 指定秒数を越えたら true を返す
bool View::inc_keyjump_counter()
{
    if( ! m_keyjump_counter ) return false;

    ++m_keyjump_counter;

    if( m_keyjump_counter > KEYJUMP_TIMEOUT / TIMER_TIMEOUT ) return true;

    return false;
}



// 数字入力ジャンプカウンタのリセット
void View::reset_keyjump_counter()
{
    m_keyjump_counter = 0;
    m_keyjump_num = 0;
}


// 数字入力ジャンプ用に sig_key_press() から呼び出す
void View::release_keyjump_key( int key )
{
    // キーパッド対応
    if( key >= GDK_KP_0 && key <= GDK_KP_9 ) key = key - GDK_KP_0 + GDK_0;

    if( key >= GDK_0 && key <= GDK_9 ){
        m_keyjump_counter = 1;
        m_keyjump_num *= 10;
        m_keyjump_num += key - '0';

        CORE::core_set_command( "set_info", "", MISC::itostr( m_keyjump_num ) );
    }
}


//
// タブのロック
//
void View::lock()
{
    m_locked = true;
    if( m_toolbar ) m_toolbar->lock();
}


//
// タブのアンロック
//
void View::unlock()
{
    m_locked = false;
    if( m_toolbar ) m_toolbar->unlock();
}


// ツールバーボタン表示更新
void View::update_toolbar()
{
    if( ! m_toolbar ) return;
    m_toolbar->update();
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

        SESSION::set_popupmenu_shown( true );
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
    SESSION::set_popupmenu_shown( false );

    // もしviewがポップアップウィンドウ上にあって、かつ
    // メニューを消したときにマウスポインタが領域外にあれば自分自身をhide
    if( ! is_mouse_on_view() ) sig_hide_popup().emit();
}
