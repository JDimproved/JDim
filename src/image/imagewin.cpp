// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imagewin.h"

#include "session.h"
#include "dndmanager.h"
#include "command.h"
#include "global.h"


using namespace IMAGE;

#define IMGWIN_FOLDSIZE 10

// タイトルバーの高さの取得方法が分からないのでとりあえずdefineしておく
#define IMGWIN_TITLBARHEIGHT 16

#define UNGRAB_TIME 100 //msec


// ウィンドウ状態
enum
{
    IMGWIN_NORMAL = 0, // 開いている
    IMGWIN_FOLD,       // 折り畳んでいる
    IMGWIN_EXPANDING,  // 展開中
    IMGWIN_HIDE,       // hide 中
    IMGWIN_UNGRAB      // ブート直後にungrab
};


ImageWin::ImageWin()
    : Gtk::Window(),
      m_boot( true ),
      m_enable_close( true ),
      m_transient( false ),
      m_maximized( false ),
      m_iconified( false ),
      m_tab( NULL )
{
    // サイズ設定
    m_x = SESSION::get_img_x();
    m_y = SESSION::get_img_y();
    m_width = SESSION::get_img_width();
    m_height = SESSION::get_img_height();

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << m_x << " " << m_y << " " << m_width << " " << m_height << std::endl;
#endif

    m_dummywin.resize( 1, 1 );
    m_dummywin.move( -1, -1 );
    m_dummywin.hide();
    m_dummywin.set_skip_taskbar_hint( true );

    set_skip_taskbar_hint( true );
    move( m_x, m_y );
    focus_out();

#if GTKMMVER >= 260
    m_statbar.pack_start( m_label_stat, Gtk::PACK_SHRINK );
#endif

    m_vbox_view.pack_end( m_statbar, Gtk::PACK_SHRINK );

    m_scrwin.set_size_request( 0, 0 );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_scrwin.add( m_vbox_view );

    m_vbox.pack_end( m_scrwin );

    add( m_vbox );
    show_all_children();

    property_window_position().set_value( Gtk::WIN_POS_NONE );
    set_transient( true );

    Glib::signal_idle().connect( sigc::mem_fun( *this, &ImageWin::slot_idle ) );
}


ImageWin::~ImageWin()
{
#ifdef _DEBUG
    std::cout << "ImageWin::~ImageWin window size : x = " << m_x << " y = " << m_y
              << " w = " << m_width << " h = " << m_height << std::endl;
#endif

    // ウィンドウサイズを保存
    SESSION::set_img_x( m_x );
    SESSION::set_img_y( m_y );
    SESSION::set_img_width( m_width );
    SESSION::set_img_height( m_height );

    SESSION::set_img_shown( false );
    CORE::core_set_command( "restore_focus" );
}


void ImageWin::clock_in()
{
    // 遅延リサイズ( focus_in()にある説明を参照 )
    if( m_mode == IMGWIN_EXPANDING ){

        ++m_counter;
        if( m_counter > 5 ){ 

            if( get_height() < m_height ) resize( m_width, m_height );
            else{
                m_mode = IMGWIN_NORMAL;
                SESSION::set_img_shown( true );
                present();
            }
        }
    }

    // ブート直後にフォーカスをメインウィンドウに戻す
    else if( m_mode == IMGWIN_UNGRAB ){

        ++m_counter;
        if( m_counter > UNGRAB_TIME / TIMER_TIMEOUT ){ 

            CORE::core_set_command( "restore_focus" );
            m_mode = IMGWIN_FOLD;
        }
    }
}


bool ImageWin::slot_idle()
{
    // ブート完了
    if( m_boot ){

#ifdef _DEBUG
        std::cout << "----------------\nImageWin::slot_idle boot end mode = " << m_mode << std::endl;
#endif
        m_boot = false;

        // 遅延させて clock_in()の中でフォーカスをメインウィンドウに戻す
        if( m_mode == IMGWIN_FOLD ){
            m_mode = IMGWIN_UNGRAB;
            m_counter = 0;
        }

        // ImageAdmin 経由で Coreにブートが終わったことを知らせるため
        // ダミーコマンドを送る
        IMAGE::get_admin()->set_command( "imgwin_boot_fin" );
    }

    return false;
}


void ImageWin::set_transient( bool set )
{
    if( set && ! m_transient && CORE::get_mainwindow() ){

#ifdef _DEBUG
    std::cout << "ImageWin::set_transient set = " << set << std::endl;
#endif
        set_transient_for( *CORE::get_mainwindow() );
        m_transient = true;
    }

    // ダミーwindowを使ってtransientを外す
    else if( ! set && m_transient ){

#ifdef _DEBUG
    std::cout << "ImageWin::set_transient set = " << set << std::endl;
#endif
        set_transient_for( m_dummywin );
        m_transient = false;
    }
}



// hide 中
const bool ImageWin::is_hide()
{
    return ( m_mode == IMGWIN_HIDE );
}


//
// フォーカスされている = 開いている
//
const bool ImageWin::has_focus()
{
    return ( m_mode == IMGWIN_NORMAL );
}



//
// ウィンドウを折り畳んだときの高さ
//
// (TODO) ウィンドウの最小高さの取得方法が分からないのでタブの高さを使っているのをなんとかする
//
int ImageWin::get_min_height()
{
    if( m_tab && m_tab->get_height() > 1 ) return m_tab->get_height() + IMGWIN_TITLBARHEIGHT;

    return 0;
}


void ImageWin::pack_remove( bool unpack, Gtk::Widget& tab, Gtk::Widget& view )
{
#ifdef _DEBUG
    std::cout << "ImageWin::pack_remove remove - " << unpack << std::endl;
#endif

    m_tab = &tab;

    m_vbox.pack_remove_end( unpack, tab, Gtk::PACK_SHRINK );
    m_vbox_view.pack_remove_end( unpack, view );

    if( ! unpack ) m_vbox.show_all_children();
}


// ステータスバー表示
void ImageWin::set_status( const std::string& stat )
{
#if GTKMMVER <= 240
    m_statbar.push( stat );
#else
    m_label_stat.set_text( stat );
#endif
}


// フォーカスイン
void ImageWin::focus_in()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_in mode = " << m_mode 
              << " maximized = " << m_maximized << " iconified = " << m_iconified << std::endl;
#endif

    SESSION::set_focus_win_img( true );

    show();
    if( m_iconified ) deiconify();

    // 開く
    //
    // GNOME環境では focus in 動作中に resize() が失敗する時が
    // あるので、遅延させて clock_in() の中でリサイズする
    if( m_enable_close && ! m_maximized ){
        m_mode = IMGWIN_EXPANDING;
        m_counter = 0;
    }
}


// フォーカスアウト
void ImageWin::focus_out()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_out mode = " << m_mode
              << " maximized = " << m_maximized << " iconified = " << m_iconified << std::endl;
#endif

    SESSION::set_focus_win_img( false );

    // ポップアップメニューを表示しているかD&D中はfocus_outしない
    if( SESSION::is_popupmenu_shown() ) return;
    if( CORE::get_dnd_manager()->now_dnd() ) return;

    if( m_maximized ) unmaximize();

    // 折り畳み
    if( m_enable_close ){
        resize( m_width, IMGWIN_FOLDSIZE );
        m_mode = IMGWIN_FOLD;
        SESSION::set_img_shown( false );
    }
}


// フォーカスインイベント
bool ImageWin::on_focus_in_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_in_event\n";
#endif

    if( ! m_boot && m_mode != IMGWIN_UNGRAB ) CORE::core_set_command( "switch_image" );

    return Gtk::Window::on_focus_in_event( event );
}


// フォーカスアウトイベント
bool ImageWin::on_focus_out_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_out_event\n";
#endif

    if( ! m_boot ) focus_out();

    return Gtk::Window::on_focus_out_event( event );
}


// Xボタンを押した
bool ImageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_delete_event\n";
#endif

    if( m_maximized ) unmaximize();

    else{

        hide();
        m_mode = IMGWIN_HIDE;
        SESSION::set_img_shown( false );
    }

    return true;
}


// 最大、最小化
bool ImageWin::on_window_state_event( GdkEventWindowState* event )
{
    if( ! m_boot ){

        m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
        m_iconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;

        if( m_iconified ){}

        else if( m_maximized ){
            m_mode = IMGWIN_NORMAL;
            SESSION::set_img_shown( true );
        }

#ifdef _DEBUG
        std::cout << "ImageWin::on_window_state_event : "
                  << " maximized = " << m_maximized << " iconified = " << m_iconified << std::endl;
#endif     
    }

    return Gtk::Window::on_window_state_event( event );
}


// サイズ変更
bool ImageWin::on_configure_event( GdkEventConfigure* event )
{
    // 最大、最小化しているときは除く
    if( ! m_boot && ! m_maximized && ! m_iconified
        && m_mode == IMGWIN_FOLD
        && get_height() > get_min_height() ){

        if( get_window() ) get_window()->get_root_origin( m_x, m_y );

        m_width = get_width();
        m_height = get_height();

#ifdef _DEBUG
        std::cout << "ImageWin::on_configure_event resizing w = "
                  << m_width << " height = " << m_height << std::endl;
#endif     
    }

    return Gtk::Window::on_configure_event( event );
}
