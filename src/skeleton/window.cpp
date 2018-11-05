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

#if GTKMM_CHECK_VERSION(3,0,0)
constexpr const char* JDWindow::s_css_stat_label;
#endif

// メッセージウィンドウでは m_mginfo が不要なので need_mginfo = false になる
JDWindow::JDWindow( const bool fold_when_focusout, const bool need_mginfo )
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
      m_gtkwidget( NULL ),
      m_gtkwindow( NULL ),
      m_grand_parent_class( NULL ),
      m_win_moved( false ),
      m_fold_when_focusout( fold_when_focusout ),
      m_boot( true ),
      m_enable_fold( m_fold_when_focusout ),
      m_transient( false ),
      m_mode( JDWIN_INIT ),
      m_count_focusout( 0 ),
      m_dummywin( NULL ),
      m_scrwin( NULL ),
      m_vbox_view( NULL )
{
    // ステータスバー
#if GTKMM_CHECK_VERSION(2,5,0)
    m_label_stat.set_size_request( 0, -1 );
    m_label_stat.set_alignment( Gtk::ALIGN_START );
    m_label_stat.set_selectable( true );
    m_label_stat.set_single_line_mode( true );

    m_label_stat_ebox.add( m_label_stat );
    m_label_stat_ebox.set_visible_window( false );

    m_statbar.pack_start( m_label_stat_ebox );
    if( need_mginfo ){
        m_mginfo_ebox.add( m_mginfo );
        m_mginfo_ebox.set_visible_window( false );
        m_statbar.pack_start( m_mginfo_ebox, Gtk::PACK_SHRINK );
    }

    m_mginfo.set_width_chars( MGINFO_CHARS );
    m_mginfo.set_alignment( Gtk::ALIGN_START );
#else
    if( need_mginfo ) m_statbar.pack_start( m_mginfo );
#endif

    m_statbar.show_all_children();

    add( m_vbox );

    m_gtkwidget = GTK_WIDGET( gobj() );
    m_gtkwindow = GTK_WINDOW( gobj() );
    gpointer parent_class = g_type_class_peek_parent( G_OBJECT_GET_CLASS( gobj() ) );
    m_grand_parent_class = g_type_class_peek_parent( parent_class );

#if GTKMM_CHECK_VERSION(3,0,0)
    auto context = m_label_stat.get_style_context();
    context->add_class( s_css_stat_label );
    context->add_provider( m_stat_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

    if( need_mginfo ) {
        context = m_mginfo.get_style_context();
        context->add_class( s_css_stat_label );
        context->add_provider( m_stat_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
    }
#endif // GTKMM_CHECK_VERSION(3,0,0)
}


JDWindow::~JDWindow()
{
    if( m_dummywin ) delete m_dummywin;
    if( m_vbox_view ) delete m_vbox_view;
    if( m_scrwin ) delete m_scrwin;
}


// windowの初期設定(サイズ変更や移動など)
void JDWindow::init_win()
{
    // フォーカスアウトで折り畳む場合
    if( m_fold_when_focusout ){

        m_scrwin = new Gtk::ScrolledWindow();
        m_vbox_view = new SKELETON::JDVBox();

        m_scrwin->set_size_request( 0, 0 );
        m_scrwin->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
        m_scrwin->add( *m_vbox_view );
        m_vbox.pack_remove_end( false, *m_scrwin, Gtk::PACK_EXPAND_WIDGET );

        m_dummywin = new Gtk::Window();
        m_dummywin->resize( 1, 1 );
        m_dummywin->move( -1, -1 );
        m_dummywin->hide();
        m_dummywin->set_skip_taskbar_hint( true );

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

        int waitcount = 0;

        // 遅延リサイズ( focus_in()にある説明を参照 )
        if( m_mode == JDWIN_EXPANDING ){

            waitcount = FOCUS_TIME / TIMER_TIMEOUT;
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
        if( ENVIRONMENT::get_wm() == ENVIRONMENT::WM_GNOME
            && ! SESSION::is_iconified_win_main() // メインウィンドウが最小化しているときに transient を外すとウィンドウが表示されなくなる
            && ! SESSION::is_focus_win_main() && ! is_focus_win() ){

            waitcount = FOCUSOUT_TIMEOUT / TIMER_TIMEOUT;

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
bool JDWindow::is_hide()
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

#if GTKMM_CHECK_VERSION(2,5,0)
    m_label_stat.set_text( stat );
#if GTKMM_CHECK_VERSION(2,12,0)
    m_label_stat_ebox.set_tooltip_text( stat );
#else
    m_tooltip.set_tip( m_label_stat_ebox, stat );
#endif
#else
    m_statbar.push( stat );
#endif
}


// 一時的にステータスバーの表示を変える( マウスオーバーでのURL表示用 )
//
// 恒久的に変えてしまうと、マウススオ-バー中に ArticleViewBase では
// ないクラスから表示を変更された場合に本来の表示に戻せなくなる。
void JDWindow::set_status_temporary( const std::string& stat )
{
    if( stat == m_status ) return;

#if GTKMM_CHECK_VERSION(2,5,0)
    m_label_stat.set_text( stat );
#else
    m_statbar.push( stat );
#endif
}

// 一時的に変えたステータスバーの表示を戻す
void JDWindow::restore_status()
{
#if GTKMM_CHECK_VERSION(2,5,0)
    m_label_stat.set_text( m_status );
#else
    m_statbar.push( m_status );
#endif
}


// マウスジェスチャ表示
void JDWindow::set_mginfo( const std::string& mginfo )
{
#if GTKMM_CHECK_VERSION(2,20,0)
    const bool realized = m_mginfo.get_realized();
#else
    const bool realized = m_mginfo.is_realized();
#endif
    if( realized ) {
        m_mginfo.set_text( mginfo );
    }
}


// ステータスの色を変える
void JDWindow::set_status_color( const std::string& color )
{
#ifdef _DEBUG
    std::cout << "JDWindow::set_status_color " << color << std::endl;
#endif

#if GTKMM_CHECK_VERSION(3,0,0)
    Glib::ustring css;
    if( color.empty() ) {
        // テキスト部分が上手く配色されないGTKテーマがあるので明示的に設定する
        css = Glib::ustring::compose( u8".%1:not(:selected) { color: unset; }", s_css_stat_label );
    }
    else {
        css = Glib::ustring::compose(
            u8".%1:not(:selected), %1:active:not(:selected) { color: white; background-color: %2; }",
            s_css_stat_label, Gdk::RGBA( color ).to_string() );
    }
    try {
        m_stat_provider->load_from_data( css );
    }
    catch( Gtk::CssProviderError& err ) {
#ifdef _DEBUG
        std::cout << "ERROR:JDWindow::set_status_color fail " << err.what() << std::endl;
#endif
    }

#elif GTKMM_CHECK_VERSION(2,5,0)
    if( color.empty() ){

        if( m_label_stat_ebox.get_visible_window() ){
            m_label_stat.unset_fg( Gtk::STATE_NORMAL );
            m_mginfo.unset_fg( Gtk::STATE_NORMAL );

            m_label_stat_ebox.set_visible_window( false );
            m_mginfo_ebox.set_visible_window( false );
        }
    }
    else{

        m_label_stat.modify_fg( Gtk::STATE_NORMAL, Gdk::Color( "white" ) );
        m_mginfo.modify_fg( Gtk::STATE_NORMAL, Gdk::Color( "white" ) );

        m_label_stat_ebox.set_visible_window( true );
        m_label_stat_ebox.modify_bg( Gtk::STATE_NORMAL, Gdk::Color( color ) );
        m_label_stat_ebox.modify_bg( Gtk::STATE_ACTIVE, Gdk::Color( color ) );

        m_mginfo_ebox.set_visible_window( true );
        m_mginfo_ebox.modify_bg( Gtk::STATE_NORMAL, Gdk::Color( color ) );
        m_mginfo_ebox.modify_bg( Gtk::STATE_ACTIVE, Gdk::Color( color ) );
    }
#endif
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

        // ダミーwindowを使ってtransientを外す
        else if( ! set && m_transient ){
            set_transient_for( *m_dummywin );
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

            if( ENVIRONMENT::get_wm() == ENVIRONMENT::WM_KDE ) switch_admin();
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
            if( ! is_maximized_win() && ! is_iconified_win() && get_window() ) set_win_pos();

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


// 移動、サイズ変更イベント
bool JDWindow::on_configure_event( GdkEventConfigure* event )
{
    const int mrg = 16;
    const int width_new = event->width;
    const int height_new = event->height;
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

        // 最大 -> 通常に戻る時はリサイズをキャンセル
        if( m_mode == JDWIN_UNMAX ) m_mode = JDWIN_NORMAL;
        else if( m_mode == JDWIN_UNMAX_FOLD ) m_mode = JDWIN_FOLD;

        // 最大、最小化しているときは除く
        else if( ! is_maximized_win() && ! is_iconified_win() && ! is_full_win() ){

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
