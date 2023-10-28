// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "preference.h"

#include "dbtree/interface.h"
#include "dbtree/boardbase.h"

#include "skeleton/msgdiag.h"

#include "jdlib/misccharcode.h"
#include "jdlib/misctime.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "cache.h"
#include "command.h"
#include "global.h"
#include "viewfactory.h"

using namespace BOARD;

Preferences::Preferences( Gtk::Window* parent, const std::string& url, const std::string& command )
    : SKELETON::PrefDiag( parent, url )
    , m_frame_write( "書き込み設定" )
    , m_entry_writename( true, "名前：" )
    , m_entry_writemail( true, "メール：" )
    , m_check_utf8_post( "(実験的な機能) UTF-8で書き込む" )
    , m_check_noname( "名前欄が空白の時は書き込まない" )
    , m_bt_clear_post_history( "この板にある全スレの書き込み履歴クリア" )
    , m_bt_set_default_namemail( "デフォルト" )

    , m_frame_cookie( "クッキーと書き込みキーワード" )
    , m_button_cookie( "削除" )

    , m_check_live( "実況する" )

    , m_vbox_encoding{ Gtk::ORIENTATION_VERTICAL, 0 }
    , m_hbox_encoding{ Gtk::ORIENTATION_HORIZONTAL, 4 }
    , m_vbox_encoding_analysis_method{ Gtk::ORIENTATION_VERTICAL, 2 }

    , m_vbox_network{ Gtk::ORIENTATION_VERTICAL, 8 }
    , m_hbox_agent{ Gtk::ORIENTATION_HORIZONTAL, 2 }
    , m_label_agent{ "_User-Agent：", true }
    , m_proxy_frame( "読み込み用" )
    , m_proxy_frame_w( "書き込み用" )

    , m_label_name( false, "板タイトル：", DBTREE::board_name( get_url() ) )
    , m_label_url( false, "板のURL：", DBTREE::url_boardbase( get_url() ) )
    , m_label_cache( false, "ローカルキャッシュのルートパス",
                     CACHE::path_board_root( DBTREE::url_boardbase( get_url() ) ) )

    , m_label_noname( false, "デフォルト名無し：", DBTREE::default_noname( get_url() ) )
    , m_label_max_line( false, "1レスの最大改行数：" )
    , m_label_max_byte( false, "1レスの最大バイト数：" )
    , m_label_last_access( false, "最終アクセス日時 ：" )
    , m_label_modified( false, "最終更新日時 ：" )
    , m_button_clearmodified( "日時クリア" )
    , m_sep_samba{ Gtk::ORIENTATION_VERTICAL }
    , m_label_samba( false, "書き込み規制秒数 (Samba24) ：" )
    , m_button_clearsamba( "秒数クリア" )
    , m_check_oldlog( "過去ログを表示する" )
    , m_vbox_abone_general{ Gtk::ORIENTATION_VERTICAL, 0 }
    , m_sep_general{ Gtk::ORIENTATION_HORIZONTAL }
    , m_hbox_abone_consecutive{ Gtk::ORIENTATION_HORIZONTAL, 0 }
    , m_label_abone_consecutive{ "(実験的な機能) 連続投稿したIDをスレのNG IDに追加する (0 : 未設定)：" }
    , m_label_abone_consecutive_over_n_times{ " 回以上" }
    , m_hbox_low_number{ Gtk::ORIENTATION_HORIZONTAL, 4 }
    , m_hbox_high_number{ Gtk::ORIENTATION_HORIZONTAL, 4 }
    , m_button_remove_old_title( "dat落ちしたスレのタイトルを削除する" )
{
    m_edit_cookies.set_editable( false );

    // 書き込み設定
    const int samba_sec = DBTREE::board_samba_sec( get_url() );
    if( ! samba_sec ) m_label_samba.set_text( "未取得" );
    else m_label_samba.set_text( std::to_string( samba_sec ) );

    m_button_clearsamba.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_samba ) );
    m_hbox_samba.pack_start( m_check_utf8_post, Gtk::PACK_SHRINK );
    m_hbox_samba.pack_start( m_sep_samba, Gtk::PACK_SHRINK );
    m_hbox_samba.pack_start( m_label_samba );
    m_hbox_samba.pack_start( m_button_clearsamba, Gtk::PACK_SHRINK );    
    m_check_utf8_post.set_margin_end( 15 );
    m_label_samba.set_margin_start( 15 );
    m_sep_samba.set_hexpand( false );
    m_check_utf8_post.set_tooltip_text(
        "掲示板がUTF-8の書き込みに対応してるか確認して使用してください。\n"
        "このオプションは実験的なサポートのため変更または廃止の可能性があります。" );

    m_check_utf8_post.set_active( DBTREE::board_check_utf8_post( get_url() ) );
    m_check_noname.set_active( DBTREE::board_check_noname( get_url() ) );

    m_entry_writename.set_text( DBTREE::board_get_write_name( get_url() ) ); 
    if( m_entry_writename.get_text().empty() ) m_entry_writename.set_text( CONFIG::get_write_name() );
    // JD_NAME_BLANK の場合空白をセットする
    else if( m_entry_writename.get_text() == JD_NAME_BLANK ) m_entry_writename.set_text( std::string() );

    m_entry_writemail.set_text( DBTREE::board_get_write_mail( get_url() ) );
    if( m_entry_writemail.get_text().empty() ) m_entry_writemail.set_text( CONFIG::get_write_mail() );
    // JD_MAIL_BLANK の場合空白をセットする
    else if( m_entry_writemail.get_text() == JD_MAIL_BLANK ) m_entry_writemail.set_text( std::string() );

    m_bt_clear_post_history.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_post_history ) );
    m_hbox_write1.set_spacing( 8 );
    m_hbox_write1.pack_start( m_check_noname );
    m_hbox_write1.pack_start( m_bt_clear_post_history );

    m_bt_set_default_namemail.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_set_default_namemail ) );

    m_hbox_write2.set_spacing( 8 );
    m_hbox_write2.pack_start( m_entry_writename );
    m_hbox_write2.pack_end( m_bt_set_default_namemail, Gtk::PACK_SHRINK );
    m_hbox_write2.pack_end( m_entry_writemail, Gtk::PACK_SHRINK );

    m_vbox_write.set_border_width( 8 );
    m_vbox_write.set_spacing( 8 );
    m_vbox_write.pack_start( m_hbox_samba,  Gtk::PACK_SHRINK );
    m_vbox_write.pack_start( m_hbox_write1, Gtk::PACK_SHRINK );
    m_vbox_write.pack_start( m_hbox_write2, Gtk::PACK_SHRINK );

    m_frame_write.add( m_vbox_write );

    set_activate_entry( m_entry_writename );
    set_activate_entry( m_entry_writemail );

    // cookie と 書き込みキーワード の設定
    std::string str_cookies;
    const std::string temp_cookies = DBTREE::board_cookie_by_host( get_url() );
    if( temp_cookies.empty() ) {
        str_cookies = "クッキー:\n未取得\n";
    }
    else {
        str_cookies = "クッキー:\n" + MISC::Iconv( temp_cookies, Encoding::utf8, DBTREE::board_encoding( get_url() ) ) + "\n";
    }

    const std::string& kw_write = DBTREE::board_keyword_for_write( get_url() );
    if( ! kw_write.empty() ) str_cookies.append( "\n書き込み用キーワード: " + kw_write + "\n" );

    const std::string& kw_new = DBTREE::board_keyword_for_newarticle( get_url() );
    if( ! kw_new.empty() ) str_cookies.append( "\nスレ立て用キーワード: " + kw_new + "\n" );

    m_edit_cookies.set_hexpand( true );
    m_edit_cookies.set_propagate_natural_height( true );
    m_edit_cookies.set_text( str_cookies );

    m_grid_cookie.property_margin() = 8;
    m_grid_cookie.attach( m_edit_cookies, 0, 0, 1, 1 );
    m_grid_cookie.attach( m_button_cookie, 1, 0, 1, 1 );
    m_button_cookie.set_valign( Gtk::ALIGN_END );
    m_button_cookie.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_delete_cookie ) );

    m_frame_cookie.add( m_grid_cookie );

    // 実況
    const int live_sec = DBTREE::board_get_live_sec( get_url() );
    m_label_live.set_text( "実況時の更新間隔(秒)：" );

    m_spin_live.set_range( MIN_LIVE_RELOAD_SEC, 1200 );
    m_spin_live.set_increments( 1, 1 );
    m_spin_live.set_value( MAX( live_sec, MIN_LIVE_RELOAD_SEC ) );
    if( live_sec ){
        m_check_live.set_active( true );
        m_spin_live.set_sensitive( true );
    }
    else{
        m_check_live.set_active( false );
        m_spin_live.set_sensitive( false );
    }
    m_check_live.signal_toggled().connect( sigc::mem_fun(*this, &Preferences::slot_check_live ) );

    m_hbox_live.set_spacing( 4 );
    m_hbox_live.pack_start( m_label_live, Gtk::PACK_SHRINK );
    m_hbox_live.pack_start( m_spin_live, Gtk::PACK_SHRINK );
    m_hbox_live.pack_start( m_check_live );

    set_activate_entry( m_spin_live );

    // テキストエンコーディング
    const char* tmpcharset = MISC::encoding_to_cstr( DBTREE::board_encoding( get_url() ) );
    if( CONFIG::get_choose_character_encoding() ) {
        m_label_charset.set_text( "テキストエンコーディング : " );
        m_combo_charset.append( MISC::encoding_to_cstr( Encoding::utf8 ) );
        m_combo_charset.append( MISC::encoding_to_cstr( Encoding::sjis ) );
        m_combo_charset.append( MISC::encoding_to_cstr( Encoding::eucjp ) );
        m_combo_charset.set_active_text( tmpcharset );

        m_hbox_encoding.pack_start( m_label_charset, Gtk::PACK_SHRINK );
        m_hbox_encoding.pack_start( m_combo_charset, Gtk::PACK_SHRINK );

        m_vbox_encoding.pack_start( m_hbox_encoding, Gtk::PACK_SHRINK );
    }
    else {
        // エンコーディング設定は安全でないので無効のときは設定欄(コンボボックス)を表示しない
        m_label_charset.set_text( Glib::ustring::compose( "テキストエンコーディング :  %1", tmpcharset ) );

        m_hbox_encoding.pack_start( m_label_charset, Gtk::PACK_SHRINK );

        m_binding_encoding = Glib::Binding::bind_property( m_toggle_encoding.property_active(),
                                                           m_revealer_encoding.property_reveal_child() );
        m_toggle_encoding.set_label( "(実験的な機能) エンコーディングの判定" );
        m_toggle_encoding.set_tooltip_text(
            "このオプションは実験的なサポートのため変更または廃止の可能性があります。" );
        m_toggle_encoding.set_halign( Gtk::ALIGN_END );
        m_hbox_encoding.pack_end( m_toggle_encoding, Gtk::PACK_SHRINK );

        m_label_encoding_analysis_method.set_markup( "<b>テキストエンコーディングを判定する方法</b>" );
        m_label_encoding_analysis_method.set_halign( Gtk::ALIGN_START );

        m_radio_encoding_default.set_group( m_group_encoding );
        m_radio_encoding_default.set_label( "デフォルト設定を使う" );

        m_radio_encoding_http_header.set_group( m_group_encoding );
        m_radio_encoding_http_header.set_label( "HTTPヘッダーのエンコーディング情報を使う" );

        m_radio_encoding_guess.set_group( m_group_encoding );
        m_radio_encoding_guess.set_label( "テキストからエンコーディングを推測する" );

        m_vbox_encoding_analysis_method.set_margin_start( 30 );
        m_vbox_encoding_analysis_method.set_margin_end( 30 );
        m_vbox_encoding_analysis_method.set_margin_top( 10 );
        m_vbox_encoding_analysis_method.set_margin_bottom( 10 );
        m_vbox_encoding_analysis_method.pack_start( m_label_encoding_analysis_method, Gtk::PACK_SHRINK );
        m_vbox_encoding_analysis_method.pack_start( m_radio_encoding_default, Gtk::PACK_SHRINK );
        m_vbox_encoding_analysis_method.pack_start( m_radio_encoding_http_header, Gtk::PACK_SHRINK );
        m_vbox_encoding_analysis_method.pack_start( m_radio_encoding_guess, Gtk::PACK_SHRINK );

        m_revealer_encoding.add( m_vbox_encoding_analysis_method );

        m_vbox_encoding.pack_start( m_hbox_encoding, Gtk::PACK_SHRINK );
        m_vbox_encoding.pack_start( m_revealer_encoding, Gtk::PACK_SHRINK );

        switch( DBTREE::board_encoding_analysis_method( get_url() ) ) {
            case EncodingAnalysisMethod::http_header:
                m_radio_encoding_http_header.set_active( true );
                break;
            case EncodingAnalysisMethod::guess:
                m_radio_encoding_guess.set_active( true );
                break;
            default:
                m_radio_encoding_default.set_active( true );
                break;
        }
    }

    // 一般ページのパッキング
    m_label_max_line.set_text( std::to_string( DBTREE::line_number( get_url() ) * 2 ) );
    m_label_max_byte.set_text( std::to_string( DBTREE::message_count( get_url() ) ) );
    m_hbox_max.pack_start( m_label_max_line );
    m_hbox_max.pack_start( m_label_max_byte );

    // 最大レス数
    const int max_res = DBTREE::board_get_number_max_res( get_url() );
    m_label_maxres.set_text( "最大レス数 (0 : 未設定)：" );
    m_spin_maxres.set_range( 0, CONFIG::get_max_resnumber() );
    m_spin_maxres.set_increments( 1, 1 );
    m_spin_maxres.set_value( max_res );
    m_spin_maxres.set_sensitive( true );

    m_hbox_max.pack_start( m_label_maxres, Gtk::PACK_SHRINK );
    m_hbox_max.pack_start( m_spin_maxres, Gtk::PACK_SHRINK );

    set_activate_entry( m_spin_maxres );

    const time_t last_access = DBTREE::board_last_access_time( get_url() );
    if( last_access ) m_label_last_access.set_text(
        MISC::timettostr( last_access, MISC::TIME_WEEK )
        + " ( " + MISC::timettostr( last_access, MISC::TIME_PASSED ) + " )"
        );

    if( DBTREE::board_date_modified( get_url() ).empty() ) m_label_modified.set_text( "未取得" );
    else m_label_modified.set_text(
        MISC::timettostr( DBTREE::board_time_modified( get_url() ), MISC::TIME_WEEK )
        + " ( " + MISC::timettostr( DBTREE::board_time_modified( get_url() ), MISC::TIME_PASSED ) + " )"
        );

    m_button_clearmodified.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_modified ) );
    m_hbox_modified.pack_start( m_label_modified );
    m_hbox_modified.pack_start( m_button_clearmodified, Gtk::PACK_SHRINK );    

    // 過去ログ表示
    if( CONFIG::get_show_oldarticle() ){

        m_check_oldlog.set_active( true );
        m_check_oldlog.set_sensitive( false );
    }
    else m_check_oldlog.set_active( DBTREE::board_show_oldlog( get_url() ) );

    m_vbox.set_border_width( 16 );
    m_vbox.set_spacing( 8 );
    m_vbox.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_cache, Gtk::PACK_SHRINK );

    m_vbox.pack_start( m_label_noname, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_max, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_last_access, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_modified, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_live, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_vbox_encoding, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_check_oldlog, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_frame_write, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_frame_cookie, Gtk::PACK_SHRINK );

    // ローカルルール
    m_localrule.reset( CORE::ViewFactory( CORE::VIEW_ARTICLEINFO, get_url() ) );

    // ネットワーク設定
    m_vbox_network.set_border_width( 16 );

    // ユーザーエージェント
    m_comment_agent.set_text( "通常は about:config で設定したユーザーエージェント(UA)で板にアクセスします\n"
                              "about:config の設定と異なるUAを使用する場合はここで設定してください (最大200字)\n"
                              "空欄または不適切な文字が含まれている状態でOKを押すと設定が消去されます" );

    m_label_agent.set_hexpand( false );
    m_label_agent.set_mnemonic_widget( m_entry_agent );

    m_entry_agent.property_truncate_multiline() = true;
    m_entry_agent.set_input_purpose( Gtk::INPUT_PURPOSE_ALPHA );
    m_entry_agent.set_max_length( 200 );
    m_entry_agent.set_placeholder_text( "空欄のときは about:config の設定が使われます" );
    const std::string& board_agent = DBTREE::board_get_board_agent( get_url() );
    if( ! board_agent.empty() ) {
        m_entry_agent.set_text( board_agent );
    }

    m_hbox_agent.set_margin_top( 8 );
    m_hbox_agent.set_margin_bottom( 12 );
    m_hbox_agent.pack_start( m_label_agent, Gtk::PACK_SHRINK );
    m_hbox_agent.pack_start( m_entry_agent );

    // プロキシ
    std::string host;

    m_comment_proxy.set_text( "通常は全体プロキシ設定でプロキシを設定します\n"
                              "全体プロキシ設定と異なるプロキシを使用する場合はここで設定してください" );

    switch( DBTREE::board_get_mode_local_proxy( get_url() ) ){
        case DBTREE::PROXY_GLOBAL: m_proxy_frame.rd_global.set_active(); break;
        case DBTREE::PROXY_DISABLE: m_proxy_frame.rd_disable.set_active(); break;
        case DBTREE::PROXY_LOCAL: m_proxy_frame.rd_local.set_active(); break;
    }
    if( DBTREE::board_get_local_proxy_basicauth( get_url() ).empty() ) host = DBTREE::board_get_local_proxy( get_url() );
    else host = DBTREE::board_get_local_proxy_basicauth( get_url() ) + "@" + DBTREE::board_get_local_proxy( get_url() );
    m_proxy_frame.entry_host.set_text( host );
    m_proxy_frame.entry_port.set_text( std::to_string( DBTREE::board_get_local_proxy_port( get_url() ) ) );

    switch( DBTREE::board_get_mode_local_proxy_w( get_url() ) ){
        case DBTREE::PROXY_GLOBAL: m_proxy_frame_w.rd_global.set_active(); break;
        case DBTREE::PROXY_DISABLE: m_proxy_frame_w.rd_disable.set_active(); break;
        case DBTREE::PROXY_LOCAL: m_proxy_frame_w.rd_local.set_active(); break;
    }
    if( DBTREE::board_get_local_proxy_basicauth_w( get_url() ).empty() ) host = DBTREE::board_get_local_proxy_w( get_url() );
    else host = DBTREE::board_get_local_proxy_basicauth_w( get_url() ) + "@" + DBTREE::board_get_local_proxy_w( get_url() );
    m_proxy_frame_w.entry_host.set_text( host );
    m_proxy_frame_w.entry_port.set_text( std::to_string( DBTREE::board_get_local_proxy_port_w( get_url() ) ) );

    m_vbox_network.pack_start( m_comment_agent, Gtk::PACK_SHRINK );
    m_vbox_network.pack_start( m_hbox_agent, Gtk::PACK_SHRINK );
    m_vbox_network.pack_start( m_comment_proxy, Gtk::PACK_SHRINK );
    m_vbox_network.pack_start( m_proxy_frame, Gtk::PACK_SHRINK );
    m_vbox_network.pack_start( m_proxy_frame_w, Gtk::PACK_SHRINK );

    set_activate_entry( m_proxy_frame.entry_host );
    set_activate_entry( m_proxy_frame.entry_port );
    set_activate_entry( m_proxy_frame_w.entry_host );
    set_activate_entry( m_proxy_frame_w.entry_port );

    // あぼーん

    // ID
    std::list< std::string > list_id = DBTREE::get_abone_list_id_board( get_url() );
    m_edit_id.set_text( MISC::concat_with_suffix( list_id, '\n' ) );

    // name
    std::list< std::string > list_name = DBTREE::get_abone_list_name_board( get_url() );
    m_edit_name.set_text( MISC::concat_with_suffix( list_name, '\n' ) );

    // word
    std::list< std::string > list_word = DBTREE::get_abone_list_word_board( get_url() );
    m_edit_word.set_text( MISC::concat_with_suffix( list_word, '\n' ) );

    // regex
    std::list< std::string > list_regex = DBTREE::get_abone_list_regex_board( get_url() );
    m_edit_regex.set_text( MISC::concat_with_suffix( list_regex, '\n' ) );

    // 一般
    m_label_warning_subject.set_markup( R"(<span size="large" weight="bold">注意</span>)" );
    m_label_warning_subject.set_margin_top( 20 );
    m_label_warning_subject.set_margin_bottom( 20 );

    m_label_warning.set_text(
        "ここでのあぼーん設定は「" +  DBTREE::board_name( get_url() ) + "」板の全スレに適用されます。\n\n"
        "設定のし過ぎは板内の全スレの表示速度を低下させます。\n\n設定のし過ぎに気を付けてください。\n\n"
        "なおNG IDはJDimを再起動するとリセットされます。" );

    m_sep_general.set_margin_top( 50 );
    m_sep_general.set_margin_bottom( 40 );

    // 連続投稿したIDをスレのNG IDに追加
    m_spin_abone_consecutive.set_increments( 1, 1 );
    m_spin_abone_consecutive.set_sensitive( true );
    m_spin_abone_consecutive.set_range( 0, 1000 );
    const int abone_consecutive = DBTREE::board_get_abone_consecutive( get_url() );
    m_spin_abone_consecutive.set_value( abone_consecutive );

    m_hbox_abone_consecutive.pack_start( m_label_abone_consecutive, Gtk::PACK_SHRINK );
    m_hbox_abone_consecutive.pack_start( m_spin_abone_consecutive, Gtk::PACK_SHRINK );
    m_hbox_abone_consecutive.pack_start( m_label_abone_consecutive_over_n_times, Gtk::PACK_SHRINK );
    m_hbox_abone_consecutive.set_tooltip_text(
        "このオプションは実験的なサポートのため変更または廃止の可能性があります。" );

    m_vbox_abone_general.pack_start( m_label_warning_subject, Gtk::PACK_SHRINK );
    m_vbox_abone_general.pack_start( m_label_warning, Gtk::PACK_SHRINK );
    m_vbox_abone_general.pack_start( m_sep_general, Gtk::PACK_SHRINK );
    m_vbox_abone_general.pack_start( m_hbox_abone_consecutive, Gtk::PACK_SHRINK );
    m_vbox_abone_general.set_margin_start( 20 );
    m_vbox_abone_general.set_margin_end( 20 );

    m_notebook_abone.append_page( m_vbox_abone_general, "一般" );
    m_notebook_abone.append_page( m_edit_id, "NG ID" );
    m_notebook_abone.append_page( m_edit_name, "NG 名前" );
    m_notebook_abone.append_page( m_edit_word, "NG ワード" );
    m_notebook_abone.append_page( m_edit_regex, "NG 正規表現" );

    // スレッドあぼーん

    // スレ数、時間
    m_label_abone_thread.set_text( "以下の数字が0の時は、設定メニューの全体あぼ〜ん設定で指定した数字が用いられます。\nまたキャッシュにログがあるスレはあぼ〜んされません。\n\n" );

    m_label_low_number.set_text( "レス以下のスレをあぼ〜ん" );
    m_spin_low_number.set_range( 0, CONFIG::get_max_resnumber() );
    m_spin_low_number.set_increments( 1, 1 );
    m_spin_low_number.set_value( DBTREE::get_abone_low_number_thread( get_url() ) );

    m_hbox_low_number.pack_start( m_spin_low_number, Gtk::PACK_SHRINK );
    m_hbox_low_number.pack_start( m_label_low_number, Gtk::PACK_SHRINK );

    set_activate_entry( m_spin_low_number );

    m_label_high_number.set_text( "レス以上のスレをあぼ〜ん" );
    m_spin_high_number.set_range( 0, CONFIG::get_max_resnumber() );
    m_spin_high_number.set_increments( 1, 1 );
    m_spin_high_number.set_value( DBTREE::get_abone_high_number_thread( get_url() ) );

    m_hbox_high_number.pack_start( m_spin_high_number, Gtk::PACK_SHRINK );
    m_hbox_high_number.pack_start( m_label_high_number, Gtk::PACK_SHRINK );

    set_activate_entry( m_spin_high_number );

    m_label_hour.set_text( "時間以上スレ立てから経過したスレをあぼ〜ん" );
    m_spin_hour.set_range( 0, 9999 );
    m_spin_hour.set_increments( 1, 1 );
    m_spin_hour.set_value( DBTREE::get_abone_hour_thread( get_url() ) );
            
    m_hbox_hour.set_spacing( 4 );
    m_hbox_hour.pack_start( m_spin_hour, Gtk::PACK_SHRINK );
    m_hbox_hour.pack_start( m_label_hour, Gtk::PACK_SHRINK );

    set_activate_entry( m_spin_hour );

    m_vbox_abone_thread.set_border_width( 16 );
    m_vbox_abone_thread.set_spacing( 8 );
    m_vbox_abone_thread.pack_start( m_label_abone_thread, Gtk::PACK_SHRINK );
    m_vbox_abone_thread.pack_start( m_hbox_low_number, Gtk::PACK_SHRINK );
    m_vbox_abone_thread.pack_start( m_hbox_high_number, Gtk::PACK_SHRINK );
    m_vbox_abone_thread.pack_start( m_hbox_hour, Gtk::PACK_SHRINK );

    // スレあぼーん
    std::list< std::string > list_thread = DBTREE::get_abone_list_thread( get_url() );
    m_edit_thread.set_text( MISC::concat_with_suffix( list_thread, '\n' ) );

    m_button_remove_old_title.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_remove_old_title ) );
    m_vbox_abone_title.pack_start( m_edit_thread );
    m_vbox_abone_title.pack_start( m_button_remove_old_title, Gtk::PACK_SHRINK );

    // スレwordあぼーん
    std::list< std::string > list_word_thread = DBTREE::get_abone_list_word_thread( get_url() );
    m_edit_word_thread.set_text( MISC::concat_with_suffix( list_word_thread, '\n' ) );

    // スレregexあぼーん
    std::list< std::string > list_regex_thread = DBTREE::get_abone_list_regex_thread( get_url() );
    m_edit_regex_thread.set_text( MISC::concat_with_suffix( list_regex_thread, '\n' ) );

    m_notebook_abone_thread.append_page( m_vbox_abone_thread, "一般" );
    m_notebook_abone_thread.append_page( m_vbox_abone_title, "NG スレタイトル" );
    m_notebook_abone_thread.append_page( m_edit_word_thread, "NG ワード" );
    m_notebook_abone_thread.append_page( m_edit_regex_thread, "NG 正規表現" );

    // SETTING.TXT
    m_edit_settingtxt.set_editable( false );
    m_edit_settingtxt.set_text( DBTREE::settingtxt( get_url() ) );

    // ディスプレイ解像度が小さい環境で表示できるようにスクロール可能にする
    m_scroll_vbox.add( m_vbox );
    m_scroll_vbox.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
    m_scroll_network.add( m_vbox_network );
    m_scroll_network.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
    m_notebook.set_scrollable( true );

    m_notebook.append_page( m_scroll_vbox, "一般" );
    const int page_localrule = 1;
    m_notebook.append_page( *m_localrule, "ローカルルール" );
    m_notebook.append_page( m_scroll_network, "ネットワーク設定" );
    const int page_abone_article = 3;
    m_notebook.append_page( m_notebook_abone, "あぼ〜ん設定(スレビュー)" );
    m_notebook.append_page( m_notebook_abone_thread, "あぼ〜ん設定(スレ一覧)" );
    m_notebook.append_page( m_edit_settingtxt, "SETTING.TXT" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Preferences::slot_switch_page ) );

    get_content_area()->pack_start( m_notebook );
    set_title( "「" + DBTREE::board_name( get_url() ) + "」のプロパティ" );
    // ウインドウの自然なサイズを設定するがディスプレイに合わせて調整される
    set_default_size( 850, 750 );
    show_all_children();

    if( command == "show_localrule" ) m_notebook.set_current_page( page_localrule );
    else if( command == "show_abone_article" ) m_notebook.set_current_page( page_abone_article );
}


void Preferences::slot_clear_modified()
{
    DBTREE::board_set_date_modified( get_url(), "" );

    if( DBTREE::board_date_modified( get_url() ).empty() ) m_label_modified.set_text( "未取得" );
    else m_label_modified.set_text(
        MISC::timettostr( DBTREE::board_time_modified( get_url() ), MISC::TIME_WEEK )
        + " ( " + MISC::timettostr( DBTREE::board_time_modified( get_url() ), MISC::TIME_PASSED ) + " )"
        );
}


void Preferences::slot_clear_samba()
{
    DBTREE::board_set_samba_sec( get_url(), 0 );

    const int samba_sec = DBTREE::board_samba_sec( get_url() );
    if( ! samba_sec ) m_label_samba.set_text( "未取得" );
    else m_label_samba.set_text( std::to_string( samba_sec ) );
}


void Preferences::slot_clear_post_history()
{
    SKELETON::MsgDiag mdiag( nullptr, "この板にある全てのスレの書き込み履歴を削除しますか？\n\nスレ数によっては時間がかかります。",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    DBTREE::board_clear_all_post_history( get_url() );

    // スレ一覧とスレビューの表示更新
    CORE::core_set_command( "update_board", DBTREE::url_boardbase( get_url() ) );
    CORE::core_set_command( "redraw_article" );
}


void Preferences::slot_set_default_namemail()
{
    m_entry_writename.set_text( CONFIG::get_write_name() );
    m_entry_writemail.set_text( CONFIG::get_write_mail() );
}


void Preferences::slot_delete_cookie()
{
    DBTREE::board_delete_cookies( get_url() );
    DBTREE::board_set_keyword_for_write( get_url(), std::string() );
    DBTREE::board_set_keyword_for_newarticle( get_url(), std::string() );

    m_edit_cookies.set_text( "クッキー:\n未取得\n" );
}


void Preferences::slot_check_live()
{
    if( m_check_live.get_active() ){
        m_spin_live.set_sensitive( true );
        SKELETON::MsgDiag mdiag( nullptr,
                                 "実況を許された板以外では実況しないようにして下さい。\n\n"
                                 "実況状態のまま閉じたスレはJDim終了時に削除されます。"
                                 "詳しくはマニュアルの実況の項目を参照して下さい。",
                                 false, Gtk::MESSAGE_WARNING );
        mdiag.run();
    }
    else m_spin_live.set_sensitive( false );
}


void Preferences::slot_remove_old_title()
{

    if( ! DBTREE::board_list_subject( get_url() ).size() ){
        SKELETON::MsgDiag mdiag( nullptr, "再読み込みしてスレ一覧を更新して下さい。", false, Gtk::MESSAGE_WARNING );
        mdiag.run();
        return;
    }

    const std::list< std::string > list_thread = DBTREE::get_abone_list_thread_remove( get_url() );
    m_edit_thread.set_text( MISC::concat_with_suffix( list_thread, '\n' ) );
}


void Preferences::slot_switch_page( Gtk::Widget*, guint page )
{
    if( m_notebook.get_nth_page( page ) == m_localrule.get() ){
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
    DBTREE::board_set_local_proxy( get_url(), MISC::utf8_trim( m_proxy_frame.entry_host.get_text().raw() ) );
    DBTREE::board_set_local_proxy_port( get_url(), atoi( m_proxy_frame.entry_port.get_text().c_str() ) );

    mode = DBTREE::PROXY_GLOBAL;
    if( m_proxy_frame_w.rd_disable.get_active() ) mode = DBTREE::PROXY_DISABLE;
    else if( m_proxy_frame_w.rd_local.get_active() ) mode = DBTREE::PROXY_LOCAL;

    DBTREE::board_set_mode_local_proxy_w( get_url(), mode );
    DBTREE::board_set_local_proxy_w( get_url(), MISC::utf8_trim( m_proxy_frame_w.entry_host.get_text().raw() ) );
    DBTREE::board_set_local_proxy_port_w( get_url(), atoi( m_proxy_frame_w.entry_port.get_text().c_str() ) );

    // 書き込み設定
    DBTREE::board_set_check_utf8_post( get_url(), m_check_utf8_post.get_active() );
    DBTREE::board_set_check_noname( get_url(), m_check_noname.get_active() );

    std::string tmpname = m_entry_writename.get_text();
    if( tmpname == CONFIG::get_write_name() ) tmpname = std::string();
    else if( tmpname.empty() ) tmpname = JD_NAME_BLANK; // 空白の場合 JD_NAME_BLANK をセットする
    DBTREE::board_set_write_name( get_url(), tmpname );

    std::string tmpmail = m_entry_writemail.get_text();
    if( tmpmail == CONFIG::get_write_mail() ) tmpmail = std::string();
    else if( tmpmail.empty() ) tmpmail = JD_MAIL_BLANK; // 空白の場合 JD_MAIL_BLANK をセットする
    DBTREE::board_set_write_mail( get_url(), tmpmail );

    // charset
    if( m_combo_charset.get_mapped() ) {
        const std::string tmpcharset = m_combo_charset.get_active_text();
        DBTREE::board_set_encoding( get_url(), MISC::encoding_from_sv( tmpcharset ) );
    }

    // 実況間隔
    int live_sec = 0;
    if( m_check_live.get_active() ) live_sec = m_spin_live.get_value_as_int();
    DBTREE::board_set_live_sec( get_url(), live_sec );
    CORE::core_set_command( "redraw_article_toolbar" );

    // テキストエンコーディングを判定する方法
    if( m_radio_encoding_http_header.get_active() ) {
        DBTREE::board_set_encoding_analysis_method( get_url(), EncodingAnalysisMethod::http_header );
    }
    else if( m_radio_encoding_guess.get_active() ) {
        DBTREE::board_set_encoding_analysis_method( get_url(), EncodingAnalysisMethod::guess );
    }
    else {
        DBTREE::board_set_encoding_analysis_method( get_url(), EncodingAnalysisMethod::use_default );
    }

    // 最大レス数
    const int number_max_res = m_spin_maxres.get_value_as_int();
    DBTREE::board_set_number_max_res( get_url(), number_max_res );

    // 過去ログ表示
    if( ! CONFIG::get_show_oldarticle() ) DBTREE::board_set_show_oldlog( get_url(), m_check_oldlog.get_active() );

    // ユーザーエージェント
    std::string board_agent;
    const Glib::ustring agent_text = m_entry_agent.get_text();
    if( ! agent_text.empty() ) {
        // 不適切な文字が含まれてないかチェックと先頭末尾の空白文字を削除する
        const std::string& raw = agent_text.raw();
        if( std::all_of( raw.begin(), raw.end(), []( char c ) { return g_ascii_isprint( c ); } ) ) {
            board_agent = MISC::ascii_trim( raw );
        }
    }
    if( board_agent != DBTREE::board_get_board_agent( get_url() ) ) {
        DBTREE::board_set_board_agent( get_url(), board_agent );
    }

    // 連続投稿したIDをスレのNG IDに追加 (回数)
    const int abone_consecutive = m_spin_abone_consecutive.get_value_as_int();
    DBTREE::board_set_abone_consecutive( get_url(), abone_consecutive );

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
    const int low_number = m_spin_low_number.get_value_as_int();
    const int high_number = m_spin_high_number.get_value_as_int();
    const int hour = m_spin_hour.get_value_as_int();

    const bool redraw = true; // ここでスレ一覧の再描画指定をする
    DBTREE::reset_abone_thread( get_url(), list_thread, list_word_thread, list_regex_thread,
                                low_number, high_number, hour, redraw );

    DBTREE::board_save_info( get_url() );
}


void Preferences::timeout()
{
    if( m_localrule ) m_localrule->clock_in();    
}
