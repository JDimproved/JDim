// ライセンス: GPL2

#include "preference.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "jdlib/miscutil.h"

#include "command.h"

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

    for( int y = 0; y < 7; ++y ) {
        // label column
        Gtk::Widget* child = m_grid_info.get_child_at( 0, y );
        child->set_halign( Gtk::ALIGN_START );

        // value column
        child = m_grid_info.get_child_at( 1, y );
        child->set_halign( Gtk::ALIGN_START );
        child->set_hexpand( true );
        static_cast<Gtk::Label*>( child )->set_selectable( true );
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
