// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageviewpopup.h"
#include "imageareapopup.h"

#include "dbimg/img.h"

#include "config/globalconf.h"

#include "colorid.h"
#include "cssmanager.h"

using namespace IMAGE;

ImageViewPopup::ImageViewPopup( const std::string& url )
    : ImageViewBase( url )
    , m_label( NULL )
    , m_length_prev( 0 )
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::ImageViewPopup url = " << get_url() << std::endl;
#endif    

    int border_width = 1;
    int margin = 0;
    std::string border_color = "black";
    std::string bg_color = CONFIG::get_color( COLOR_BACK );
    int classid = CORE::get_css_manager()->get_classid( "imgpopup" );
    if( classid >= 0 ){
        CORE::CSS_PROPERTY css = CORE::get_css_manager()->get_property( classid );
        border_width = css.border_left_width_px;
        margin = css.mrg_left_px;
        if( css.border_left_color >= 0 ) border_color = CORE::get_css_manager()->get_color( css.border_left_color );
        if( css.bg_color > 0 ) bg_color = CORE::get_css_manager()->get_color( css.bg_color );
    }

    //枠を描くためにm_eventの外にもう一つEventBoxを作る ( Gtk::HBox は modify_fg() 無効なので )
    pack_start( m_event_frame );
    m_event_frame.add( m_event_margin );
    m_event_margin.add( get_event() );

    // 枠の幅
    m_event_margin.set_border_width( border_width );

    // マージン
    get_event().set_border_width( margin );

    // 枠色
    m_event_frame.modify_bg( Gtk::STATE_NORMAL, Gdk::Color( border_color ) );

    // 背景色
    Gdk::Color color_bg( bg_color );
    m_event_margin.modify_bg( Gtk::STATE_NORMAL, color_bg );
    get_event().modify_bg( Gtk::STATE_NORMAL, color_bg );

    setup_common();
}


ImageViewPopup::~ImageViewPopup()
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::~ImageViewPopup url = " << get_url() << std::endl;
#endif    
}


//
// クロック入力
//
void ImageViewPopup::clock_in()
{
    View::clock_in();

    // ロード中
    if( is_loading() ){

        // 読み込みサイズの表示更新
        if( get_img()->is_loading() ) update_label();

        // ロード完了
        else {

            set_loading( false );

            if( CONFIG::get_use_image_popup() ){

                // 画像表示
                show_view();

                // リサイズ依頼
                sig_resize_popup().emit();
            }

            // ポップアップを閉じる
            else sig_hide_popup().emit();
        }
    }
}


//
// 画像表示停止
//
void ImageViewPopup::stop()
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::stop url = " << get_url() << std::endl;
#endif

    if( get_imagearea() ){
        get_imagearea()->stop();
        get_imagearea()->wait();
    }
}


//
// ラベルを貼る
//
void ImageViewPopup::set_label( const std::string& status )
{
    if( !m_label ){
        m_label = Gtk::manage( new Gtk::Label() );
        assert( m_label );
        get_event().add( *m_label );
        m_label->set_text( status );
        m_label->show();
    }
}



//
// ラベル削除
//
void ImageViewPopup::remove_label()
{
    if( m_label ){
        get_event().remove();
        delete m_label;
        m_label = NULL;
    }
}



//
// 表示
//
void ImageViewPopup::show_view()
{
    if( is_loading() ) return;
    if( get_imagearea() ) return;  // 画像を既に表示している

#ifdef _DEBUG
    std::cout << "ImageViewPopup::show_view url = " << get_url() << std::endl;
#endif    

    // サーバから読み込み中        
    if( get_img()->is_loading() ){

        set_loading( true );
        m_length_prev = 0;

        // ラベルに経過表示
        set_label( "読み込み中" );
    }

    // 画像張り付け
    // サーバから画像をロード中の時はclock_in()経由でもう一度 show_view()が呼び出される
    else{

        // キャッシュがあったら画像貼り付け
        if( get_img()->is_cached() ){

            ImageAreaBase* imagearea = Gtk::manage( new ImageAreaPopup( get_url() ) );
            imagearea->show_image();
            if( imagearea->get_errmsg().empty() ){
                remove_label();
                set_imagearea( imagearea );
            }

            // エラー表示
            else{
                set_label( "" );
                m_label->set_text( imagearea->get_errmsg() );
            }
        }

        // キャッシュが無い
        else{
            set_label( "" );
            if( get_img()->get_str_code( ).empty() ) m_label->set_text( "キャッシュが存在しません" );
            else m_label->set_text( get_img()->get_str_code( ) );
        }
    }

    // マージンやボーダーの分を幅と高さに加える
    int classid = CORE::get_css_manager()->get_classid( "imgpopup" );
    if( classid >= 0 ){

        CORE::CSS_PROPERTY css = CORE::get_css_manager()->get_property( classid );
        const int border_width = css.border_left_width_px;
        const int margin = css.mrg_left_px;
        set_width_client( width_client() + margin*2 + border_width*2 );
        set_height_client( height_client() + margin*2 + border_width*2 );
    }

    show_all_children();
}


//
// ラベル表示更新
//
void ImageViewPopup::update_label()
{
    if( ! m_label ) return;
        
    if( m_length_prev != get_img()->current_length() ){

        m_length_prev = get_img()->current_length();

        char tmpstr[ 256 ];
        snprintf( tmpstr, 256, "%zd k / %zd k", m_length_prev/1024, get_img()->total_length()/1024 );
        m_label->set_text( tmpstr );
    }
}

