// ライセンス: GPL2

#include "preference.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "cache.h"

using namespace IMAGE;

Preferences::Preferences( const std::string& url )
    : SKELETON::PrefDiag( url, false )
    ,m_label_url( false, "URL : ", get_url() )
    ,m_label_cache( false, "ローカルキャッシュパス : ", CACHE::path_img( get_url() ) )
    ,m_label_ref( false, "参照元スレ : " )
    ,m_label_url_ref( false, "参照元スレのURL : " )
{
    // 一般
    int num_from, num_to;
    std::string refurl = DBIMG::refurl( get_url() );
    std::string daturl = DBTREE::url_dat( refurl, num_from, num_to );
    std::string readcgi = DBTREE::url_readcgi( daturl, num_from, 0 );

    m_label_ref.set_text( DBTREE::article_subject( daturl ) );
    m_label_url_ref.set_text( readcgi );

    m_vbox_info.set_border_width( 16 );
    m_vbox_info.set_spacing( 8 );
    m_vbox_info.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_cache, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_ref, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url_ref, Gtk::PACK_SHRINK );

    set_title( "プロパティ" );
    get_vbox()->pack_start( m_vbox_info );
    resize( 600, 400 );
    show_all_children();
}
