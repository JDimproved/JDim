// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageviewpopup.h"
#include "imageareapopup.h"

#include "dbimg/img.h"

#include "config/globalconf.h"

#include "control/controlid.h"

#include "colorid.h"
#include "cssmanager.h"

using namespace IMAGE;

ImageViewPopup::ImageViewPopup( const std::string& url )
    : ImageViewBase( url )
    , m_label( NULL )
    , m_length_prev( 0 )
    , m_clicked( false )
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::ImageViewPopup url = " << get_url() << std::endl;
#endif    

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_IMAGEVIEW );
    get_control().add_mode( CONTROL::MODE_IMAGEICON );

    int border_width = 1;
    int margin = 0;
    std::string border_color = "black";
    std::string bg_color = CONFIG::get_color( COLOR_BACK );
    const int classid = CORE::get_css_manager()->get_classid( "imgpopup" );
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
#if GTKMM_CHECK_VERSION(3,0,0)
    m_event_frame.override_background_color( Gdk::RGBA( border_color ), Gtk::STATE_FLAG_NORMAL );
#else
    m_event_frame.modify_bg( Gtk::STATE_NORMAL, Gdk::Color( border_color ) );
#endif

    // 背景色
#if GTKMM_CHECK_VERSION(3,0,0)
    const Gdk::RGBA color_bg( bg_color );
    m_event_margin.override_background_color( color_bg, Gtk::STATE_FLAG_NORMAL );
    get_event().override_background_color( color_bg, Gtk::STATE_FLAG_NORMAL );
#else
    const Gdk::Color color_bg( bg_color );
    m_event_margin.modify_bg( Gtk::STATE_NORMAL, color_bg );
    get_event().modify_bg( Gtk::STATE_NORMAL, color_bg );
#endif

    // 全ての領域を表示できないならカーソルの上に表示
    set_popup_upside( true );

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

        std::string text_color = CONFIG::get_color( COLOR_CHAR );
        const int classid = CORE::get_css_manager()->get_classid( "imgpopup" );
        if( classid >= 0 ){
            CORE::CSS_PROPERTY css = CORE::get_css_manager()->get_property( classid );
            if( css.color >= 0 ) text_color = CORE::get_css_manager()->get_color( css.color );
        }
#if GTKMM_CHECK_VERSION(3,0,0)
        const Gdk::RGBA color_text( text_color );
        m_label->override_color( color_text, Gtk::STATE_FLAG_NORMAL );
#else
        const Gdk::Color color_text( text_color );
        m_label->modify_fg( Gtk::STATE_NORMAL, color_text );
#endif

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
#ifdef _DEBUG
    std::cout << "ImageViewPopup::show_view url = " << get_url() << std::endl;
#endif

    // 待ち状態
    if( is_wait() && ! get_img()->is_wait() ){

        set_wait( false );

        if( get_img()->is_loading() ){

            set_loading( true  );
            set_label( "読み込み中" );
        }
        else{
            set_loading( false );
            show_view_impl();
        }
    }

    // ロード完了
    else if( is_loading() && ! get_img()->is_loading() ){

        set_loading( false );

        // 画像表示
        if( CONFIG::get_use_image_popup() ){

            show_view_impl();

            // リサイズ依頼
            sig_resize_popup().emit();
        }

        // ポップアップを閉じる
        else sig_hide_popup().emit();
    }
    else show_view_impl();
}


void ImageViewPopup::show_view_impl()
{
    if( is_loading() ) return;
    if( is_wait() ) return;

#ifdef _DEBUG
    std::cout << "ImageViewPopup::show_view_impl url = " << get_url() << std::endl;
#endif    

    // 画像を既に表示している
    if( get_imagearea() ){

        // キャッシュされてるなら再描画
        if( get_img()->is_cached() ){
#ifdef _DEBUG
            std::cout << "redraw\n";
#endif    
            get_imagearea()->show_image();
        }

        return;
    }

    // サーバから読み込み中
    // 読み込みが終わったら show_view() が呼び出されて画像が表示される
    if( get_img()->is_loading() || get_img()->is_wait() ){

        if( get_img()->is_wait() ){
#ifdef _DEBUG
            std::cout << "wait\n";
#endif
            set_wait( true );
            set_loading( false );
            set_label( "待機中" );
        }
        else{

#ifdef _DEBUG
            std::cout << "loading\n";
#endif

            set_wait( false );
            set_loading( true );
            set_label( "読み込み中" );
        }

        m_length_prev = 0;
    }

    // 画像張り付け
    else{

#ifdef _DEBUG
        std::cout << "not loading\n";
#endif    

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
    const int classid = CORE::get_css_manager()->get_classid( "imgpopup" );
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


// クリックした時の処理
void ImageViewPopup::clicked()
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::clicked\n";
#endif

    // クリックしたらマウスボタンのリリース時に閉じる
    m_clicked = true;
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* ImageViewPopup::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_popup" ) );
    return popupmenu;
}


//
// 閉じる
//
void ImageViewPopup::close_view()
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::close_view\n";
#endif

    sig_hide_popup().emit();
}


//
// viewの操作
//
bool ImageViewPopup::operate_view( const int control )
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::operate_view control = " << control << std::endl;
#endif

    switch( control ){

        case CONTROL::CancelMosaic:
        case CONTROL::CancelMosaicButton:
            slot_cancel_mosaic();
            break;

        case CONTROL::Cancel:
        case CONTROL::CloseImageButton:
        case CONTROL::CloseImageTabButton:
        case CONTROL::Quit:
            close_view();
            break;

        case CONTROL::Save:
            slot_save();
            break;

        case CONTROL::Delete:
            delete_view_impl( true );
            break;

        default:
            if( m_clicked ) close_view();
            break;
    }

    m_clicked = false;
    return true;
}
