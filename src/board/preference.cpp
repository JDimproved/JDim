// ライセンス: 最新のGPL

#include "preference.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "cache.h"
#include "command.h"

using namespace BOARD;
 
Preferences::Preferences( const std::string& url )
    : SKELETON::PrefDiag( url ),
      m_frame_cookie( "クッキー＆Hana" ),
      m_button_cookie( "削除" ) ,
      m_check_noname( "名前欄が空白の時は書き込まない" ),

      m_label_name( DBTREE::board_name( get_url() ), Gtk::ALIGN_LEFT ),
      m_label_url( "URL : ", DBTREE::url_boardbase( get_url() ) ),
      m_label_cache( "ローカルキャッシュパス", CACHE::path_board_root( DBTREE::url_boardbase( get_url() ) ) ),

      m_label_noname(  "デフォルト名無し : ", DBTREE::default_noname( get_url() ) ),
      m_label_line( "1レスの最大改行数 : " ),
      m_label_byte( "1レスの最大バイト数 : " )
{
    m_edit_cookies.textview().set_editable( false );
    m_check_noname.set_active( DBTREE::board_check_noname( get_url() ) );

    // cookie と hana をセット
    std::string str_cookies_hana;
    std::list< std::string > list_cookies = DBTREE::board_list_cookies_for_write( get_url() );
    if( list_cookies.empty() ) str_cookies_hana = "cookie: 未取得\n";
    else{
        std::list< std::string >::iterator it = list_cookies.begin();
        for( ; it != list_cookies.end(); ++it )
            str_cookies_hana += "cookie: " + MISC::iconv( (*it), DBTREE::board_charset( get_url() ), "UTF-8" ) + "\n";
    }

    std::string hana = DBTREE::board_hana_for_write( get_url() );
    if( ! hana.empty() ) str_cookies_hana += "\nhana: " + hana + "\n";

    m_edit_cookies.set_text( str_cookies_hana );

    m_hbox_cookie.set_border_width( 8 );
    m_hbox_cookie.set_spacing( 8 );
    m_hbox_cookie.pack_start( m_edit_cookies );
    m_hbox_cookie.pack_start( m_vbox_cookie, Gtk::PACK_SHRINK );
    m_vbox_cookie.pack_end( m_button_cookie, Gtk::PACK_SHRINK );
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
    m_vbox.pack_end( m_check_noname, Gtk::PACK_SHRINK );

    std::string str_thread, str_word, str_regex;
    std::list< std::string >::iterator it;

    // スレあぼーん
    std::list< std::string > list_thread = DBTREE::get_abone_list_thread( get_url() );
    for( it = list_thread.begin(); it != list_thread.end(); ++it ) if( ! ( *it ).empty() ) str_thread += ( *it ) + "\n";
    m_edit_thread.set_text( str_thread );

    // スレwordあぼーん
    std::list< std::string > list_word = DBTREE::get_abone_list_word_thread( get_url() );
    for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
    m_edit_word.set_text( str_word );

    // スレregexあぼーん
    std::list< std::string > list_regex = DBTREE::get_abone_list_regex_thread( get_url() );
    for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
    m_edit_regex.set_text( str_regex );


    // SETTING.TXT
    m_edit_settingtxt.textview().set_editable( false );
    m_edit_settingtxt.set_text( DBTREE::settingtxt( get_url() ) );

    m_notebook.append_page( m_vbox, "一般" );
    m_notebook.append_page( m_edit_thread, "NG スレタイトル" );
    m_notebook.append_page( m_edit_word, "NG ワード(スレあぼ〜ん用)" );
    m_notebook.append_page( m_edit_regex, "NG 正規表現(スレあぼ〜ん用)" );
    m_notebook.append_page( m_edit_settingtxt, "SETTING.TXT" );

    get_vbox()->pack_start( m_notebook );
    set_title( "板のプロパティ" );
    resize( 600, 400 );
    show_all_children();
}


void Preferences::slot_delete_cookie()
{
    DBTREE::board_set_list_cookies_for_write( get_url(), std::list< std::string >() );
    DBTREE::board_set_hana_for_write( get_url(), std::string() );

    m_edit_cookies.set_text( "未取得" );
}


//
// OK 押した
//
void Preferences::slot_ok_clicked()
{
    // あぼーん再設定
    std::list< std::string > list_thread = MISC::get_lines( m_edit_thread.get_text(), true );
    std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text(), true );
    std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text(), true );
    DBTREE::reset_abone_thread( get_url(), list_thread, list_word, list_regex );  // 板の再描画も行われる

    // 名無し書き込みチェック
    DBTREE::board_set_check_noname( get_url(), m_check_noname.get_active() );

    DBTREE::board_save_info( get_url() );
}
