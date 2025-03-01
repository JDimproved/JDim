// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "admin.h"
#include "view.h"

#include "config/globalconf.h"
#include "history/historymanager.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscmsg.h"

#include "global.h"
#include "session.h"
#include "command.h"

using namespace SKELETON;

View::View( const std::string& url, const std::string& arg1 ,const std::string& arg2 )
    : m_url( url )
    , m_autoreload_mode( AUTORELOAD_NOT )
    , m_lockable( true )
    , m_writeable( true )
    , m_id_toolbar( 0 ) // 派生クラスでコンテキストが異なる
{}


const std::string& View::get_url_admin()
{
    return get_admin()->get_url();
}


//
// url_new に URL を変更
//
void View::set_url( const std::string& url_new )
{
    if( ! m_url.empty() && ! url_new.empty() && m_url != url_new ){

        // View履歴のURLを更新
        HISTORY::get_history_manager()->replace_url_viewhistory( m_url, url_new );

        // ツールバーのURLを更新
        get_admin()->set_command( "update_toolbar_url", m_url, url_new );

        m_url = url_new;
    }
}


//
// url 更新
//
// 移転があったときなどにadminから呼び出される
//
void View::update_url( const std::string& url_old, const std::string& url_new )
{
    if( m_url.rfind( url_old, 0 ) != 0 ) return;

    std::string url = url_new + m_url.substr( url_old.length() );

#ifdef _DEBUG
    std::cout << "View::update_url\n";
    std::cout << m_url << " -> " << url << std::endl;
#endif

    set_url( url );
}


// クロック入力
// clock_in_always()はviewの種類に依らず常に呼び出されるので重い処理を含めてはいけない
void View::clock_in_always()
{
    // タブ単位でのオートリロードモード
    if( m_autoreload_mode == AUTORELOAD_ONCE && inc_autoreload_counter() ) reload();

    // キーボード数字入力ジャンプ
    if( inc_keyjump_counter() ){
        goto_num( m_keyjump_num, 0 );
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
    if( m_autoreload_mode != AUTORELOAD_NOT && mode != AUTORELOAD_NOT ) return;

    m_autoreload_mode = mode;
    m_autoreload_sec = sec;
    m_autoreload_counter = 0;
}


// オートリロードのカウンタをリセット
void View::reset_autoreload_counter()
{
    m_autoreload_counter = 0;

    // オートリロードのモードがAUTORELOAD_ONCEの時はオートリロード停止
    if( m_autoreload_mode == AUTORELOAD_ONCE ) m_autoreload_mode = AUTORELOAD_NOT;
}


// 数字入力ジャンプカウンタのインクリメント
// 指定秒数を越えたら true を返す
bool View::inc_keyjump_counter()
{
    if( ! m_keyjump_counter ) return false;

    ++m_keyjump_counter;

    if( m_keyjump_counter > CONFIG::get_numberjmp_msec() / TIMER_TIMEOUT ) return true;

    return false;
}



// 数字入力ジャンプカウンタのリセット
void View::reset_keyjump_counter()
{
    m_keyjump_counter = 0;
    m_keyjump_num = 0;
}


// 数字入力ジャンプ用に sig_key_press() から呼び出す
bool View::release_keyjump_key( int key )
{
    // キーパッド対応
    if( key >= GDK_KEY_KP_0 && key <= GDK_KEY_KP_9 ) key = key - GDK_KEY_KP_0 + GDK_KEY_0;

    if( key >= GDK_KEY_0 && key <= GDK_KEY_9 ){
        m_keyjump_counter = 1;
        m_keyjump_num *= 10;
        m_keyjump_num += key - '0';

        CORE::core_set_command( "set_info", "", std::to_string( m_keyjump_num ) );
        return true;
    }

    return false;
}



// view 上にマウスポインタがあれば true
bool View::is_mouse_on_view() const
{
    bool ret = false;

    int x,y;
    MISC::get_pointer_at_window( get_window(), x, y );
    if( x < get_width() && x >= 0 && y < get_height() && y >= 0 ) ret = true;

#ifdef _DEBUG
    std::cout << "View::is_mouse_on_view ret = " << ret
              << " x= " << x << " y= " << y << " w= " << get_width() << " h= " << get_height() << std::endl;
#endif

    return ret;
}



/** @brief ポップアップメニューを指定位置に表示する
 *
 * @param[in] url           ビューのURL、またはメニューを識別するID文字列
 * @param[in] position      ポップアップメニューの表示位置
 * @param[in] anchor_widget メニューの表示位置に指定するウィジェット
 */
void View::show_popupmenu( const std::string& url, PopupMenuPosition position, Gtk::Widget* anchor_widget )
{
    // ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
    activate_act_before_popupmenu( url );

    Gtk::Menu* popupmenu = get_popupmenu( url );
    if( popupmenu ){

#ifdef _DEBUG
        std::cout << "View::show_popupmenu\n";
#endif

        const auto result = m_url_popup.insert( url );
        if( result.second ) {
            popupmenu->signal_map().connect( sigc::mem_fun( *this, &View::slot_map_popupmenu ) ); 
            popupmenu->signal_hide().connect( sigc::mem_fun( *this, &View::slot_hide_popupmenu ) );
        }

        if( position == PopupMenuPosition::view_top_left ) {
            // viewの左上とメニューの左上を揃える
            // メニューのサイズが大きく、ディスプレイの外にはみ出す場合でも、
            // 自動的に画面内に収まるように調整します。
            popupmenu->popup_at_widget( this, Gdk::GRAVITY_NORTH_WEST, Gdk::GRAVITY_NORTH_WEST, nullptr );
        }
        else if( position == PopupMenuPosition::mouse_pointer ) {
            // 現在のイベントに関連するマウスポインターの座標にメニューを表示する
            // nullptr を渡すことで現在のイベントから自動的に座標を取得します。
            popupmenu->popup_at_pointer( nullptr );
        }
        else if( position == PopupMenuPosition::toolbar_button && anchor_widget ) {
            // anchor_widgetの右下とメニューの右上を揃える
            // メニューのサイズが大きく、ディスプレイの外にはみ出す場合でも、
            // 自動的に画面内に収まるように調整します。
            popupmenu->popup_at_widget( anchor_widget, Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_NORTH_EAST, nullptr );
        }
        else {
            std::string errmsg = "View::show_popupmenu: Incorrect argument value: position=";
            errmsg.append( std::to_string( static_cast<int>( position ) ) );
            errmsg.append( static_cast<bool>( anchor_widget ) ? ", with anchor" : "without anchor" );
            MISC::ERRMSG( errmsg );
        }
    }
}


//
// ポップアップメニューがmapしたときに呼ばれるslot
//
void View::slot_map_popupmenu()
{
#ifdef _DEBUG
    std::cout << "View::slot_map_popupmenu\n";
#endif

    SESSION::set_popupmenu_shown( true );
}


//
// ポップアップメニューがhideしたときに呼ばれるslot
//
void View::slot_hide_popupmenu()
{
#ifdef _DEBUG
    std::cout << "View::slot_hide_popupmenu\n";
#endif

    SESSION::set_popupmenu_shown( false );

    // もしviewがポップアップウィンドウ上にあって、かつ
    // メニューを消したときにマウスポインタが領域外にあれば自分自身をhide
    if( ! is_mouse_on_view() ) sig_hide_popup().emit();
}


//
// ラベルやステータスバーの色
//
std::string View::get_color() const
{
    if( is_broken() ) return "red";
    else if( is_old() ) return "blue";
    else if( is_overflow() ) return "green";

    return "";
}


/** @brief 最小の幅と自然な幅の初期値を取得する
 *
 * @param[out] minimum_width ウィジェットの最小の幅
 * @param[out] natural_width ウィジェットの自然な幅
 */
void View::get_preferred_width_vfunc( int& minimum_width, int& natural_width ) const
{
    minimum_width = 0;
    natural_width = width_client();
}


/** @brief 最小の高さと自然な高さの初期値を取得する
 *
 * @param[out] minimum_width ウィジェットの最小の高さ
 * @param[out] natural_width ウィジェットの自然な高さ
 */
void View::get_preferred_height_vfunc( int& minimum_height, int& natural_height ) const
{
    minimum_height = 0;
    natural_height = height_client();
}
