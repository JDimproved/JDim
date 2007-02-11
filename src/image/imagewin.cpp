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
    IMGWIN_MAXIMIZING  // 最大化中
};


ImageWin::ImageWin()
    : m_boot( true ), m_mode( IMGWIN_NORMAL ), m_tab( NULL )
{
    // サイズ設定
    int x = SESSION::get_img_x();
    int y = SESSION::get_img_y();
    int w = SESSION::get_img_width();
    m_height = SESSION::get_img_height();
    m_maximized = SESSION::mes_maximized();

#ifdef _DEBUG
    std::cout << "MessageWin::MessageWin x y w h = " << x << " " << y << " " << w << " " << m_height << std::endl;
#endif

    move( x, y );
    resize( w, m_height );

    if( m_maximized ) maximize();

    m_scrwin.set_size_request( 0, 0 );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    add( m_vbox );
    show_all_children();

    property_window_position().set_value( Gtk::WIN_POS_NONE );
    if( CORE::get_mainwindow() ) set_transient_for( *CORE::get_mainwindow() );
}



ImageWin::~ImageWin()
{
    // ウィンドウサイズを保存
    int width, height;;
    int x = 0;
    int y = 0;

    get_size( width, height );
    if( ! has_focus() ) height = m_height;

    if( get_window() ) get_window()->get_root_origin( x, y );

#ifdef _DEBUG
    std::cout << "ImageWin::~ImageWin window size : x = " << x << " y = " << y << " w = " << width << " h = " << height
              << " max = " << m_maximized << std::endl;
#endif

    if( ! m_maximized ){
        SESSION::set_img_x( x );
        SESSION::set_img_y( y );
        SESSION::set_img_width( width );
        SESSION::set_img_height( height );
    }
    SESSION::set_img_maximized( m_maximized );
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



bool ImageWin::on_expose_event( GdkEventExpose* event )
{
#ifdef _DEBUG
    if( m_boot ) std::cout << "ImageWin::on_expose_event\n";
#endif

    // 最初の expose イベントが来たら起動完了とする
    if( m_boot ){

        m_boot = false;

        // ImageAdmin 経由で Coreにブートが終わったことを知らせるため
        // ダミーコマンドを送る
        IMAGE::get_admin()->set_command( "imgwin_boot_fin" );
    }

    return Gtk::Window::on_expose_event( event );
}



void ImageWin::focus_in()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_in\n";
#endif

    // 展開
    if( ! has_focus() ){

        resize( get_width(), m_height );
        m_mode = IMGWIN_EXPANDING;
    }

    present();
}


void ImageWin::focus_out()
{
#ifdef _DEBUG
    std::cout << "ImageWin::focus_out\n";
#endif

    // ポップアップメニューを表示しているかD&D中はfocus_outしない
    if( SESSION::is_popupmenu_shown() ) return;
    if( CORE::get_dnd_manager()->now_dnd() ) return;

    // 折り畳み
    if( has_focus() ){

        resize( get_width(), IMGWIN_FOLDSIZE );
        m_mode = IMGWIN_FOLDING;

        // もう折り畳んだということにして、 on_configure_eventの中ではなくて
        // ここで set_img_shown( false ) しておく
        SESSION::set_img_shown( false );
    }

}


bool ImageWin::on_focus_in_event( GdkEventFocus* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_focus_in_event in = " << event->in << std::endl;
#endif

    if( ! m_boot ) CORE::core_set_command( "switch_image" );

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
    std::cout << "\nImageWin::show_win\n";
#endif

    if( CORE::get_mainwindow() ) set_transient_for( *CORE::get_mainwindow() );
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
    std::cout << "\nImageWin::hide_win\n";
#endif

    // ダミーwindowを使ってtransientを外す
    Gtk::Window dummy;
    set_transient_for( dummy );

    get_window()->lower();
}



bool ImageWin::on_delete_event( GdkEventAny* event )
{
#ifdef _DEBUG
    std::cout << "ImageWin::on_delete_event\n";
#endif

    hide();
    CORE::core_set_command( "switch_leftview" );    

    return true;
}


bool ImageWin::on_window_state_event( GdkEventWindowState* event )
{
    if( ! m_boot ){
        m_maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
        m_mode = IMGWIN_MAXIMIZING;
    }

#ifdef _DEBUG
    std::cout << "ImageWin::on_window_state_event : maximized = " << m_maximized << std::endl;
#endif     

    return Gtk::Window::on_window_state_event( event );
}


bool ImageWin::on_configure_event( GdkEventConfigure* event )
{
    if( ! m_boot ){

        // 開いた
        if( m_mode == IMGWIN_EXPANDING ){
            m_mode = IMGWIN_NORMAL;
            SESSION::set_img_shown( true );
        }

        // 閉じた
        else if( m_mode == IMGWIN_FOLDING ) m_mode = IMGWIN_FOLD;

        // 最大最小化
        else if( m_mode == IMGWIN_MAXIMIZING ) m_mode = IMGWIN_NORMAL;

        // リサイズ中
        else if( get_height() > get_min_height() ) m_height = get_height();

#ifdef _DEBUG
        std::cout << "ImageWin::on_configure_event mode = " << m_mode << " height = " << m_height << std::endl;
#endif     
    }

    return Gtk::Window::on_configure_event( event );
}
