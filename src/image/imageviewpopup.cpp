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
    , m_provider{ Gtk::CssProvider::create() }
{
#ifdef _DEBUG
    std::cout << "ImageViewPopup::ImageViewPopup url = " << get_url() << std::endl;
#endif    

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_IMAGEVIEW );
    get_control().add_mode( CONTROL::MODE_IMAGEICON );

    // $XDG_CONFIG_HOME/gtk-3.0/gtk.css から設定できるようにCSSセレクタを追加する (テスト用)
    set_name( "jdim-imageview-popup" );

    // ポップアップの文字色、背景色を設定する
    int border_width = 1;
    int margin = 0;
    Gdk::RGBA border_color{ "black" };
    Gdk::RGBA bg_color{ CONFIG::get_color( COLOR_BACK ) };
    Gdk::RGBA text_color{ CONFIG::get_color( COLOR_CHAR ) };
    const auto manager = CORE::get_css_manager();
    const int classid = manager->get_classid( "imgpopup" );
    if( classid >= 0 ){
        // jd.cssの設定を読み込む
        const CORE::CSS_PROPERTY& css = manager->get_property( classid );
        border_width = css.border_left_width_px;
        margin = css.mrg_left_px;
        if( css.border_left_color >= 0 ) border_color.set( manager->get_color( css.border_left_color ) );
        if( css.bg_color > 0 ) bg_color.set( manager->get_color( css.bg_color ) );
        if( css.color >= 0 ) text_color.set( manager->get_color( css.color ) );
    }

    pack_start( get_event() );

    try {
        // XXX: 修正前の動作を維持するためmaginの値はpaddingとして反映している
        m_provider->load_from_data( Glib::ustring::compose(
            R"( box {
                padding: %1px;
                border: %2px solid %3;
                background-color: %4;
            }
            label {
                color: %5;
            } )",
            margin, border_width, border_color.to_string(), bg_color.to_string(), text_color.to_string() ) );
    }
    catch( Gtk::CssProviderError& ) {
        std::terminate();
    }
    get_style_context()->add_provider( m_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

    // 全ての領域を表示できないならカーソルの上に表示
    set_popup_upside( true );

    setup_common();
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
        m_label = std::make_unique<Gtk::Label>( status );
        get_event().add( *m_label );

        // $XDG_CONFIG_HOME/gtk-3.0/gtk.css から設定できるようにCSSセレクタを追加する (テスト用)
        m_label->set_name( "jdim-imageview-popup" );
        m_label->get_style_context()->add_provider( m_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

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
        m_label.reset();
    }
}



/** @brief ウィジェットの自然な幅と高さを使用してビューのサイズを調整する
 *
 * @details 取得した自然なサイズが現在の値より大きい場合は更新する。
 * @param[in] widget 自然な幅と高さを取得する対象
 */
void ImageViewPopup::adjust_client_size( Gtk::Widget& widget )
{
    [[maybe_unused]] int unused_value;
    int natural_width = 0;
    int natural_height = 0;

    widget.get_preferred_width( unused_value, natural_width );
    widget.get_preferred_height( unused_value, natural_height );

    if( width_client() < natural_width ) {
        set_width_client( natural_width );
    }
    if( height_client() < natural_height ) {
        set_height_client( natural_height );
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

                adjust_client_size( *m_label );
            }
        }

        // キャッシュが無い
        else{
            set_label( "" );
            if( get_img()->get_str_code( ).empty() ) m_label->set_text( "キャッシュが存在しません" );
            else m_label->set_text( get_img()->get_str_code( ) );

            adjust_client_size( *m_label );
        }
    }

    // マージンやボーダーの分を幅と高さに加える
    const CORE::Css_Manager* const manager = CORE::get_css_manager();
    const int classid = manager->get_classid( "imgpopup" );
    if( classid >= 0 ){

        const CORE::CSS_PROPERTY& css = manager->get_property( classid );
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

        std::string tmpstr = std::to_string( m_length_prev / 1024 );
        tmpstr.append( " / " );
        tmpstr.append( std::to_string( get_img()->total_length() / 1024 ) );
        tmpstr.append( " KiB" );
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
