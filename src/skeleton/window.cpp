// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "window.h"

#include "config/globalconf.h"

#include "environment.h"
#include "global.h"
#include "session.h"
#include "dndmanager.h"
#include "command.h"

#include <gtk/gtkwindow.h>
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif // GDK_WINDOWING_WAYLAND

enum
{
    MGINFO_CHARS = MAX_MG_LNG * 2 + 16, // マウスジェスチャ表示欄の文字数

    FOCUSOUT_TIMEOUT = 250, // GNOME環境でフォーカスを外すまでの時間 ( msec )
    FOCUS_TIME = 100, // JDWindowにフォーカスを移してウィンドウサイズを復元するまでの時間 ( msec )
    UNGRAB_TIME = 100, // ブート直後にフォーカスをメインウィンドウに戻すまでの時間 ( msec )

    JDWIN_FOLDSIZE = 10  // 折りたたみ時に指定するウィンドウ高さ
};


// ウィンドウ状態
enum
{
    JDWIN_INIT = 0,
    JDWIN_NORMAL,     // 開いている
    JDWIN_FOLD,       // 折り畳んでいる
    JDWIN_EXPANDING,  // 展開中( clock_in() 参照 )
    JDWIN_UNMAX,      // 最大化 -> 通常
    JDWIN_UNMAX_FOLD, // 最大化 -> 折り畳み
    JDWIN_HIDE,       // hide 中
    JDWIN_UNGRAB      // ブート直後にungrabする( clock_in() 参照 )
};


//////////////////////////////////////////////


using namespace SKELETON;


// メッセージウィンドウでは m_mginfo が不要なので need_mginfo = false になる
JDWindow::JDWindow( const bool fold_when_focusout, const bool need_mginfo )
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL )
    , m_fold_when_focusout( fold_when_focusout )
    , m_boot( true )
    , m_enable_fold( m_fold_when_focusout )
    , m_mode( JDWIN_INIT )
{
    // ステータスバー
    m_label_stat.set_size_request( 0, -1 );
    m_label_stat.set_xalign( 0 );
    m_label_stat.set_selectable( true );
    m_label_stat.set_single_line_mode( true );
    m_label_stat.set_ellipsize( Pango::ELLIPSIZE_END );

    m_label_stat_ebox.add( m_label_stat );
    m_label_stat_ebox.set_visible_window( false );

    m_statbar.pack_start( m_label_stat_ebox );
    if( need_mginfo ){
        m_mginfo_ebox.add( m_mginfo );
        m_mginfo_ebox.set_visible_window( false );
        m_statbar.pack_start( m_mginfo_ebox, Gtk::PACK_SHRINK );
    }

    m_mginfo.set_width_chars( MGINFO_CHARS );
    m_mginfo.set_xalign( 0 );

    m_statbar.show_all_children();

    add( m_vbox );

    m_gtkwidget = GTK_WIDGET( gobj() );
    m_gtkwindow = GTK_WINDOW( gobj() );
    gpointer parent_class = g_type_class_peek_parent( G_OBJECT_GET_CLASS( gobj() ) );
    m_grand_parent_class = g_type_class_peek_parent( parent_class );

    auto context = m_label_stat.get_style_context();
    context->add_class( s_css_stat_label );
    context->add_provider( m_stat_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

    if( need_mginfo ) {
        context = m_mginfo.get_style_context();
        context->add_class( s_css_stat_label );
        context->add_provider( m_stat_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
    }

    try {
        m_stat_provider->load_from_data(
            ".red:not(:selected), .red:active:not(:selected) { color: white; background-color: red; }"
            ".green:not(:selected), .green:active:not(:selected) { color: white; background-color: green; }"
            ".blue:not(:selected), .blue:active:not(:selected) { color: white; background-color: blue; }" );
    }
    catch( Gtk::CssProviderError& err ) {
#ifdef _DEBUG
        std::cout << "ERROR:JDWindow::JDWindow css fail " << err.what() << std::endl;
#endif
    }
}


// windowの初期設定(サイズ変更や移動など)
void JDWindow::init_win()
{
    // フォーカスアウトで折り畳む場合
    if( m_fold_when_focusout ){

        m_scrwin = std::make_unique<Gtk::ScrolledWindow>();
        m_vbox_view = std::make_unique<SKELETON::JDVBox>();

        m_scrwin->set_size_request( 0, 0 );
        m_scrwin->set_policy( Gtk::POLICY_EXTERNAL, Gtk::POLICY_EXTERNAL );
        m_scrwin->add( *m_vbox_view );
        m_vbox.pack_remove_end( false, *m_scrwin, Gtk::PACK_EXPAND_WIDGET );

        set_skip_taskbar_hint( true );
        resize( get_width_win(), 1 );
        move_win( get_x_win(), get_y_win() );
        focus_out();

        property_window_position().set_value( Gtk::WIN_POS_NONE );
        set_transient( true );

        Glib::signal_idle().connect( sigc::mem_fun( *this, &JDWindow::slot_idle ) );
    }

    // 通常のウィンドウ
    else{
        resize( get_width_win(), get_height_win() );
        move_win( get_x_win(), get_y_win() );
        if( is_maximized_win() ) maximize_win();
        property_window_position().set_value( Gtk::WIN_POS_NONE );
        set_shown_win( true );

        Glib::signal_idle().connect( sigc::mem_fun( *this, &JDWindow::slot_idle ) );
    }
}


bool JDWindow::slot_idle()
{
    // ブート完了
    if( m_boot ){

#ifdef _DEBUG
        std::cout << "----------------\nJDWinow::slot_idle boot end mode = " << m_mode << std::endl;
#endif
        m_boot = false;
        move_win( get_x_win(), get_y_win() );

        if( m_fold_when_focusout ){

            // 遅延させて clock_in()の中でフォーカスをメインウィンドウに戻す
            if( m_mode == JDWIN_FOLD ){
                m_mode = JDWIN_UNGRAB;
                m_counter = 0;
            }
        }

        CORE::core_set_command( "window_boot_fin" );
    }

    return false;
}


// クロック入力
void JDWindow::clock_in()
{
    // 折りたたみ処理
    if( m_fold_when_focusout ){

        // 遅延リサイズ( focus_in()にある説明を参照 )
        if( m_mode == JDWIN_EXPANDING ){

            constexpr int waitcount = FOCUS_TIME / TIMER_TIMEOUT;
            ++m_counter;
            if( m_counter > waitcount && ! ( m_counter % waitcount ) ){

                if( get_height() < get_height_win() ) resize( get_width_win(), get_height_win() );
                else{
#ifdef _DEBUG
                    std::cout << "JDWindow::clock_in resize\n";
#endif
                    m_mode = JDWIN_NORMAL;
                    set_shown_win( true );
                    present();
                }
            }
        }

        // ブート直後にフォーカスをメインウィンドウに戻す
        else if( m_mode == JDWIN_UNGRAB ){

            ++m_counter;
            if( m_counter > UNGRAB_TIME / TIMER_TIMEOUT ){
#ifdef _DEBUG
                std::cout << "JDWindow::clock_in ungrab\n";
#endif

                // WMによってはフォーカスが外れない時があるのでlower()して
                // 無理矢理フォーカスを外す
                set_transient( false );
                get_window()->lower();
                set_transient( true );

                CORE::core_set_command( "restore_focus", "", "present" );
                m_mode = JDWIN_FOLD;
            }
        }

        // GNOME環境ではタスクトレイなどで切り替えたときに画像windowがフォーカスされてしまうので
        // メインウィンドウと画像ウィンドウが同時にフォーカスアウトしたら
        // 一時的に transient 指定を外す。メインウィンドウがフォーカスインしたときに
        // Admin::focus_out() で transient 指定を戻す
        using ENVIRONMENT::DesktopType;
        const DesktopType wm = ENVIRONMENT::get_wm();
        if( ( wm == DesktopType::gnome || wm == DesktopType::mate || wm == DesktopType::cinnamon )
            && ! SESSION::is_iconified_win_main() // メインウィンドウが最小化しているときに transient を外すとウィンドウが表示されなくなる
            && ! SESSION::is_focus_win_main() && ! is_focus_win() ){

            constexpr int waitcount = FOCUSOUT_TIMEOUT / TIMER_TIMEOUT;

            if( m_count_focusout < waitcount  ) ++m_count_focusout;
            if( m_count_focusout == waitcount ){

#ifdef _DEBUG
                std::cout << "JDWindow::clock_in focus timeout\n";
#endif
                set_transient( false );
                ++m_count_focusout;
            }
        }
        else m_count_focusout = 0;
    }
}


void JDWindow::set_spacing( int space )
{
    m_vbox.set_spacing( space );
}


void JDWindow::maximize_win()
{
    set_maximized_win( true );
    maximize();
}


void JDWindow::unmaximize_win()
{
    set_maximized_win( false );
    unmaximize();
}


void JDWindow::iconify_win()
{
    set_iconified_win( true );
    iconify();
}


//
// ウィンドウ移動
//
void JDWindow::move_win( const int x, const int y )
{
    if( ! CONFIG::get_manage_winpos() ) return;
#ifdef GDK_WINDOWING_WAYLAND
    // Wayland環境で実行するとウインドウの移動が意図した通りに機能しないので、JDimは移動に関与しない
    // ウインドウの移動や配置はWaylandコンポジタに任せる
    if( GDK_IS_WAYLAND_DISPLAY( get_display()->gobj() ) ) return;
#endif

#ifdef _DEBUG
    std::cout << "JDWindow::move_win "
              << "x = " << x << " y = " << y << std::endl;
#endif

    move( x, y );
    set_x_win( x );
    set_y_win( y );

    // compiz 環境などでは move() で指定した座標がズレるので補正する
    m_win_moved = true;
}


//
// ウィンドウ座標取得
//
void JDWindow::set_win_pos()
{
    if( ! get_window() ) return;

    int x,y;
    get_window()->get_root_origin( x, y );

#ifdef _DEBUG
    std::cout << "JDWindow::set_win_pos "
              << "x = " << x << " / " << get_x_win() << ", y = " << y << " / " << get_y_win() << std::endl;
#endif

    // compiz 環境などでは move() で指定した座標がズレるので補正する
    if( m_win_moved ){

        if( x != get_x_win() || y != get_y_win() ){

            // 補正量がmrgを越えたら補正を諦める
            const int mrg = 64;

            const int delta_x = x - get_x_win();
            const int delta_y = y - get_y_win();

            x = get_x_win();
            y = get_y_win();
            m_win_moved = false;

            if( abs( delta_x ) <= mrg && abs( delta_y ) <= mrg ){

                move( get_x_win() - delta_x, get_y_win() - delta_y );
#ifdef _DEBUG
                std::cout << "!!! moved x = " << get_x_win() << " y = " << get_y_win() << " dx = " << delta_x << " dy = " << delta_y << std::endl;
#endif
            }
        }
    }

    set_x_win( x );
    set_y_win( y );
}


// hide 中
bool JDWindow::is_hide() const
{
    return ( m_mode == JDWIN_HIDE );
}


void JDWindow::pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    if( m_fold_when_focusout ){
        m_vbox_view->pack_remove_start( unpack, child, options, padding );
        if( ! unpack ) m_vbox_view->show_all_children();
    }
    else{
        m_vbox.pack_remove_start( unpack, child, options, padding );
        if( ! unpack ) m_vbox.show_all_children();
    }
}


void JDWindow::pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    if( m_fold_when_focusout ){
        m_vbox_view->pack_remove_end( unpack, child, options, padding );
        if( ! unpack ) m_vbox_view->show_all_children();
    }
    else{
        m_vbox.pack_remove_end( unpack, child, options, padding );
        if( ! unpack ) m_vbox.show_all_children();
    }
}


// ステータスバー表示
void JDWindow::set_status( const std::string& stat )
{
    if( stat == m_status ) return;

    m_status = stat;

    m_label_stat.set_text( stat );
    m_label_stat_ebox.set_tooltip_text( stat );
}


// 一時的にステータスバーの表示を変える( マウスオーバーでのURL表示用 )
//
// 恒久的に変えてしまうと、マウススオ-バー中に ArticleViewBase では
// ないクラスから表示を変更された場合に本来の表示に戻せなくなる。
void JDWindow::set_status_temporary( const std::string& stat )
{
    if( stat == m_status ) return;

    m_label_stat.set_text( stat );
}

// 一時的に変えたステータスバーの表示を戻す
void JDWindow::restore_status()
{
    m_label_stat.set_text( m_status );
}


// マウスジェスチャ表示
void JDWindow::set_mginfo( const std::string& mginfo )
{
    if( m_mginfo.get_realized() ) {
        m_mginfo.set_text( mginfo );
    }
}


// ステータスの色を変える
void JDWindow::set_status_color( const std::string& color )
{
#ifdef _DEBUG
    std::cout << "JDWindow::set_status_color " << color << std::endl;
#endif

    auto context = m_label_stat.get_style_context();
    context->remove_class( "red" );
    context->remove_class( "green" );
    context->remove_class( "blue" );
    if( ! color.empty() ) context->add_class( color );

    if( m_mginfo.get_realized() ) {
        context = m_mginfo.get_style_context();
        context->remove_class( "red" );
        context->remove_class( "green" );
        context->remove_class( "blue" );
        if( ! color.empty() ) context->add_class( color );
    }
}


// メインウィンドウに対して transient 設定
void JDWindow::set_transient( bool set )
{
    if( m_fold_when_focusout ){

#ifdef _DEBUG
        std::cout << "JDWindow::set_transient set = " << set << " " << m_transient << std::endl;
#endif

        if( set && ! m_transient && CORE::get_mainwindow() ){
            set_transient_for( *CORE::get_mainwindow() );
            m_transient = true;
        }

        // transientを外す
        else if( ! set && m_transient ){
            unset_transient_for();
            m_transient = false;
        }
    }
}


//
// ダイアログ表示などでフォーカスが外れてもウインドウを畳まないようにする
//
void JDWindow::set_enable_fold( bool enable )
{
    if( m_fold_when_focusout && m_enable_fold != enable ){

#ifdef _DEBUG
        std::cout << "JDWindow::set_enable_fold " << enable << std::endl;
#endif

        m_enable_fold = enable;

        // XFCE 環境の場合はここでpresent()しておかないとフォーカスが外れる
        if( m_mode == JDWIN_NORMAL && m_enable_fold ){

            if( ENVIRONMENT::get_wm() == ENVIRONMENT::DesktopType::kde ) switch_admin();
            else present();
        }
    }
}


// フォーカスイン
void JDWindow::focus_in()
{
    // 折りたたみ処理
    if( m_fold_when_focusout ){

        // メインウィンドウが最小化しているときはメインウィンドウを開く
        if( SESSION::is_iconified_win_main() ){
            m_mode = JDWIN_UNGRAB;
            m_counter = 0;
            return;
        }

        if( ! m_enable_fold ) return;

        show();
        if( is_iconified_win() ) deiconify();

        if( ! is_maximized_win() && get_window() ){
            int x, y;
            get_window()->get_root_origin( x, y );
            if( x != get_x_win() || y != get_y_win() ) move_win( get_x_win(), get_y_win() );
        }


        // 開く
        //
        // GNOME環境では focus in 動作中に resize() が失敗する時が
        // あるので、遅延させて clock_in() の中でリサイズとpresentする
        if( ! is_maximized_win() && m_mode != JDWIN_EXPANDING ){
            m_mode = JDWIN_EXPANDING;
            m_counter = 0;
        }

#ifdef _DEBUG
        std::cout << "JDWindow::focus_in mode = " << m_mode
                  << " maximized = " << is_maximized_win()
                  << " iconified = " << is_iconified_win()  << std::endl;
#endif
    }

    else{
        show();
        if( is_iconified_win() ) deiconify();
        present();
    }

}


// フォーカスアウト
void JDWindow::focus_out()
{
    // 折りたたみ処理
    if( m_fold_when_focusout && m_enable_fold ){

        // ポップアップメニューを表示しているかD&D中はfocus_outしない
        if( SESSION::is_popupmenu_shown() ) return;
        if( CORE::DND_Now_dnd() ) return;

        // 最大化している時は通常状態に戻しておかないと表示されなくなる
        if( is_maximized_win() ) unmaximize();

        // 折り畳み
        if( m_mode != JDWIN_FOLD ){
            resize( get_width_win(), JDWIN_FOLDSIZE );
            m_mode = JDWIN_FOLD;
            set_shown_win( false );
            CORE::core_set_command( "restore_focus" );
        }

#ifdef _DEBUG
        std::cout << "JDWindow::focus_out mode = " << m_mode
                  << " maximized = " << is_maximized_win()
                  << " iconified = " << is_iconified_win() << std::endl;
#endif
    }
}


// フォーカスインイベント
bool JDWindow::on_focus_in_event( GdkEventFocus* event )
{
    set_focus_win( true );

    if( ! m_boot ){

        // 折りたたみ処理
        if( m_fold_when_focusout ){

#ifdef _DEBUG
            std::cout << "JDWindow::on_focus_in_event\n";
#endif

            if( m_mode != JDWIN_UNGRAB ) switch_admin();
        }
    }

    return Gtk::Window::on_focus_in_event( event );
}


// フォーカスアウトイベント
bool JDWindow::on_focus_out_event( GdkEventFocus* event )
{
    set_focus_win( false );

    if( ! m_boot ){

        // 折りたたみ処理
        if( m_fold_when_focusout ){

#ifdef _DEBUG
            std::cout << "JDWindow::on_focus_out_event\n";
#endif
            focus_out();
        }
    }

    return Gtk::Window::on_focus_out_event( event );
}


// Xボタンを押した
bool JDWindow::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "JDWindow::on_delete_event\n";
#endif

    // 折りたたみ処理
    if( m_fold_when_focusout ){

        if( is_maximized_win() ) unmaximize();

        else{

            // hideする前に座標保存
            if( ! is_iconified_win() && get_window() ) set_win_pos();

            hide();
            m_mode = JDWIN_HIDE;
            set_shown_win( false );
        }

        return true;
    }

    return Gtk::Window::on_delete_event( event );
}


// 最大、最小化
bool JDWindow::on_window_state_event( GdkEventWindowState* event )
{
    const bool maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
    const bool iconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;
    const bool fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;

#ifdef _DEBUG
    std::cout << "JDWindow::on_window_state_event : "
              << "maximized = " << is_maximized_win() << " -> " << maximized
              << " / iconified = " << is_iconified_win() << " -> " << iconified
              << " / full = " << is_full_win() << " -> " << fullscreen << std::endl;
#endif

    if( ! m_boot ){

        // 通常 -> アイコン化
        if( ! is_iconified_win() && iconified ) set_shown_win( false );

        // アイコン -> 通常
        else if( is_iconified_win() && ! iconified && m_mode != JDWIN_FOLD ) set_shown_win( true );

        // 通常 -> 最大化
        if( ! is_maximized_win() && maximized ){
            m_mode = JDWIN_NORMAL;
            set_shown_win( true );
        }

        // 最大 -> 折り畳み or 通常
        else if( is_maximized_win() && ! maximized ){

            // 最大 -> 折り畳み
            if( m_fold_when_focusout && m_mode == JDWIN_FOLD ) m_mode = JDWIN_UNMAX_FOLD;

            // 最大 -> 通常
            else m_mode = JDWIN_UNMAX;
        }

        // 通常 -> 全画面
        else if( ! is_full_win() && fullscreen ) set_full_win( true );

        // 全画面 -> 通常
        else if( is_full_win() && ! fullscreen ) set_full_win( false );

#ifdef _DEBUG
        std::cout << " mode = " << m_mode << std::endl;
#endif
    }

    set_maximized_win( maximized );
    set_iconified_win( iconified );

    return Gtk::Window::on_window_state_event( event );
}


/** @brief 移動、サイズ変更イベント
 *
 * @note 一部のウィンドウマネージャーでは、 `event->width/height` の値に
 * Client-side decoration (CSD)などの装飾が含まれている可能性があります。
 * `Gtk::Window::resize()`はウィンドウのコンテンツ領域（クライアント領域）
 * を指定したサイズに変更するため、装飾が含まれたサイズを渡すと、
 * 保存したサイズに正しく復元しません。代わりに、コンテンツ領域の
 * サイズ保存に適している `Gtk::Window::get_size()` を使用します。
 *
 * @note Wayland では、`on_window_state_event()`と`on_configure_event()`が
 * 呼び出される順序が X11 とは逆になっているため、`on_configure_event()`が
 * 呼び出された時点では`is_maximized_win()`などの結果が更新されていません。
 * その影響で、X11 と同じ条件でサイズ保存を行うと最大化やフルスクリーンの
 * サイズが保存されてしまい、元のサイズに復元できなくなっていました。
 * そのため、Wayland では代わりに`Gdk::Window::get_state()`を用いて
 * 最新の状態を取得し、保存するかどうかを判定するようにしました。
 */
bool JDWindow::on_configure_event( GdkEventConfigure* event )
{
    const int mrg = 16;
    int width_new = 1;
    int height_new = 1;
    get_size( width_new, height_new );
    int min_height = 0;
    if( m_scrwin ) min_height = m_vbox.get_height() - m_scrwin->get_height() + mrg;

    if( ! m_boot ){

#ifdef _DEBUG
    std::cout << "JDWindow::on_configure_event"
              << " boot = " << m_boot
              << " mode = " << m_mode
              << " x = " << event->x << " y = " << event->y
              << " w = " << width_new << " h = " << height_new
              << " min_height = " << min_height << std::endl;
#endif

        const auto is_window_normal = [this] {
            // Wayland では、最新の状態を取得して保存するかどうか判定します。
            if( ENVIRONMENT::get_display_type() == ENVIRONMENT::DisplayType::wayland ) {
                const auto state = get_window()->get_state();
                const auto mask = ( Gdk::WINDOW_STATE_MAXIMIZED | Gdk::WINDOW_STATE_ICONIFIED
                                    | Gdk::WINDOW_STATE_FULLSCREEN );
                return ! static_cast<bool>( state & mask );
            }

            return ! ( is_maximized_win() || is_iconified_win() || is_full_win() );
        };

        // 最大 -> 通常に戻る時はリサイズをキャンセル
        if( m_mode == JDWIN_UNMAX ) m_mode = JDWIN_NORMAL;
        else if( m_mode == JDWIN_UNMAX_FOLD ) m_mode = JDWIN_FOLD;

        // 最大、最小化しているときは除く
        else if( is_window_normal() ){

            set_win_pos();

            // サイズ変更
            if( ( ! m_fold_when_focusout || m_mode == JDWIN_NORMAL || m_mode == JDWIN_FOLD )
                && height_new > min_height
                ) {
                set_width_win( width_new );
                set_height_win( height_new );
            }
        }

#ifdef _DEBUG
    std::cout << "configure fin --> mode = " << m_mode << " show = " << is_shown_win()
              << " maximized = " << is_maximized_win()
              << " iconified = " << is_iconified_win()
              << " x = " << get_x_win() << " y = " << get_y_win()
              << " w = " << get_width_win() << " height = " << get_height_win() << std::endl;
#endif
    }

    return Gtk::Window::on_configure_event( event );
}


// uimなど、漢字変換モードの途中でctrl+qを押すとキーアクセレータが
// 優先されてJDが終了する問題があった。
//
// gedit-window.c の gedit_window_key_press_event を見ると
// gtk_window_propagate_key_event() を実行した後でキーアクセレータ
// の処理をするようにしていたのでJDもそうした。
bool JDWindow::on_key_press_event( GdkEventKey* event )
{
    if( gtk_window_propagate_key_event( m_gtkwindow, event ) ) return true;

    if( gtk_window_activate_key( m_gtkwindow, event ) ) return true;

#ifdef _DEBUG
    std::cout << "JDWindow::on_key_press_event key = " << event->keyval << std::endl;
    std::cout << m_grand_parent_class << " - " << m_gtkwidget << std::endl;
#endif

    return GTK_WIDGET_CLASS( m_grand_parent_class )->key_press_event( m_gtkwidget, event );
}
