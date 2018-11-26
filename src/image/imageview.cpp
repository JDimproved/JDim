// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageview.h"
#include "imagearea.h"

#include "skeleton/msgdiag.h"

#include "dbimg/img.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "control/controlid.h"

#include "history/historymanager.h"

#include "command.h"
#include "httpcode.h"
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
      m_update_status( false ),
      m_show_label( false ),
      m_do_resizing( false ),
      m_scrolled( false )
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


ImageViewMain::~ImageViewMain()
{
    // 閉じた画像履歴更新
    HISTORY::append_history( URL_HISTCLOSEIMGVIEW,
                             get_url(),
                             MISC::get_filename( get_url() ), TYPE_IMAGE );
}


//
// クロック入力
//
// アクティブ(表示されている)view以外にはクロックは来ないのに注意
//
void ImageViewMain::clock_in()
{
    View::clock_in();

    const int width_view = get_width();
    const int height_view = get_height();

    // ステータスのタブ番号などの表示内容更新
    if( m_update_status ){
        m_update_status = false;
        add_tab_number();
        m_show_status = true;
    }

    // viewがアクティブになった(クロック入力が来た)ときにステータス表示
    if( m_show_status ){
        m_show_status = false;
        IMAGE::get_admin()->set_command( "set_status", get_url(), get_status() );
    }

    // ロード待ち
    if( is_wait() ){

        // ロード待ち状態解除
        if( ! get_img()->is_wait() ){

            set_wait( false );

            if( get_img()->is_loading() ){
                set_loading( true  ); 
                set_status_local( "読み込み中" );
                add_tab_number();
                if( m_show_label ) m_label.set_text( get_status_local() );
            }

            else{
                set_loading( false );
                show_view();
                show_status();
            }
        }
    }

    // ロード中
    else if( is_loading() ){

        // 読み込みサイズの表示更新
        if( get_img()->is_loading() ) show_status();

        // ロード完了
        else{

            set_loading( false ); 
            show_view();
            show_status();
        }
    }

    // ロード中でなく、viewが表示されている
    else if( get_imagearea() && ! get_imagearea()->is_loading()
             && width_view > 1 && height_view > 1 ){

        // バックグラウンドで開いた時やロード直後に画像を表示すると重くなるので
        // ビューがアクティブになった(クロック入力が来た) 時点で画面を表示する
        if( ! get_imagearea()->is_ready() ){

            m_pre_width = width_view;
            m_pre_height = height_view;
            m_redraw_count = 10000;

            get_imagearea()->show_image();

            // 読み込みエラーが起きたらimageareaを除いてラベルを貼る
            if( ! get_imagearea()->get_errmsg().empty() ){
                set_status_local( get_imagearea()->get_errmsg() );
                add_tab_number();
                remove_imagearea();
                set_label();
            }

            show_status();

            if( CONFIG::get_instruct_tglimg() ) show_instruct_diag();
        }

        // サイズが変わって、かつ zoom to fit モードの場合再描画
        else if( get_imagearea()->is_ready()
                 && get_img()->is_cached()
                 && get_img()->is_zoom_to_fit()
                 && ( m_pre_width != width_view || m_pre_height != height_view )
            ){

            // 毎回再描画していると遅いのでカウンタを付ける
            ++m_redraw_count;
            if( m_redraw_count >= ( IMGWIN_REDRAWTIME / TIMER_TIMEOUT ) ) {

                m_pre_width = width_view;
                m_pre_height = height_view;
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
    SKELETON::MsgCheckDiag mdiag( get_parent_win(), 
                                  "画像ビューからスレビューに戻る方法として\n\n(1) マウスジェスチャを使う\n(マウス右ボタンを押しながら左または下にドラッグして右ボタンを離す)\n\n(2) マウスの5ボタンを押す\n\n(3) Alt+x か h か ← を押す\n\n(4) ツールバーのスレビューアイコンを押す\n\n(5) 表示メニューからスレビューを選ぶ\n\nなどがあります。詳しくはオンラインマニュアルを参照してください。",
                                  "今後表示しない(_D)"
        );

    mdiag.set_title( "ヒント" );
    mdiag.run();

    if( mdiag.get_chkbutton().get_active() ) CONFIG::set_instruct_tglimg( false );
}



//
// ラベルを貼る
//
void ImageViewMain::set_label()
{
    if( !m_show_label ){
        m_label.set_text( get_status_local() );
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
    if( is_loading() ) return;
    if( is_wait() ) return;

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

    // ロード待ち、又は読み込み中        
    if( get_img()->is_wait() || get_img()->is_loading() ){

        if( get_img()->is_wait() ){
#ifdef _DEBUG
            std::cout << "wait\n";
#endif
            set_wait( true );
            set_loading( false );
            set_status_local( "待機中" );
        }
        else{
#ifdef _DEBUG
            std::cout << "loading\n";
#endif
            set_wait( false );
            set_loading( true );
            set_status_local( "読み込み中" );
        }
        add_tab_number();

        m_length_prev = 0;
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

        if( ! get_img()->get_str_code().empty() ) set_status_local( get_img()->get_str_code() );
        else set_status_local( "画像情報が存在しません。再読み込みして下さい" );
        add_tab_number();

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
    if( ! is_loading() && ! is_wait() ){

        // 画像が表示されていたら画像情報
        if( get_imagearea() ){

            std::stringstream ss;
            ss << get_img()->get_width() << " x " << get_img()->get_height();
            if( get_img()->get_width() )
                ss << " (" << get_img()->get_size() << " %)";
            ss << " " << get_img()->total_length()/1024 << " K ";
            if( get_img()->is_protected() ) ss << " 保護中";

            set_status_local( ss.str() );
        }

        // エラー(ネットワーク系)
        else if( get_img()->get_code() != HTTP_OK ) set_status_local( get_img()->get_str_code() );

        add_tab_number();

        m_show_status = true; // viewがアクティブになった時点でステータス表示
        if( m_show_label ) m_label.set_text( get_status_local() );
    }

    // ロード中
    else if( is_loading() ){

        // 読み込みサイズが更新した場合
        if( m_length_prev != get_img()->current_length() ){

            m_length_prev = get_img()->current_length();

            char tmpstr[ 256 ];
            snprintf( tmpstr, 256, "%zd k / %zd k", m_length_prev/1024, get_img()->total_length()/1024 );
            set_status_local( tmpstr );
            add_tab_number();

            m_show_status = true; // viewがアクティブになった時点でステータス表示
            if( m_show_label ) m_label.set_text( get_status_local() );
        }
    }
}


void ImageViewMain::update_status()
{
    m_update_status = true; // viewがアクティブになった時点でステータス表示更新
}


//
// ステータスのタブ番号などの表示内容更新
//
void ImageViewMain::add_tab_number()
{
    set_status( " [" + MISC::itostr( IMAGE::get_admin()->get_current_page() + 1 )
                + "/" + MISC::itostr( IMAGE::get_admin()->get_tab_nums() ) + "] "
                + get_status_local()
                + " [" + MISC::get_filename( get_url() )
                + " (" + MISC::get_hostname( get_url(), false ) + ")]" );

#ifdef _DEBUG
    std::cout << "ImageViewMain::add_tab_number : " << get_status() << std::endl;;
#endif
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
    m_scrolled = false;

    // ボタンをリリースした時に大きさを変更
    m_do_resizing = false;
    if( ! is_loading() && ! is_wait()
        && get_control().button_alloted( event, CONTROL::ResizeImageButton ) ) m_do_resizing = true;

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
//        std::cout << "state = " << event->state << " / " << GDK_BUTTON1_MASK << " button = " << event_button.button << std::endl;
#endif

        if( ( ( event->state & GDK_BUTTON1_MASK ) && event_button.button == 1 )
            || ( ( event->state & GDK_BUTTON2_MASK ) && event_button.button == 2 )
            || ( ( event->state & GDK_BUTTON3_MASK ) && event_button.button == 3 )
            ){

            auto hadj = m_scrwin->get_hadjustment();
            auto vadj = m_scrwin->get_vadjustment();

            gdouble dx = event->x_root - m_x_motion;
            gdouble dy = event->y_root - m_y_motion;

#ifdef _DEBUG
//            std::cout << "dx = " << dx << " dy = " << dy << std::endl;
#endif

            m_x_motion = event->x_root;
            m_y_motion = event->y_root;

            if( hadj ) hadj->set_value(
                MAX( hadj->get_lower(), MIN( hadj->get_upper() - hadj->get_page_size(), hadj->get_value() - dx ) ) );

            if( vadj ) vadj->set_value(
                MAX( vadj->get_lower(), MIN( vadj->get_upper() - vadj->get_page_size(), vadj->get_value() - dy ) ) );

            m_scrolled = true;
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

    auto vadjust = m_scrwin->get_vadjustment();
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

    auto vadjust = m_scrwin->get_vadjustment();
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

    auto hadjust = m_scrwin->get_hadjustment();
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

    auto hadjust = m_scrwin->get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value(  MIN( hadjust->get_upper() - hadjust->get_page_size(),
                              hadjust->get_value() + hadjust->get_step_increment() ) );
}


//
// viewの操作
//
bool ImageViewMain::operate_view( const int control )
{
    // スクロールしたときはキャンセル
    if( m_scrolled ){
        m_scrolled = false;
        return false;
    }

    int cntl = control;

    if( m_do_resizing ){

        m_do_resizing = false;

        if( get_img()->get_size() == 100 ) cntl = CONTROL::ZoomFitImage;
        else cntl = CONTROL::OrgSizeImage;
    }

    return ImageViewBase::operate_view( cntl );
}
