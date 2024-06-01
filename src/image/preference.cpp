// ライセンス: GPL2

#include "preference.h"

#include "dbtree/interface.h"

#include "dbimg/img.h"
#include "dbimg/imginterface.h"

#include "config/globalconf.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscutil.h"

#include "command.h"

#include <ios>
#include <sstream>

using namespace IMAGE;

Preferences::Preferences( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url, true )
    , m_label_url{ "URL:" }
    , m_label_url_value{ get_url() }
    , m_label_cache{ "ローカルキャッシュパス:" }
    , m_label_cache_value{ DBIMG::get_cache_path( url ) }
    , m_label_ref{ "参照元スレ名:" }
    , m_label_url_ref{ "参照元レス:" }
    , m_open_ref{ "開く" }
    , m_label_wh{ "大きさ:" }
    , m_label_size{ "サイズ( byte / Kbyte ):" }
    , m_label_type{ "種類:" }
    , m_label_imghash{ "画像のハッシュ値:" }
    ,m_check_protect( "キャッシュを保護する" )
{
    // 一般
    int num_from, num_to;
    std::string num_str;
    const std::string refurl = DBIMG::get_refurl( get_url() );
    const std::string daturl = DBTREE::url_dat( refurl, num_from, num_to, num_str );
    const std::string readcgi = DBTREE::url_readcgi( daturl, num_from, 0 );

    m_label_ref_value.set_text( MISC::to_plain( DBTREE::article_modified_subject( daturl ) ) );
    m_label_url_ref_value.set_text( readcgi );

    m_open_ref.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_open_ref ) );

    m_label_wh_value.set_text( std::to_string( DBIMG::get_width( get_url() ) )
                                         + " x "
                                         + std::to_string( DBIMG::get_height( get_url() ) ) );

    int size = DBIMG::get_filesize( get_url() );
    m_label_size_value.set_text( std::to_string( size ) + " / " + std::to_string( size/1024 ) );

    std::string type;
    switch( DBIMG::get_type_real( get_url() ) ){
        case DBIMG::T_JPG: type = "JPEG"; break;
        case DBIMG::T_PNG: type = "PNG"; break;
        case DBIMG::T_GIF: type = "GIF"; break;
        case DBIMG::T_BMP: type = "BMP"; break;
        case DBIMG::T_WEBP: type = "WebP"; break;
        case DBIMG::T_AVIF: type = "AVIF"; break;
    }

    if( DBIMG::is_fake( get_url() ) ) type += " ※拡張子が偽装されています※";
    m_label_type_value.set_text( type );

    if( std::optional<DBIMG::DHash> dhash = DBIMG::get_dhash( get_url() ); dhash ) {
        m_label_imghash_value.set_text( Glib::ustring::compose(
            "%1 %2",
            Glib::ustring::format(std::uppercase, std::hex, dhash->row_hash),
            Glib::ustring::format(std::uppercase, std::hex, dhash->col_hash) ) );
        m_button_copy = Gtk::make_managed<Gtk::Button>();
        m_button_copy->set_hexpand( false );
        m_button_copy->set_image_from_icon_name( "edit-copy-symbolic" );
        m_button_copy->set_tooltip_text( "NG 画像ハッシュ設定をクリップボードにコピー" );
        m_button_copy->signal_clicked().connect( sigc::mem_fun( *this, &IMAGE::Preferences::slot_copy_clicked ) );

        m_check_abone = Gtk::make_managed<Gtk::CheckButton>( "NG 画像ハッシュに追加する (この画像もあぼ〜んされる)" );
        m_check_abone->set_halign( Gtk::ALIGN_START );

        m_check_protect.signal_toggled().connect( sigc::mem_fun( *this, &IMAGE::Preferences::slot_toggled_protect ) );

    }

    Gtk::Label* label_abone_reason{};
    Gtk::Label* label_abone_reason_value{};
    std::string abone_reason = DBIMG::get_img_abone_reason( get_url() );
    if( ! abone_reason.empty() ) {
        abone_reason = MISC::replace_str( abone_reason, "<br>", "\n" );
        label_abone_reason_value = Gtk::make_managed<Gtk::Label>( abone_reason );
        label_abone_reason_value->set_ellipsize( Pango::ELLIPSIZE_END );
        label_abone_reason_value->set_halign( Gtk::ALIGN_START );
        label_abone_reason_value->set_hexpand( true );
        label_abone_reason_value->set_selectable( true );

        label_abone_reason = Gtk::make_managed<Gtk::Label>( "備考:" );
        label_abone_reason->set_halign( Gtk::ALIGN_START );
        label_abone_reason->set_valign( Gtk::ALIGN_START );
    }

    m_check_protect.set_active( DBIMG::is_protected( get_url() ) );

    m_grid_info.set_column_spacing( 10 );
    m_grid_info.set_row_spacing( 8 );

    m_grid_info.attach( m_label_url, 0, 0, 1, 1 );
    m_grid_info.attach( m_label_url_value, 1, 0, 2, 1 );
    m_grid_info.attach( m_label_cache, 0, 1, 1, 1 );
    m_grid_info.attach( m_label_cache_value, 1, 1, 2, 1 );
    m_grid_info.attach( m_label_ref, 0, 2, 1, 1 );
    m_grid_info.attach( m_label_ref_value, 1, 2, 2, 1 );
    m_grid_info.attach( m_label_url_ref, 0, 3, 1, 1 );
    m_grid_info.attach( m_label_url_ref_value, 1, 3, 1, 1 );
    m_grid_info.attach( m_open_ref, 2, 3, 1, 1 );
    m_grid_info.attach( m_label_wh, 0, 4, 1, 1 );
    m_grid_info.attach( m_label_wh_value, 1, 4, 2, 1 );
    m_grid_info.attach( m_label_size, 0, 5, 1, 1 );
    m_grid_info.attach( m_label_size_value, 1, 5, 2, 1 );
    m_grid_info.attach( m_label_type, 0, 6, 1, 1 );
    m_grid_info.attach( m_label_type_value, 1, 6, 2, 1 );
    m_grid_info.attach( m_label_imghash, 0, 7, 1, 1 );

    int bottom_of_row;
    if( m_button_copy ) {
        m_grid_info.attach( m_label_imghash_value, 1, 7, 1, 1 );
        m_grid_info.attach( *m_button_copy, 2, 7, 1, 1 );
        m_grid_info.attach( *m_check_abone, 1, 8, 2, 1 );
        bottom_of_row = 9;
    }
    else {
        m_grid_info.attach( m_label_imghash_value, 1, 7, 2, 1 );
        bottom_of_row = 8;
    }

    if( label_abone_reason ) {
        m_grid_info.attach( *label_abone_reason, 0, bottom_of_row, 1, 1 );
        m_grid_info.attach( *label_abone_reason_value, 1, bottom_of_row, 2, 1 );
    }

    for( int y = 0; y < 8; ++y ) {
        // label column
        Gtk::Widget* child = m_grid_info.get_child_at( 0, y );
        child->set_halign( Gtk::ALIGN_START );

        // value column
        child = m_grid_info.get_child_at( 1, y );
        child->set_halign( Gtk::ALIGN_START );
        child->set_hexpand( true );
        static_cast<Gtk::Label*>( child )->set_selectable( true );
        static_cast<Gtk::Label*>( child )->set_ellipsize( Pango::ELLIPSIZE_END );
    }

    set_title( "画像のプロパティ" );
    get_content_area()->property_margin() = 16;
    get_content_area()->pack_start( m_grid_info );
    get_content_area()->pack_end( m_check_protect, Gtk::PACK_SHRINK );
    set_default_size( 700, 400 );
    show_all_children();
}


//
// OK 押した
//
void Preferences::slot_ok_clicked()
{
    if( m_check_protect.get_active() ) DBIMG::set_protect( get_url(), true );
    else DBIMG::set_protect( get_url(), false );

    if( m_check_abone && m_check_abone->get_active() ) {
        DBIMG::push_abone_imghash( get_url(), CONFIG::get_img_hash_initial_threshold() );
    }

    // viewの再レイアウト
    CORE::core_set_command( "relayout_article", get_url() );
}


//
// 参照元を開く
//
// ImageViewBase::slot_open_ref() からのコピペ
//
void Preferences::slot_open_ref()
{
    std::string refurl = DBIMG::get_refurl( get_url() );

    int center, from, to;
    std::string num_str;
    const std::string url = DBTREE::url_dat( refurl, center, to, num_str );
    if( url.empty() ) return;

    const int range = 10;
    from = MAX( 0, center - range );
    to = center + range;
    std::stringstream ss;
    ss << from << "-" << to;

    CORE::core_set_command( "open_article_res", url, ss.str(), std::to_string( center ) );

    response( Gtk::RESPONSE_CANCEL );
}


/**
 * @brief NG 画像ハッシュの設定をクリップボードにコピーする
 */
void Preferences::slot_copy_clicked()
{
    std::string data = m_label_imghash_value.get_text();
    if( data.empty() ) data = "0 0";

    if( DBIMG::Img* img = DBIMG::get_img( get_url() ); img ) {
        data.push_back( ' ' );
        data.append( std::to_string( CONFIG::get_img_hash_initial_threshold() ) );
        data.push_back( ' ' );
        data.append( std::to_string( DBIMG::kImgHashReserved ) );
        data.push_back( ' ' );
        data.append( std::to_string( img->get_time_modified() ) );
        data.push_back( ' ' );
        data.append( img->url() );
        data.push_back( '\n' );
    }
    else {
        data.append( " 0 0 0\n" );
    }
    MISC::CopyClipboard( data );

    // コピーしたことを表すためボタンのアイコンを2秒間チェックマークに変更する
    constexpr int timeout_ms = 2000; // 単位はミリ秒
    Glib::signal_timeout().connect_once( sigc::mem_fun( *this, &Preferences::slot_button_icon_timeout ), timeout_ms );
    m_button_copy->set_image_from_icon_name( "object-select-symbolic" );
}


/**
 * @brief コピーボタンのアイコンを元に戻す
 */
void Preferences::slot_button_icon_timeout()
{
    m_button_copy->set_image_from_icon_name( "edit-copy-symbolic" );
}


/**
 * @brief キャッシュを保護するときはNG 画像ハッシュに追加するチェックボタンを無効にする
 */
void Preferences::slot_toggled_protect()
{
    if( m_check_protect.get_active() ) {
        m_check_abone->set_active( false );
        m_check_abone->set_sensitive( false );
    }
    else {
        m_check_abone->set_sensitive( true );
    }
}
