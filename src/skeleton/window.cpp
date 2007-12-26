// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "window.h"

#include "global.h"
#include "session.h"
#include "dndmanager.h"
#include "command.h"


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
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
      m_fold_when_focusout( fold_when_focusout ),
      m_boot( fold_when_focusout ),
      m_enable_fold( m_fold_when_focusout ),
      m_transient( false ),
      m_mode( JDWIN_INIT ),
      m_count_focusout( 0 ),
      m_dummywin( NULL ),
      m_scrwin( NULL ),
      m_vbox_view( NULL )
{
    // ステータスバー
#if GTKMMVER <= 240
    if( need_mginfo ) m_statbar.pack_start( m_mginfo );
#else
    m_label_stat.set_size_request( 0, -1 );
    m_label_stat.set_alignment( Gtk::ALIGN_LEFT );
    m_label_stat.set_selectable( true );

    m_label_stat_ebox.add( m_label_stat );
    m_label_stat_ebox.set_visible_window( false );

    m_statbar.pack_start( m_label_stat_ebox );
    if( need_mginfo ) m_statbar.pack_start( m_mginfo, Gtk::PACK_SHRINK );

    m_mginfo.set_width_chars( MGINFO_CHARS );
    m_mginfo.set_alignment( Gtk::ALIGN_LEFT );
#endif

    m_statbar.show_all_children();

    add( m_vbox );
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
        move( get_x_win(), get_y_win() );
        focus_out();

        property_window_position().set_value( Gtk::WIN_POS_NONE );
        set_transient( true );

        Glib::signal_idle().connect( sigc::mem_fun( *this, &JDWindow::slot_idle ) );
    }

    // 通常のウィンドウ
    else{
        resize( get_width_win(), get_height_win() );
        move( get_x_win(), get_y_win() );
        if( is_maximized_win() ) maximize_win();
        property_window_position().set_value( Gtk::WIN_POS_NONE );
        set_shown_win( true );
    }
}


bool JDWindow::slot_idle()
{
    // ブート完了
    if( m_fold_when_focusout && m_boot ){

#ifdef _DEBUG
        std::cout << "----------------\nJDWinow::slot_idle boot end mode = " << m_mode << std::endl;
#endif
        m_boot = false;

        // 遅延させて clock_in()の中でフォーカスをメインウィンドウに戻す
        if( m_mode == JDWIN_FOLD ){
            m_mode = JDWIN_UNGRAB;
            m_counter = 0;
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
        if( SESSION::get_wm() == SESSION::WM_GNOME
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


// hide 中
const bool JDWindow::is_hide()
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
#if GTKMMVER <= 240
    m_statbar.push( stat );
#else
    m_label_stat.set_text( stat );
    m_tooltip.set_tip( m_label_stat_ebox, stat );
#endif
}


// マウスジェスチャ表示
void JDWindow::set_mginfo( const std::string& mginfo )
{
    if( m_mginfo.is_realized() ) m_mginfo.set_text( mginfo );
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

            if( SESSION::get_wm() == SESSION::WM_KDE ) switch_admin();
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
            if( x != get_x_win() || y != get_y_win() ) move( get_x_win(), get_y_win() );
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
        if( CORE::get_dnd_manager()->now_dnd() ) return;

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
            if( ! is_maximized_win() && ! is_iconified_win() && get_window() ){
                int x, y;
                get_window()->get_root_origin( x, y );
                set_x_win( x );
                set_y_win( y );
            }

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
    bool maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
    bool iconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;

#ifdef _DEBUG
    std::cout << "JDWindow::on_window_state_event : " 
              << "maximized = " << is_maximized_win() << " -> " << maximized
              << " / iconified = " << is_iconified_win() << " -> " << iconified << std::endl;
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
    int min_height = 0;
    if( m_scrwin ) min_height = m_vbox.get_height() - m_scrwin->get_height() + mrg;

#ifdef _DEBUG
    std::cout << "JDWindow::on_configure_event\n"
              << " w = " << get_width() << " h = " << get_height() << " min_height = " << min_height << "\n------->\n";
#endif

    if( ! m_boot ){

        // 最大 -> 通常に戻る時はリサイズをキャンセル
        if( m_mode == JDWIN_UNMAX ) m_mode = JDWIN_NORMAL;
        else if( m_mode == JDWIN_UNMAX_FOLD ) m_mode = JDWIN_FOLD;

        // 最大、最小化しているときは除く
        else if( ! is_maximized_win() && ! is_iconified_win() ){

            if( get_window() ){
                int x,y;
                get_window()->get_root_origin( x, y );
                set_x_win( x );
                set_y_win( y );
            }

            // サイズ変更
            if( ( ! m_fold_when_focusout || m_mode == JDWIN_FOLD ) && get_height() > min_height ){
                set_width_win( get_width() );
                set_height_win( get_height() );
            }
        }
    }

#ifdef _DEBUG
    std::cout << " mode = " << m_mode << " show = " << is_shown_win() << std::endl
              << " maximized = " << is_maximized_win()
              << " iconified = " << is_iconified_win()
              << " x = " << get_x_win() << " y = " << get_y_win()
              << " w = " << get_width_win() << " height = " << get_height_win() << std::endl;
#endif     

    return Gtk::Window::on_configure_event( event );
}
