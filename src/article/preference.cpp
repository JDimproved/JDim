// ライセンス: 最新のGPL

#include "preference.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "cache.h"

#include <string>
#include <list>

using namespace ARTICLE;

Preferences::Preferences( const std::string& url )
    : SKELETON::PrefDiag( url )
    ,m_label_name( DBTREE::article_subject( get_url() ), Gtk::ALIGN_LEFT )
    ,m_label_url( "URL : ", DBTREE:: url_readcgi( get_url(),0,0 ) )
    ,m_label_url_dat( "DAT : ", DBTREE:: url_dat( get_url() ) )
    ,m_label_cache( "ローカルキャッシュパス : ", CACHE::path_dat( get_url() ) )

    ,m_label_since( "スレ立て日時 : ", DBTREE::article_since_date( get_url() ) )
{
    // 一般
    m_vbox_info.set_border_width( 16 );
    m_vbox_info.set_spacing( 8 );
    m_vbox_info.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url_dat, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_cache, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_since, Gtk::PACK_SHRINK );

    std::string str_id, str_name;
    std::list< std::string >::iterator it;

    // id
    std::list< std::string > list_id = DBTREE::get_abone_list_id( get_url() );
    for( it = list_id.begin(); it != list_id.end(); ++it ) if( ! ( *it ).empty() ) str_id += ( *it ) + "\n";
    m_edit_id.set_text( str_id );

    // name
    std::list< std::string > list_name = DBTREE::get_abone_list_name( get_url() );
    for( it = list_name.begin(); it != list_name.end(); ++it ) if( ! ( *it ).empty() ) str_name += ( *it ) + "\n";
    m_edit_name.set_text( str_name );

    m_notebook.append_page( m_vbox_info, "一般" );
    m_notebook.append_page( m_edit_id, "あぼ〜んID" );
    m_notebook.append_page( m_edit_name, "あぼ〜んName" );

    get_vbox()->pack_start( m_notebook );
    set_title( "スレのプロパティ" );
    resize( 600, 400 );
    show_all_children();
}

Preferences::~Preferences()
{}


//
// OK 押した
//
void Preferences::slot_ok_clicked()
{
    // あぼーん再設定
    std::list< std::string > list_id = MISC::get_lines( m_edit_id.get_text(), true );
    std::list< std::string > list_name = MISC::get_lines( m_edit_name.get_text(), true );
    DBTREE::reset_abone( get_url(), list_id, list_name );
}
