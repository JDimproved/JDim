// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageview.h"
#include "imagearea.h"

#include "skeleton/msgdiag.h"

#include "dbimg/img.h"

#include "config/globalconf.h"

#include "command.h"
#include "httpcode.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

#include <sstream>

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif

enum
{
    IMGWIN_REDRAWTIME = 250 // msec
};

using namespace IMAGE;


ImageViewMain::ImageViewMain( const std::string& url )
    : ImageViewBase( url ),
      m_scrwin( 0 ),
      m_length_prev( 0 ),
      m_show_status( false ),
      m_show_label( false ),
      m_show_instdialog( true )
{
#ifdef _DEBUG    
    std::cout << "ImageViewMain::ImageViewMain : " << get_url() << std::endl;
#endif

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_IMAGEVIEW );

    // スクロールウィンドウを作ってEventBoxを貼る
    m_scrwin = Gtk::manage( new Gtk::ScrolledWindow() );
    assert( m_scrwin );

    m_scrwin->property_vscrollbar_policy() = Gtk::POLICY_AUTOMATIC;
    m_scrwin->property_hscrollbar_policy() = Gtk::POLICY_AUTOMATIC;
    m_scrwin->add( get_event() );
    pack_start( *m_scrwin );

    setup_common();

    // タイトルセット
    set_title( "[ 画像 ]" );
    IMAGE::get_admin()->set_command( "set_title", get_url(), get_title() );
}


//
// クロック入力
//
void ImageViewMain::clock_in()
{
    View::clock_in();

    // viewがアクティブになった(クロック入力が来た)ときにステータス表示
    if( m_show_status ){
        m_show_status = false;
        IMAGE::get_admin()->set_command( "set_status", get_url(), get_status() );
    }

    // ロード中
    if( loading() ){

        // 読み込みサイズの表示更新
        if( get_img()->is_loading() ) show_status();

        // ロード完了
        // 次にclock_in()が呼ばれたら下のelseの中に入る
        else{

            set_loading( false ); 
            show_view();
            show_status();
        }
    }

    // ロード中でない
    else if( get_imagearea() ){

        // バックグラウンドで開いた時やロード直後に画像を表示すると重くなるので
        // ビューがアクティブになった(クロック入力が来た) 時点で画面を表示する
        if( ! get_imagearea()->is_ready() ) {

            m_pre_width = get_width();
            m_pre_height = get_height();
            m_redraw_count = 10000;

            get_imagearea()->show_image();

            // 読み込みエラーが起きたらimageareaを除いてラベルを貼る
            if( ! get_imagearea()->get_errmsg().empty() ){
                set_status( get_imagearea()->get_errmsg() );
                remove_imagearea();
                set_label();
            }

            show_status();

            if( m_show_instdialog && get_imagearea() && get_imagearea()->is_ready()
                && CONFIG::get_instruct_tglimg() ) show_instruct_diag();
        }

        // サイズが変わって、かつ zoom to fit モードの場合再描画
        else if( get_img()->is_cached()
                 && get_img()->is_zoom_to_fit()
                 && get_width() > 1 && get_height() > 1
                 && ( m_pre_width != get_width() || m_pre_height != get_height() )
            ){

            // 毎回再描画していると遅いのでカウンタを付ける
            ++m_redraw_count;
            if( m_redraw_count >= ( IMGWIN_REDRAWTIME / TIMER_TIMEOUT ) ) {

                m_pre_width = get_width();
                m_pre_height = get_height();
                m_redraw_count = 0;

#ifdef _DEBUG
                std::cout << "ImageViewMain::clock_in resize\n" << get_url()
                          << " " << m_pre_width << " - " << m_pre_height << std::endl;
#endif
                get_imagearea()->show_image();
                show_status();
            }
        }
        else m_redraw_count = 0;
    }
}


//
// スレビューとの切り替え方法説明ダイアログ表示
//
void ImageViewMain::show_instruct_diag()
{
    const int mrg = 16;

    IMAGE::get_admin()->set_command_immediately( "disable_fold_win" );

    SKELETON::MsgDiag mdiag( NULL, 
        "画像ビューからスレビューに戻る方法として\n\n(1) マウスジェスチャを使う\n(マウス右ボタンを押しながら左または下にドラッグして右ボタンを離す)\n\n(2) マウスの5ボタンを押す\n\n(3) Alt+x か h か ← を押す\n\n(4) ツールバーのスレビューアイコンを押す\n\n(5) 表示メニューからスレビューを選ぶ\n\nなどがあります。詳しくはオンラインマニュアルを参照してください。" );
    Gtk::HBox hbox;
    Gtk::CheckButton chkbutton( "今後表示しない(_D)", true );
    hbox.pack_start( chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
//    chkbutton.set_alignment( 1.0, 0.0 );
    mdiag.get_vbox()->pack_start( hbox, Gtk::PACK_SHRINK );
    mdiag.set_title( "ヒント" );
    mdiag.show_all_children();
    mdiag.run();

    if( chkbutton.get_active() ) CONFIG::set_instruct_tglimg( false );
    m_show_instdialog = false;

    IMAGE::get_admin()->set_command_immediately( "enable_fold_win" );
}



//
// ラベルを貼る
//
void ImageViewMain::set_label()
{
    if( !m_show_label ){
        m_label.set_text( get_status() );
        get_event().add( m_label );
        m_label.show();
        m_show_label = true;
    }
}



//
// ラベルをremove
//
void ImageViewMain::remove_label()
{
    if( m_show_label ){
        get_event().remove();
        m_show_label = false;
    }
}



//
// 表示
//
void ImageViewMain::show_view()
{
    if( loading() ) return;

#ifdef _DEBUG
    std::cout << "ImageViewMain::show_view url = " << get_url() << std::endl;
#endif    

    // 画像を既に表示している
    if( get_imagearea() ){

        // キャッシュされてるなら再描画
        if( get_img()->is_cached() ){
#ifdef _DEBUG
            std::cout << "redraw\n";
#endif    
            get_imagearea()->show_image();
            show_status();
            return;
        }

        remove_imagearea();
    }

    remove_label();

    // 読み込み中        
    if( get_img()->is_loading() ){

#ifdef _DEBUG
        std::cout << "now loading\n";
#endif    
        set_loading( true );
        m_length_prev = 0;
        set_status( "loading..." );
        m_show_status = true; // viewがアクティブになった時点でステータス表示

        set_label();
    }

    // キャッシュがあるなら画像を表示
    else if( get_img()->is_cached() ){

#ifdef _DEBUG
        std::cout << "set image\n";
#endif    
        // 表示はビューがアクティブになった時に clock_in()の中で行う
        set_imagearea( Gtk::manage( new ImageAreaMain( get_url() ) ) );
    }

    // エラー
    else{

        set_status( get_img()->get_str_code() );
        m_show_status = true; // viewがアクティブになった時点でステータス表示

        set_label();
    }

    show_all_children();
}



//
// ステータス表示
//
void ImageViewMain::show_status()
{
    if( ! loading() ){

        // 画像が表示されていたら画像情報
        if( get_imagearea() ){

            std::stringstream ss;
            ss << get_img()->get_width() << " x " << get_img()->get_height();
            if( get_img()->get_width() )
                ss << " (" << get_img()->get_size() << " %)";
            ss << " " << get_img()->total_length()/1024 << " kb ";
            if( get_img()->is_protected() ) ss << " キャッシュ保護されています";

            set_status( ss.str() );
        }

        // エラー(ネットワーク系)
        else if( get_img()->get_code() != HTTP_OK ) set_status( get_img()->get_str_code() );

        // ステータス標示
        IMAGE::get_admin()->set_command( "set_status", get_url(), get_status() );
        if( m_show_label ) m_label.set_text( get_status() );

#ifdef _DEBUG
            std::cout << "ImageViewMain::show_status : " << get_status() << std::endl;;
#endif
    }

    // ロード中
    else{

        // 読み込みサイズが更新した場合
        if( m_length_prev != get_img()->current_length() ){

            m_length_prev = get_img()->current_length();

            char tmpstr[ 256 ];
            snprintf( tmpstr, 256, "%zd k / %zd k", m_length_prev/1024, get_img()->total_length()/1024 );
            set_status( tmpstr );

            // ステータス標示
            IMAGE::get_admin()->set_command( "set_status", get_url(), get_status() );
            if( m_show_label ) m_label.set_text( get_status() );

#ifdef _DEBUG
            std::cout << "ImageViewMain::show_status : " << get_status() << std::endl;;
#endif
        }
    }
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* ImageViewMain::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    return popupmenu;
}



//
// ボタンクリック
//
bool ImageViewMain::slot_button_press( GdkEventButton* event )
{
    ImageViewBase::slot_button_press( event );

    // ドラッグして画像移動するときの起点
    m_x_motion = event->x_root;
    m_y_motion = event->y_root;

    return true;
}




//
// マウスモーション
//
bool ImageViewMain::slot_motion_notify( GdkEventMotion* event )
{
    ImageViewBase::slot_motion_notify( event );

    // スクロールバー移動
    if( m_scrwin ){

        GdkEventButton event_button;
        get_control().get_eventbutton( CONTROL::ScrollImageButton, event_button );

#ifdef _DEBUG
        std::cout << "state = " << event->state << " / " << GDK_BUTTON1_MASK << " button = " << event_button.button << std::endl;
#endif

        if( ( ( event->state & GDK_BUTTON1_MASK ) && event_button.button == 1 )
            || ( ( event->state & GDK_BUTTON2_MASK ) && event_button.button == 2 )
            || ( ( event->state & GDK_BUTTON3_MASK ) && event_button.button == 3 )
            ){

            Gtk::Adjustment* hadj = m_scrwin->get_hadjustment();
            Gtk::Adjustment* vadj = m_scrwin->get_vadjustment();

            gdouble dx = event->x_root - m_x_motion;
            gdouble dy = event->y_root - m_y_motion;

#ifdef _DEBUG
            std::cout << "dx = " << dx << " dy = " << dy << std::endl;
#endif

            m_x_motion = event->x_root;
            m_y_motion = event->y_root;

            if( hadj ) hadj->set_value(
                MAX( hadj->get_lower(), MIN( hadj->get_upper() - hadj->get_page_size(), hadj->get_value() - dx ) ) );

            if( vadj ) vadj->set_value(
                MAX( vadj->get_lower(), MIN( vadj->get_upper() - vadj->get_page_size(), vadj->get_value() - dy ) ) );
        }
    }

    return true;
}


//
// 上スクロール
//
void ImageViewMain::scroll_up()
{
#ifdef _DEBUG
    std::cout << "ImageViewMain::scroll_up\n";
#endif

    Gtk::Adjustment*  vadjust = m_scrwin->get_vadjustment();
    if( !vadjust ) return;
    vadjust->set_value( MAX( 0,  vadjust->get_value() - vadjust->get_step_increment() ) );
}


//
// 下スクロール
//
void ImageViewMain::scroll_down()
{
#ifdef _DEBUG
    std::cout << "ImageViewMain::scroll_down\n";
#endif

    Gtk::Adjustment*  vadjust = m_scrwin->get_vadjustment();
    if( !vadjust ) return;
    vadjust->set_value(  MIN( vadjust->get_upper() - vadjust->get_page_size(),
                              vadjust->get_value() + vadjust->get_step_increment() ) );
}


//
// 左スクロール
//
void ImageViewMain::scroll_left()
{
#ifdef _DEBUG
    std::cout << "ImageViewMain::scroll_left\n";
#endif

    Gtk::Adjustment*  hadjust = m_scrwin->get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value( MAX( 0,  hadjust->get_value() - hadjust->get_step_increment() ) );
}


//
// 右スクロール
//
void ImageViewMain::scroll_right()
{
#ifdef _DEBUG
    std::cout << "ImageViewMain::scroll_right\n";
#endif

    Gtk::Adjustment*  hadjust = m_scrwin->get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value(  MIN( hadjust->get_upper() - hadjust->get_page_size(),
                              hadjust->get_value() + hadjust->get_step_increment() ) );
}
