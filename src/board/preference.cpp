// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "preference.h"

#include "dbtree/interface.h"
#include "dbtree/boardbase.h"

#include "jdlib/miscutil.h"

#include "cache.h"
#include "command.h"
#include "global.h"
#include "viewfactory.h"

using namespace BOARD;

Preferences::Preferences( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_frame_write( "書き込み設定" ),
      m_entry_writename( true, "名前：" ),
      m_entry_writemail( true, "メール：" ),
      m_check_noname( "名前欄が空白の時は書き込まない" ),

      m_frame_cookie( "クッキー＆Hana" ),
      m_button_cookie( "削除" ) ,

      m_proxy_frame( "読み込み用" ),
      m_proxy_frame_w( "書き込み用" ),

      m_label_name( "板タイトル：" + DBTREE::board_name( get_url() ), Gtk::ALIGN_LEFT ),
      m_label_url( false, "板のURL：", DBTREE::url_boardbase( get_url() ) ),
      m_label_cache( false, "ローカルキャッシュのルートパス", CACHE::path_board_root( DBTREE::url_boardbase( get_url() ) ) ),


      m_label_noname( false, "デフォルト名無し：", DBTREE::default_noname( get_url() ) ),
      m_label_line( false, "1レスの最大改行数：" ),
      m_label_byte( false, "1レスの最大バイト数：" ),
      m_label_samba( false, "書き込み規制秒数 (Samba24) ：" ),
      m_button_clearsamba( "秒数クリア" ),
      m_localrule( NULL )
{
    m_edit_cookies.set_editable( false );

    // 書き込み設定
    m_check_noname.set_active( DBTREE::board_check_noname( get_url() ) );
    m_entry_writename.set_text(DBTREE::board_get_write_name( get_url() ) ); 
    m_entry_writemail.set_text(DBTREE::board_get_write_mail( get_url() ) );
    if( m_entry_writemail.get_text().empty() ) m_entry_writemail.set_text( "sage" );
    // JD_MAIL_BLANK の場合空白をセットする
    else if( m_entry_writemail.get_text() == JD_MAIL_BLANK ) m_entry_writemail.set_text( std::string() );

    m_hbox_write.set_spacing( 8 );
    m_hbox_write.pack_start( m_entry_writename );
    m_hbox_write.pack_start( m_entry_writemail );

    m_vbox_write.set_border_width( 8 );
    m_vbox_write.set_spacing( 8 );
    m_vbox_write.pack_start( m_check_noname, Gtk::PACK_SHRINK );
    m_vbox_write.pack_start( m_hbox_write, Gtk::PACK_SHRINK );

    m_frame_write.add( m_vbox_write );


    // cookie と hana の設定
    std::string str_cookies_hana;
    std::list< std::string > list_cookies = DBTREE::board_list_cookies_for_write( get_url() );
    if( list_cookies.empty() ) str_cookies_hana = "cookie: 未取得\n";
    else{
        std::list< std::string >::iterator it = list_cookies.begin();
        for( ; it != list_cookies.end(); ++it )
            str_cookies_hana += "cookie: " + MISC::Iconv( (*it), DBTREE::board_charset( get_url() ), "UTF-8" ) + "\n";
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


    // 一般ページのパッキング
    m_label_line.set_text( MISC::itostr( DBTREE::line_number( get_url() ) * 2 ) );
    m_label_byte.set_text( MISC::itostr( DBTREE::message_count( get_url() ) ) );

    int samba_sec = DBTREE::board_samba_sec( get_url() );
    if( ! samba_sec ) m_label_samba.set_text( "未取得" );
    else m_label_samba.set_text( MISC::itostr( samba_sec ) );

    m_button_clearsamba.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_samba ) );
    m_hbox_samba.pack_start( m_label_samba, Gtk::PACK_SHRINK );
    m_hbox_samba.pack_start( m_button_clearsamba, Gtk::PACK_SHRINK );    

    m_vbox.set_border_width( 16 );
    m_vbox.set_spacing( 8 );
    m_vbox.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_cache, Gtk::PACK_SHRINK );

    m_vbox.pack_start( m_label_noname, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_line, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_byte, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_samba, Gtk::PACK_SHRINK );
    m_vbox.pack_end( m_frame_cookie, Gtk::PACK_SHRINK );
    m_vbox.pack_end( m_frame_write, Gtk::PACK_SHRINK );

    // ローカルルール
    m_localrule = CORE::ViewFactory( CORE::VIEW_ARTICLEINFO, get_url() );

    // プロキシ
    m_vbox_proxy.set_border_width( 16 );
    m_vbox_proxy.set_spacing( 8 );

    m_label_proxy.set_text( "通常は全体プロキシ設定でプロキシを設定します\n全体プロキシ設定と異なるプロキシを使用する場合はここで設定して下さい");

    switch( DBTREE::board_get_mode_local_proxy( get_url() ) ){
        case DBTREE::PROXY_GLOBAL: m_proxy_frame.rd_global.set_active(); break;
        case DBTREE::PROXY_DISABLE: m_proxy_frame.rd_disable.set_active(); break;
        case DBTREE::PROXY_LOCAL: m_proxy_frame.rd_local.set_active(); break;
    }
    m_proxy_frame.entry_host.set_text( DBTREE::board_get_local_proxy( get_url() ) );
    m_proxy_frame.entry_port.set_text( MISC::itostr( DBTREE::board_get_local_proxy_port( get_url() ) ) );

    switch( DBTREE::board_get_mode_local_proxy_w( get_url() ) ){
        case DBTREE::PROXY_GLOBAL: m_proxy_frame_w.rd_global.set_active(); break;
        case DBTREE::PROXY_DISABLE: m_proxy_frame_w.rd_disable.set_active(); break;
        case DBTREE::PROXY_LOCAL: m_proxy_frame_w.rd_local.set_active(); break;
    }
    m_proxy_frame_w.entry_host.set_text( DBTREE::board_get_local_proxy_w( get_url() ) );
    m_proxy_frame_w.entry_port.set_text( MISC::itostr( DBTREE::board_get_local_proxy_port_w( get_url() ) ) );

    m_vbox_proxy.pack_start( m_label_proxy, Gtk::PACK_SHRINK );
    m_vbox_proxy.pack_start( m_proxy_frame, Gtk::PACK_SHRINK );
    m_vbox_proxy.pack_start( m_proxy_frame_w, Gtk::PACK_SHRINK );

    // あぼーん
    std::string str_id, str_name, str_word, str_regex;
    std::string str_thread, str_word_thread, str_regex_thread;
    std::list< std::string >::iterator it;

    // ID
    std::list< std::string > list_id = DBTREE::get_abone_list_id_board( get_url() );
    for( it = list_id.begin(); it != list_id.end(); ++it ) if( ! ( *it ).empty() ) str_id += ( *it ) + "\n";
    m_edit_id.set_text( str_id );

    // name
    std::list< std::string > list_name = DBTREE::get_abone_list_name_board( get_url() );
    for( it = list_name.begin(); it != list_name.end(); ++it ) if( ! ( *it ).empty() ) str_name += ( *it ) + "\n";
    m_edit_name.set_text( str_name );

    // word
    std::list< std::string > list_word = DBTREE::get_abone_list_word_board( get_url() );
    for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
    m_edit_word.set_text( str_word );

    // regex
    std::list< std::string > list_regex = DBTREE::get_abone_list_regex_board( get_url() );
    for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
    m_edit_regex.set_text( str_regex );

    m_label_warning.set_text( "ここでのあぼーん設定は「" +  DBTREE::board_name( get_url() ) + "」板の全スレに適用されます。\n\n設定のし過ぎは板内の全スレの表示速度を低下させます。\n\n設定のし過ぎに気を付けてください。\n\nなおNG IDはJDを再起動するとリセットされます。" );

    m_notebook_abone.append_page( m_label_warning, "注意" );
    m_notebook_abone.append_page( m_edit_id, "NG ID" );
    m_notebook_abone.append_page( m_edit_name, "NG 名前" );
    m_notebook_abone.append_page( m_edit_word, "NG ワード" );
    m_notebook_abone.append_page( m_edit_regex, "NG 正規表現" );

    // スレッドあぼーん

    // スレ数、時間
    m_label_abone_thread.set_text( "以下の数字が0の時は、設定メニューの全体あぼ〜ん設定で指定した数字が用いられます。\n\n" );

    m_label_number.set_text( "スレ以上のレスをあぼ〜ん" );
    m_spin_number.set_range( 0, 1001 );
    m_spin_number.set_increments( 1, 1 );
    m_spin_number.set_value( DBTREE::get_abone_number_thread( get_url() ) );
            
    m_hbox_number.set_spacing( 4 );
    m_hbox_number.pack_start( m_spin_number, Gtk::PACK_SHRINK );
    m_hbox_number.pack_start( m_label_number, Gtk::PACK_SHRINK );

    m_label_hour.set_text( "時間以上スレ立てから経過したレスをあぼ〜ん" );
    m_spin_hour.set_range( 0, 1000 );
    m_spin_hour.set_increments( 1, 1 );
    m_spin_hour.set_value( DBTREE::get_abone_hour_thread( get_url() ) );
            
    m_hbox_hour.set_spacing( 4 );
    m_hbox_hour.pack_start( m_spin_hour, Gtk::PACK_SHRINK );
    m_hbox_hour.pack_start( m_label_hour, Gtk::PACK_SHRINK );

    m_vbox_abone_thread.set_border_width( 16 );
    m_vbox_abone_thread.set_spacing( 8 );
    m_vbox_abone_thread.pack_start( m_label_abone_thread, Gtk::PACK_SHRINK );
    m_vbox_abone_thread.pack_start( m_hbox_number, Gtk::PACK_SHRINK );
    m_vbox_abone_thread.pack_start( m_hbox_hour, Gtk::PACK_SHRINK );

    // スレあぼーん
    std::list< std::string > list_thread = DBTREE::get_abone_list_thread( get_url() );
    for( it = list_thread.begin(); it != list_thread.end(); ++it ) if( ! ( *it ).empty() ) str_thread += ( *it ) + "\n";
    m_edit_thread.set_text( str_thread );

    // スレwordあぼーん
    std::list< std::string > list_word_thread = DBTREE::get_abone_list_word_thread( get_url() );
    for( it = list_word_thread.begin(); it != list_word_thread.end(); ++it ) if( ! ( *it ).empty() ) str_word_thread += ( *it ) + "\n";
    m_edit_word_thread.set_text( str_word_thread );

    // スレregexあぼーん
    std::list< std::string > list_regex_thread = DBTREE::get_abone_list_regex_thread( get_url() );
    for( it = list_regex_thread.begin(); it != list_regex_thread.end(); ++it ) if( ! ( *it ).empty() ) str_regex_thread += ( *it ) + "\n";
    m_edit_regex_thread.set_text( str_regex_thread );

    m_notebook_abone_thread.append_page( m_vbox_abone_thread, "一般" );
    m_notebook_abone_thread.append_page( m_edit_thread, "NG スレタイトル" );
    m_notebook_abone_thread.append_page( m_edit_word_thread, "NG ワード" );
    m_notebook_abone_thread.append_page( m_edit_regex_thread, "NG 正規表現" );

    // SETTING.TXT
    m_edit_settingtxt.set_editable( false );
    m_edit_settingtxt.set_text( DBTREE::settingtxt( get_url() ) );

    m_notebook.append_page( m_vbox, "一般" );
    m_notebook.append_page( *m_localrule, "ローカルルール" );
    m_notebook.append_page( m_vbox_proxy, "プロキシ設定" );
    m_notebook.append_page( m_notebook_abone, "あぼーん設定(スレビュー)" );
    m_notebook.append_page( m_notebook_abone_thread, "あぼーん設定(スレ一覧)" );
    m_notebook.append_page( m_edit_settingtxt, "SETTING.TXT" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Preferences::slot_switch_page ) );

    get_vbox()->pack_start( m_notebook );
    set_title( "「" + DBTREE::board_name( get_url() ) + "」のプロパティ" );
    resize( 600, 400 );
    show_all_children();
}


Preferences::~Preferences()
{
    if( m_localrule ) delete m_localrule;
    m_localrule = NULL;
}


void Preferences::slot_clear_samba()
{
    DBTREE::board_set_samba_sec( get_url(), 0 );
    m_label_samba.set_text( MISC::itostr( DBTREE::board_samba_sec( get_url() ) ) );
}


void Preferences::slot_delete_cookie()
{
    DBTREE::board_set_list_cookies_for_write( get_url(), std::list< std::string >() );
    DBTREE::board_set_hana_for_write( get_url(), std::string() );

    m_edit_cookies.set_text( "未取得" );
}



void Preferences::slot_switch_page( GtkNotebookPage*, guint page )
{
    if( m_notebook.get_nth_page( page ) == m_localrule ){
        m_localrule->set_command( "clear_screen" );
        m_localrule->set_command( "append_html", DBTREE::localrule( get_url() ) );
    }
}


//
// OK 押した
//
void Preferences::slot_ok_clicked()
{
    // プロクシ
    int mode = DBTREE::PROXY_GLOBAL;
    if( m_proxy_frame.rd_disable.get_active() ) mode = DBTREE::PROXY_DISABLE;
    else if( m_proxy_frame.rd_local.get_active() ) mode = DBTREE::PROXY_LOCAL;
    DBTREE::board_set_mode_local_proxy( get_url(), mode );
    DBTREE::board_set_local_proxy( get_url(), MISC::remove_space( m_proxy_frame.entry_host.get_text() ) );
    DBTREE::board_set_local_proxy_port( get_url(), atoi( m_proxy_frame.entry_port.get_text().c_str() ) );

    mode = DBTREE::PROXY_GLOBAL;
    if( m_proxy_frame_w.rd_disable.get_active() ) mode = DBTREE::PROXY_DISABLE;
    else if( m_proxy_frame_w.rd_local.get_active() ) mode = DBTREE::PROXY_LOCAL;

    DBTREE::board_set_mode_local_proxy_w( get_url(), mode );
    DBTREE::board_set_local_proxy_w( get_url(), MISC::remove_space( m_proxy_frame_w.entry_host.get_text() ) );
    DBTREE::board_set_local_proxy_port_w( get_url(), atoi( m_proxy_frame_w.entry_port.get_text().c_str() ) );

    // あぼーん再設定
    std::list< std::string > list_id = MISC::get_lines( m_edit_id.get_text() );
    std::list< std::string > list_name = MISC::get_lines( m_edit_name.get_text() );
    std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text() );
    std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text() );
    DBTREE::reset_abone_board( get_url(), list_id, list_name, list_word, list_regex ); 

    // スレあぼーん再設定
    std::list< std::string > list_thread = MISC::get_lines( m_edit_thread.get_text() );
    std::list< std::string > list_word_thread = MISC::get_lines( m_edit_word_thread.get_text() );
    std::list< std::string > list_regex_thread = MISC::get_lines( m_edit_regex_thread.get_text() );
    const int number = m_spin_number.get_value_as_int();
    const int hour = m_spin_hour.get_value_as_int();
    DBTREE::reset_abone_thread( get_url(), list_thread, list_word_thread, list_regex_thread, number, hour );  // 板の再描画も行われる

    // 書き込み設定
    DBTREE::board_set_check_noname( get_url(), m_check_noname.get_active() );
    DBTREE::board_set_write_name( get_url(), m_entry_writename.get_text() );

    std::string tmpmail = m_entry_writemail.get_text();
    if( tmpmail.empty() ) tmpmail = JD_MAIL_BLANK; // 空白の場合 JD_MAIL_BLANK をセットする
    DBTREE::board_set_write_mail( get_url(), tmpmail );

    DBTREE::board_save_info( get_url() );
}


bool Preferences::slot_timeout( int timer_number )
{
    if( m_localrule ) m_localrule->clock_in();    
    return true;
}
