// ライセンス: 最新のGPL

#include "preference.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "cache.h"
#include "command.h"

#include <string>
#include <list>

using namespace ARTICLE;

Preferences::Preferences( const std::string& url )
    : SKELETON::PrefDiag( url )
    ,m_label_name( DBTREE::article_subject( get_url() ), Gtk::ALIGN_LEFT )
    ,m_label_url( "URL : ", DBTREE:: url_readcgi( get_url(),0,0 ) )
    ,m_label_url_dat( "DAT : ", DBTREE:: url_dat( get_url() ) )
    ,m_label_cache( "ローカルキャッシュパス : ", std::string() )
    ,m_label_size( "サイズ(byte) : ", std::string() )
    ,m_label_since( "スレ立て日時 : ", DBTREE::article_since_date( get_url() ) )
    ,m_label_modified( "最終更新日時 : ", std::string() )
    ,m_label_write( "最終書き込み日時 : ", std::string() )
{
    // 一般
    if( DBTREE::article_is_cached( get_url() ) ){
        m_label_cache.set_text( CACHE::path_dat( get_url() ) );
        m_label_size.set_text( MISC::itostr( DBTREE::article_lng_dat( get_url() ) ) );
        m_label_modified.set_text( MISC::timettostr( DBTREE::article_time_modified( get_url() ) ) );
        if( DBTREE::article_write_time( get_url() ) ) m_label_write.set_text( DBTREE::article_write_date( get_url() ) );
    }

    m_vbox_info.set_border_width( 16 );
    m_vbox_info.set_spacing( 8 );
    m_vbox_info.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url_dat, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_cache, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_size, Gtk::PACK_SHRINK );

    m_vbox_info.pack_start( m_label_since, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_modified, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_write, Gtk::PACK_SHRINK );

    std::string str_id, str_name, str_word, str_regex;
    std::list< std::string >::iterator it;

    if( DBTREE::article_is_cached( get_url() ) ){ 

        // id
        std::list< std::string > list_id = DBTREE::get_abone_list_id( get_url() );
        for( it = list_id.begin(); it != list_id.end(); ++it ) if( ! ( *it ).empty() ) str_id += ( *it ) + "\n";
        m_edit_id.set_text( str_id );

        // name
        std::list< std::string > list_name = DBTREE::get_abone_list_name( get_url() );
        for( it = list_name.begin(); it != list_name.end(); ++it ) if( ! ( *it ).empty() ) str_name += ( *it ) + "\n";
        m_edit_name.set_text( str_name );

        // word
        std::list< std::string > list_word = DBTREE::get_abone_list_word( get_url() );
        for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
        m_edit_word.set_text( str_word );

        // regex
        std::list< std::string > list_regex = DBTREE::get_abone_list_regex( get_url() );
        for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
        m_edit_regex.set_text( str_regex );
    }
    else{
        m_edit_id.set_editable( false );
        m_edit_name.set_editable( false );
        m_edit_word.set_editable( false );
        m_edit_regex.set_editable( false );
    }

    m_notebook.append_page( m_vbox_info, "一般" );
    m_notebook.append_page( m_edit_id, "NG ID" );
    m_notebook.append_page( m_edit_name, "NG Name" );
    m_notebook.append_page( m_edit_word, "NG Word" );
    m_notebook.append_page( m_edit_regex, "NG 正規表現" );

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
    std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text(), true );
    std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text(), true );
    DBTREE::reset_abone( get_url(), list_id, list_name, list_word, list_regex );

    // viewの再レイアウト
    CORE::core_set_command( "relayout_article", get_url() );
}
