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
    ,m_label_url( false, "URL：", get_url() )
    ,m_label_cache( false, "ローカルキャッシュパス：", DBIMG::get_cache_path( url ) )
    ,m_label_ref( false, "参照元スレ名：" )
    ,m_label_url_ref( false, "参照元レス：" )
    ,m_open_ref( "開く" )
    ,m_label_wh( false, "大きさ : ", std::string() )
    ,m_label_size( false, "サイズ( byte / Kbyte ) : ", std::string() )
    ,m_label_type( false, "種類 : ", std::string() )
    ,m_check_protect( "キャッシュを保護する" )
{
    // 一般
    int num_from, num_to;
    std::string num_str;
    const std::string refurl = DBIMG::get_refurl( get_url() );
    const std::string daturl = DBTREE::url_dat( refurl, num_from, num_to, num_str );
    const std::string readcgi = DBTREE::url_readcgi( daturl, num_from, 0 );

    m_label_ref.set_text( MISC::to_plain( DBTREE::article_modified_subject( daturl ) ) );
    m_label_url_ref.set_text( readcgi );

    m_open_ref.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_open_ref ) );
    m_hbox_ref.pack_start( m_label_url_ref );
    m_hbox_ref.pack_start( m_open_ref, Gtk::PACK_SHRINK );

    m_label_wh.set_text( std::to_string( DBIMG::get_width( get_url() ) )
                                         + " x "
                                         + std::to_string( DBIMG::get_height( get_url() ) ) );

    int size = DBIMG::get_filesize( get_url() );
    m_label_size.set_text( std::to_string( size ) + " / " + std::to_string( size/1024 ) );

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
    m_label_type.set_text( type );

    m_check_protect.set_active( DBIMG::is_protected( get_url() ) );

    m_vbox_info.set_border_width( 16 );
    m_vbox_info.set_spacing( 8 );
    m_vbox_info.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_cache, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_ref, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_hbox_ref, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_wh, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_size, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_type, Gtk::PACK_SHRINK );

    m_vbox_info.pack_end( m_check_protect, Gtk::PACK_SHRINK );

    set_title( "画像のプロパティ" );
    get_content_area()->pack_start( m_vbox_info );
    resize( 600, 400 );
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
