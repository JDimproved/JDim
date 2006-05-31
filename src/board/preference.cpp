// ライセンス: 最新のGPL

#include "preference.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "cache.h"

using namespace BOARD;
 
Preferences::Preferences( const std::string& url )
    : SKELETON::PrefDiag( url, false ),
      m_frame_cookie( "クッキー" ),
      m_label_cookie( "未取得" ),
      m_button_cookie( "削除" ) ,
      m_label_name( DBTREE::board_name( get_url() ), Gtk::ALIGN_LEFT ),
      m_label_url( "URL : ", DBTREE::url_boardbase( get_url() ) ),
      m_label_cache( "ローカルキャッシュパス", CACHE::path_board_root( DBTREE::url_boardbase( get_url() ) ) ),

      m_label_noname(  "デフォルト名無し : ", DBTREE::default_noname( get_url() ) ),
      m_label_line( "1レスの最大改行数 : " ),
      m_label_byte( "1レスの最大バイト数 : " )
{
    m_label_cookie.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );
    std::string str_tmp = DBTREE::board_cookie_for_write( get_url() );
    if( !str_tmp.empty() ) m_label_cookie.set_label( str_tmp );
    m_hbox_cookie.set_border_width( 8 );
    m_hbox_cookie.set_spacing( 8 );
    m_hbox_cookie.pack_start( m_label_cookie );
    m_hbox_cookie.pack_start( m_button_cookie, Gtk::PACK_SHRINK );
    m_button_cookie.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_delete_cookie ) );

    m_frame_cookie.add( m_hbox_cookie );

    m_label_line.set_text( MISC::itostr( DBTREE::line_number( get_url() ) * 2 ) );
    m_label_byte.set_text( MISC::itostr( DBTREE::message_count( get_url() ) ) );

    m_vbox.set_border_width( 16 );
    m_vbox.set_spacing( 8 );
    m_vbox.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_cache, Gtk::PACK_SHRINK );

    m_vbox.pack_start( m_label_noname, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_line, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_byte, Gtk::PACK_SHRINK );
    m_vbox.pack_end( m_frame_cookie, Gtk::PACK_SHRINK );

    // SETTING.TXT
    m_edit_settingtxt.textview().set_editable( false );
    m_edit_settingtxt.set_text( DBTREE::settingtxt( get_url() ) );

    m_notebook.append_page( m_vbox, "一般" );
    m_notebook.append_page( m_edit_settingtxt, "SETTING.TXT" );

    get_vbox()->pack_start( m_notebook );
    set_title( "板のプロパティ" );
    resize( 600, 400 );
    show_all_children();
}


void Preferences::slot_delete_cookie()
{
    DBTREE::board_set_cookie_for_write( get_url(), std::string() );
    m_label_cookie.set_label( "未取得" );
}

