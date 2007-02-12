// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imagewin.h"

#include "session.h"
#include "dndmanager.h"
#include "command.h"


using namespace IMAGE;

#define IMGWIN_FOLDSIZE 10

// タイトルバーの高さの取得方法が分からないのでとりあえずdefineしておく
#define IMGWIN_TITLBARHEIGHT 16


// ウィンドウ状態
enum
{
    IMGWIN_NORMAL = 0, // 開いている
    IMGWIN_FOLDING,    // 折り畳み中
    IMGWIN_FOLD,       // 折り畳んでいる
    IMGWIN_EXPANDING,  // 展開中
    IMGWIN_HIDE        // hide 中
};


ImageWin::ImageWin()
    : Gtk::Window(), m_boot( true ), m_maximized( false ), m_tab( NULL )
{
    // サイズ設定
    m_x = SESSION::get_img_x();
    m_y = SESSION::get_img_y();
    m_width = SESSION::get_img_width();
    m_height = SESSION::get_img_height();

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << m_x << " " << m_y << " " << m_width << " " << m_height << std::endl;
#endif

    move( m_x, m_y );
    resize( m_width, m_height );
    m_mode = IMGWIN_NORMAL;
    SESSION::set_img_shown( true );

    m_scrwin.set_size_request( 0, 0 );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

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


bool ImageWin::slot_idle()
{
    // ブート完了
    if( m_boot ){

#ifdef _DEBUG
    std::cout << "----------------\nImageWin::slot_idle boot end\n";
#endif
        m_boot = false;

        // ImageAdmin 経由で Coreにブートが終わったことを知らせるため
        // ダミーコマンドを送る
        IMAGE::get_admin()->set_command( "imgwin_boot_fin" );
    }

    // 開いた
    else if( m_mode == IMGWIN_EXPANDING ){

#ifdef _DEBUG
    std::cout << "----------------\nImageWin::slot_idle expanded\n";
#endif

        m_mode = IMGWIN_NORMAL;
        SESSION::set_img_shown( true );
        present();
    }

    // 閉じた
    else if( m_mode == IMGWIN_FOLDING ){

#ifdef _DEBUG
        std::cout << "----------------\nImageWin::slot_idle folded\n";
#endif

        m_mode = IMGWIN_FOLD;
    }

    return false;
}


void ImageWin::set_transient( bool set )
{
#ifdef _DEBUG
    std::cout << "ImageWin::set_transient set = " << set << std::endl;
#endif

    if( set && CORE::get_mainwindow() ) set_transient_for( *CORE::get_mainwindow() );

    // ダミーwindowを使ってtransientを外す
    else set_transient_for( m_dummywin );
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

    m_vbox.pack_remove_start( unpack, tab, Gtk::PACK_SHRINK );

    if( unpack ){

        // ScrolledWindow は Gtk::Viewport を作って widget を add するときがあるので注意
        Gtk::Viewport *vport = dynamic_cast< Gtk::Viewport* >( m_scrwin.get_child() );
        if( vport ) vport->remove();
        else m_scrwin.remove();
    }
    else m_scrwin.add( view );

    m_vbox.pack_remove_start( unpack, m_scrwin );

    if( ! unpack ) m_vbox.show_all_children();
}


void ImageWin::focus_in()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_in mode = " << m_mode << std::endl;
#endif

    if( m_maximized ) return;

    if( ( m_mode == IMGWIN_FOLD
          || m_mode == IMGWIN_FOLDING // folding 中にキャンセル
          || m_mode == IMGWIN_HIDE )
        ){

        resize( m_width, m_height );

        m_mode = IMGWIN_EXPANDING;
        Glib::signal_idle().connect( sigc::mem_fun( *this, &ImageWin::slot_idle ) );
    }
}


void ImageWin::focus_out()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_out mode = " << m_mode << std::endl;;
#endif

    // ポップアップメニューを表示しているかD&D中はfocus_outしない
    if( SESSION::is_popupmenu_shown() ) return;
    if( CORE::get_dnd_manager()->now_dnd() ) return;

    // 折り畳み
    if( m_mode == IMGWIN_NORMAL
        || m_mode == IMGWIN_EXPANDING // expanding 中にキャンセル
        ){

        resize( m_width, IMGWIN_FOLDSIZE );
        m_mode = IMGWIN_FOLDING;
        Glib::signal_idle().connect( sigc::mem_fun( *this, &ImageWin::slot_idle ) );
    }

    // もう折り畳んだということにして、 on_configure_eventの中ではなくて
    // ここで set_img_shown( false ) しておく
    SESSION::set_img_shown( false );
}


bool ImageWin::on_focus_in_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_in_event in = " << event->in << std::endl;
#endif

    if( ! m_boot ){

        if( ! has_focus() ) CORE::core_set_command( "switch_image" );

    }

    return Gtk::Window::on_focus_in_event( event );
}


bool ImageWin::on_focus_out_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_out_event in = " << event->in << std::endl;
#endif

    if( ! m_boot ) focus_out();

    return Gtk::Window::on_focus_out_event( event );
}


//
// ImageWin::hide_win() でwindowを隠した後に show する
//
void ImageWin::show_win()
{
#ifdef _DEBUG
    std::cout << "ImageWin::show_win\n";
#endif

    set_transient( true );
    CORE::core_set_command( "restore_focus" );
}


//
// hide する
//
// (注意) 実際には hide しないで transient 指定を外して lower するだけ
//
void ImageWin::hide_win()
{
#ifdef _DEBUG
    std::cout << "ImageWin::hide_win\n";
#endif

    set_transient( false );
    get_window()->lower();
}



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


bool ImageWin::on_window_state_event( GdkEventWindowState* event )
{
    if( ! m_boot ){

        m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
        if( m_maximized ){
            m_mode = IMGWIN_EXPANDING;
            Glib::signal_idle().connect( sigc::mem_fun( *this, &ImageWin::slot_idle ) );
        }

#ifdef _DEBUG
        std::cout << "ImageWin::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     
    }

    return Gtk::Window::on_window_state_event( event );
}


bool ImageWin::on_configure_event( GdkEventConfigure* event )
{
    // サイズ変更
    if( ! m_boot && ! m_maximized
        && ( m_mode == IMGWIN_FOLD )
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
