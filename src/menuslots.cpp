// ライセンス: GPL2

// メインウィンドウのメニューのslot関数

//#define _DEBUG
#include "jddebug.h"

#include "core.h"
#include "command.h"
#include "cache.h"
#include "session.h"
#include "login2ch.h"
#include "loginbe.h"
#include "loginp2.h"
#include "winmain.h"
#include "fontid.h"
#include "colorid.h"
#include "global.h"

#include "linkfiltermanager.h"
#include "prefdiagfactory.h"
#include "environment.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "control/controlutil.h"

#include "skeleton/msgdiag.h"
#include "skeleton/aboutdiag.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "jdlib/miscutil.h"

#include "message/logmanager.h"

#include "bbslist/bbslistadmin.h"
#include "board/boardadmin.h"
#include "article/articleadmin.h"
#include "image/imageadmin.h"
#include "message/messageadmin.h"


using namespace CORE;


//
// オンライン、オフライン切替え
//
void Core::slot_toggle_online()
{
    SESSION::set_online( !SESSION::is_online() );
    set_maintitle();

    // オートリロードキャンセル
    if( ! SESSION::is_online() ){
        BOARD::get_admin()->set_command( "cancel_reload" );
        ARTICLE::get_admin()->set_command( "cancel_reload" );
    }
}


//
// 2chにログイン
//
void Core::slot_toggle_login2ch()
{
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_login2ch\n";
#endif

    // ログイン中ならログアウト
    if( CORE::get_login2ch()->login_now() ){
        CORE::get_login2ch()->logout();
        set_maintitle();
    }

    // ログオフ中ならログイン開始
    else CORE::get_login2ch()->start_login();
}


//
// BEにログイン
//
void Core::slot_toggle_loginbe()
{
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_loginbe\n";
#endif

    // ログイン中ならログアウト
    if( CORE::get_loginbe()->login_now() ) CORE::get_loginbe()->logout();

    // ログオフ中ならログイン開始
    else CORE::get_loginbe()->start_login();

    set_maintitle();
}


//
// p2にログイン
//
void Core::slot_toggle_loginp2()
{
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_loginp2\n";
#endif

    // ログイン中ならログアウト
    if( CORE::get_loginp2()->login_now() ){
        CORE::get_loginp2()->logout();
        set_maintitle();
    }

    // ログオフ中ならログイン開始
    else CORE::get_loginp2()->start_login();
}


//
// 板リスト再読込
//
void Core::slot_reload_list()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    DBTREE::download_bbsmenu();
    CORE::core_set_command( "set_status","", "板一覧再読み込み中...." );        
}


//
// お気に入り保存
//
void Core::slot_save_favorite()
{
    CORE::core_set_command( "save_favorite","" );
}


//
// 終了
//
void Core::slot_quit()
{
    m_win_main.hide();
}


//
// ツールバーの表示モード
//
void Core::slot_toggle_toolbarpos( int pos )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_toolbarpos pos = " << pos << " / " << SESSION::toolbar_pos() << std::endl;
#endif

    pack_widget( true );

    // 表示切り替え
    if( SESSION::toolbar_pos() == pos ) pos = SESSION::TOOLBAR_POS_NOT;
    SESSION::set_toolbar_pos( pos );

    pack_widget( false );

    restore_focus( true, false );
}


//
// 板一覧のツールバー表示切り替え
//
void Core::slot_toggle_toolbarbbslist()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    SESSION::set_show_bbslist_toolbar( ! SESSION::get_show_bbslist_toolbar() );
    BBSLIST::get_admin()->set_command_immediately( "toggle_toolbar" );
}


//
// スレ一覧のツールバー表示切り替え
//
void Core::slot_toggle_toolbarboard()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    SESSION::set_show_board_toolbar( ! SESSION::get_show_board_toolbar() );
    BOARD::get_admin()->set_command_immediately( "toggle_toolbar" );
}


//
// スレビューのツールバー表示切り替え
//
void Core::slot_toggle_toolbararticle()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    SESSION::set_show_article_toolbar( ! SESSION::get_show_article_toolbar() );
    ARTICLE::get_admin()->set_command_immediately( "toggle_toolbar" );
}


//
// スレ一覧のタブ表示切り替え
//
void Core::slot_toggle_tabboard()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    SESSION::set_show_board_tab( ! SESSION::get_show_board_tab() );
    BOARD::get_admin()->set_command_immediately( "toggle_tab" );
}


//
// スレビューのタブ表示切り替え
//
void Core::slot_toggle_tabarticle()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    SESSION::set_show_article_tab( ! SESSION::get_show_article_tab() );
    ARTICLE::get_admin()->set_command_immediately( "toggle_tab" );
}


//
// 2paneモード
//
void Core::slot_toggle_2pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ) return;

    pack_widget( true );
    SESSION::set_mode_pane( SESSION::MODE_2PANE );
    pack_widget( false );

    restore_focus( true, false );
}



//
// 3paneモード
//
void Core::slot_toggle_3pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_3PANE ) return;

    pack_widget( true );
    SESSION::set_mode_pane( SESSION::MODE_3PANE );
    pack_widget( false );

    restore_focus( true, false );
}


//
// 縦3paneモード
//
void Core::slot_toggle_v3pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_V3PANE ) return;

    pack_widget( true );
    SESSION::set_mode_pane( SESSION::MODE_V3PANE );
    pack_widget( false );

    restore_focus( true, false );
}


//
// messageビューをウィンドウ表示
//
void Core::slot_toggle_winmsg()
{
    pack_widget( true );
    SESSION::set_embedded_mes( false );
    pack_widget( false );

    restore_focus( true, false );
}


//
// messageビューを埋め込み表示
//
void Core::slot_toggle_embmsg()
{
    pack_widget( true );
    SESSION::set_embedded_mes( true );
    pack_widget( false );

    restore_focus( true, false );
}


//
// imageビュー表示設定
//
void Core::slot_toggle_imgview( const int mode )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    int current_mode = IMGVIEW_NO;

    if( CONFIG::get_use_image_view() ){
        if( SESSION::get_embedded_img() ) current_mode = IMGVIEW_EMB;
        else current_mode = IMGVIEW_WINDOW;
    }

    // ビュー使用切り替え
    if( current_mode == IMGVIEW_NO || current_mode == mode ){
        CONFIG::set_use_image_view( ! CONFIG::get_use_image_view() );
        if( ! CONFIG::get_use_image_view() ) IMAGE::get_admin()->set_command( "close_all_views" );
    }

    // ウィンドウ、埋め込みモード切り替え
    pack_widget( true );
    if( mode == IMGVIEW_EMB ) SESSION::set_embedded_img( true );
    else SESSION::set_embedded_img( false );
    pack_widget( false );

    SESSION::set_focused_admin( SESSION::FOCUS_NOT );
    SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NOT );
    restore_focus( true, false );
}


//
// 画像ポップアップon/off
//
void Core::slot_toggle_use_imgpopup()
{
    CONFIG::set_use_image_popup( ! CONFIG::get_use_image_popup() );
}


//
// インライン画像on/off
//
void Core::slot_toggle_use_inlineimg()
{
    CONFIG::set_use_inline_image( ! CONFIG::get_use_inline_image() );
    ARTICLE::get_admin()->set_command( "relayout_all" );
}


//
// ssspアイコン on/off
//
void Core::slot_toggle_show_ssspicon()
{
    CONFIG::set_show_sssp_icon( ! CONFIG::get_show_ssspicon() );
    ARTICLE::get_admin()->set_command( "relayout_all" );
}


//
// リスト項目(スレ一覧)の設定
//
void Core::slot_setup_boarditem_column()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARDITEM_COLUM, "" );
    pref->run();
    delete pref;
}


//
// ツールバーのアイコン(メインツールバー)の表示項目
//
void Core::slot_setup_mainitem()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_MAINITEM, "" );
    pref->run();
    delete pref;
}


//
// ツールバーのアイコン(サイドバー)の表示項目
//
void Core::slot_setup_sidebaritem()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_SIDEBARITEM, "" );
    pref->run();
    delete pref;
}


//
// ツールバーのアイコン(スレ一覧)の表示項目
//
void Core::slot_setup_boarditem()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARDITEM, "" );
    pref->run();
    delete pref;
}


//
// ツールバーのアイコン(スレビュー)の表示項目
//
void Core::slot_setup_articleitem()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLEITEM, "" );
    pref->run();
    delete pref;
}


//
// ツールバーのアイコン(書き込みビュー)の表示項目
//
void Core::slot_setup_msgitem()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_MSGITEM, "" );
    pref->run();
    delete pref;
}


//
// コンテキストメニュー(スレビュー)の表示項目
//
void Core::slot_setup_articleitem_menu()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLEITEM_MENU, "" );
    pref->run();
    delete pref;
}


//
// スレ一覧のプロパティ
//
void Core::slot_board_pref()
{
    BOARD::get_admin()->set_command( "show_current_preferences" );
}


//
// スレのプロパティ
//
void Core::slot_article_pref()
{
    ARTICLE::get_admin()->set_command( "show_current_preferences" );
}

//
// 画像のプロパティ
//
void Core::slot_image_pref()
{
    IMAGE::get_admin()->set_command( "show_current_preferences" );
}


//
// 過去ログ表示切替え
//
void Core::slot_toggle_oldarticle()
{
    CONFIG::set_show_oldarticle( ! CONFIG::get_show_oldarticle() );

    SKELETON::MsgDiag mdiag( NULL, "次に開いた板から有効になります" );
    mdiag.run();
}


//
// 起動時にviewを復元
//
void Core::slot_toggle_restore_views()
{
    bool status = CONFIG::get_restore_board() & CONFIG::get_restore_article() & CONFIG::get_restore_image();

    CONFIG::set_restore_board( ! status );
    CONFIG::set_restore_article( ! status );
    CONFIG::set_restore_image( ! status );
}


//
// 非アクティブ時に書き込みビューを折りたたむ
//
void Core::slot_toggle_fold_message()
{
    CONFIG::set_fold_message( ! CONFIG::get_fold_message() );

    SKELETON::MsgDiag mdiag( NULL, "次に書き込みビューを開いた時から有効になります" );
    mdiag.run();
}


//
// 書き込みログを保存
//
void Core::slot_toggle_save_post_log()
{
    CONFIG::set_save_post_log( ! CONFIG::get_save_post_log() );
}


//
// 書き込み履歴を保存
//
void Core::slot_toggle_save_post_history()
{
    CONFIG::set_save_post_history( ! CONFIG::get_save_post_history() );
}


//
// 画像モザイクon/off
//
void Core::slot_toggle_use_mosaic()
{
    CONFIG::set_use_mosaic( ! CONFIG::get_use_mosaic() );

    SKELETON::MsgDiag mdiag( NULL, "次に開いた画像から有効になります" );
    mdiag.run();
}


//
// まちBBSのofflawモードの切り替え
//
void Core::slot_toggle_use_machi_offlaw()
{
    CONFIG::set_use_machi_offlaw( ! CONFIG::get_use_machi_offlaw() );

    if( CONFIG::get_use_machi_offlaw() ){

        SKELETON::MsgDiag mdiag( NULL, "offlaw.cgiを使用すると以下の問題が生じるので注意して下さい。\n\n(1) リモートホストが表示されません\n\n(2) バージョン2.3.0以前のJDではofflaw.cgiで取得したログは読めません" );
        mdiag.run();
    }
}


//
// タブで開くボタンを入れ替える
//
void Core::slot_toggle_tabbutton()
{
    bool toggled = CONTROL::is_toggled_tab_button() && CONTROL::is_toggled_tab_key();

    CONTROL::toggle_tab_button( !toggled );
    CONTROL::toggle_tab_key( !toggled );
}


//
// クリックで多重ポップアップモードに移行
//
void Core::slot_toggle_popupwarpmode()
{
    CONTROL::toggle_popup_warpmode();
}


//
// マウス移動で多重ポップアップモードに移行
//
void Core::slot_shortmargin_popup()
{
    int margin = 2;
    if( CONFIG::get_margin_popup() != CONFIG::CONF_MARGIN_POPUP ) margin = CONFIG::CONF_MARGIN_POPUP;

    CONFIG::set_margin_popup( margin );
}


//
// editview を emacs風のキーバインドにする
void Core::slot_toggle_emacsmode()
{
    if( ! m_enable_menuslot ) return;
    CONTROL::toggle_emacs_mode();
}


//
// マウスジェスチャ詳細設定
//
void Core::slot_setup_mouse()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_MOUSE, "" );
    pref->run();
    delete pref;
}


//
// キーボード詳細設定
//
void Core::slot_setup_key()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_KEY, "" );
    pref->run();
    delete pref;
}


//
// マウスボタン詳細設定
//
void Core::slot_setup_button()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BUTTON, "" );
    pref->run();
    delete pref;
}


//
// メインフォント変更
//
void Core::slot_changefont_main()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname( FONT_MAIN ) );
    diag.set_title( "スレビューフォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_MAIN, diag.get_font_name() );
        ARTICLE::get_admin()->set_command( "init_font" );
        ARTICLE::get_admin()->set_command( "relayout_all" );

        CONFIG::set_fontname( FONT_MESSAGE, diag.get_font_name() );
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }
}


//
// ポップアップフォント変更
//
void Core::slot_changefont_popup()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname( FONT_POPUP ) );
    diag.set_title( "ポップアップフォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_POPUP, diag.get_font_name() );
        ARTICLE::get_admin()->set_command( "init_font" );
    }
}

 
//
// 板／スレ一覧のフォント変更
//
void Core::slot_changefont_tree()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname( FONT_BBS ) );
    diag.set_title( "板／スレ一覧フォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_BBS, diag.get_font_name() );
        BBSLIST::get_admin()->set_command( "relayout_all" );

        CONFIG::set_fontname( FONT_BOARD, diag.get_font_name() );
        BOARD::get_admin()->set_command( "relayout_all" );
    }
}


//
// スレ文字色変更
//
void Core::slot_changecolor_char()
{
    if( open_color_diag( "スレビュー文字色", COLOR_CHAR ) ){

        ARTICLE::get_admin()->set_command( "relayout_all" );

        CONFIG::set_color( COLOR_CHAR_MESSAGE, CONFIG::get_color( COLOR_CHAR ) );
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }
}


//
// スレ、ポップアップ背景色変更
//
void Core::slot_changecolor_back()
{
    if( open_color_diag( "スレビュー背景色", COLOR_BACK ) ){

        CONFIG::set_color( COLOR_BACK_POPUP, CONFIG::get_color( COLOR_BACK) );

        CONFIG::set_color( COLOR_BACK_MESSAGE, CONFIG::get_color( COLOR_BACK) );
        MESSAGE::get_admin()->set_command( "relayout_all" );

        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}


//
// 板／スレ一覧文字色変更
//
void Core::slot_changecolor_char_tree()
{
    if( open_color_diag( "板／スレ一覧文字色", COLOR_CHAR_BBS ) ){

        CONFIG::set_color( COLOR_CHAR_BBS_COMMENT, CONFIG::get_color( COLOR_CHAR_BBS ) );
        CONFIG::set_color( COLOR_CHAR_BOARD, CONFIG::get_color( COLOR_CHAR_BBS ) );

        BBSLIST::get_admin()->set_command( "relayout_all" );
        BOARD::get_admin()->set_command( "relayout_all" );
    }
}



//
// 板／スレ一覧背景色変更
//
void Core::slot_changecolor_back_tree()
{
    if( open_color_diag( "板／スレ一覧背景色", COLOR_BACK_BBS ) ){

        CONFIG::set_color( COLOR_BACK_BBS_EVEN, CONFIG::get_color( COLOR_BACK_BBS ) );

        CONFIG::set_color( COLOR_BACK_BOARD, CONFIG::get_color( COLOR_BACK_BBS ) );
        CONFIG::set_color( COLOR_BACK_BOARD_EVEN, CONFIG::get_color( COLOR_BACK_BBS ) );

        BBSLIST::get_admin()->set_command( "relayout_all" );
        BOARD::get_admin()->set_command( "relayout_all" );
    }
}


//
// フォントと色の詳細設定
//
void Core::slot_setup_fontcolor()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_FONTCOLOR, "" );
    pref->run();
    delete pref;
}


//
// プロキシ設定
//
void Core::slot_setup_proxy()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_PROXY, "" );
    pref->run();
    delete pref;
}


//
// ブラウザ設定
//
void Core::slot_setup_browser()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BROWSER, URL_BROWSER );
    pref->run();
    delete pref;
}


//
// パスワード設定
//
void Core::slot_setup_passwd()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_PASSWD, "" );
    pref->run();
    delete pref;
}


//
// IPv6使用
//
void Core::slot_toggle_ipv6()
{
    CONFIG::set_use_ipv6( ! CONFIG::get_use_ipv6() );
}


//
// あぼーん設定
//
void Core::slot_setup_abone()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_GLOBALABONE, "" );
    pref->run();
    delete pref;
}


//
// スレあぼーん設定
//
void Core::slot_setup_abone_thread()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_GLOBALABONETHREAD, "" );
    pref->run();
    delete pref;
}


//
// 透明/連鎖あぼーん切り替え
//
void Core::slot_toggle_abone_transp_chain()
{
    bool status = CONFIG::get_abone_chain() & CONFIG::get_abone_transparent();

    CONFIG::set_abone_transparent( ! status );
    CONFIG::set_abone_chain( ! status );

    // あぼーん情報更新
    DBTREE::update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


// 実況設定
void Core::slot_setup_live()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_LIVE, "" );
    pref->run();
    delete pref;
}


//
// ユーザコマンドの編集
//
void Core::slot_usrcmd_pref()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_USRCMD, URL_USRCMD );
    pref->run();
    delete pref;
}


//
// リンクフィルタの編集
//
void Core::slot_filter_pref()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_LINKFILTER, URL_LINKFILTER );
    pref->run();
    delete pref;
}


//
// about:config
//
void Core::slot_aboutconfig()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ABOUTCONFIG, URL_ABOUTCONFIG );
    pref->run();
    delete pref;
}



// プライバシー情報のクリア
void Core::slot_clear_privacy()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_PRIVACY, URL_PRIVACY );
    pref->run();
    delete pref;
}


//
// 書き込みログのクリア
//
void Core::slot_clear_post_log()
{
    SKELETON::MsgDiag mdiag( NULL, "書き込みログを削除しますか？",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    MESSAGE::get_log_manager()->clear_post_log();

    // ログ表示を閉じる
    ARTICLE::get_admin()->set_command( "close_view", "postlog" + std::string( POSTLOG_SIGN ), "closeall"  );
}


//
// 全スレの書き込み履歴(鉛筆マーク)のクリア
//
void Core::slot_clear_post_history()
{
    SKELETON::MsgDiag mdiag( NULL, "全スレの書き込み履歴を削除します。\n\nある板、または特定のスレの履歴を削除するには板またはスレのプロパティから行って下さい。\n\nまた全スレの書き込み履歴の削除には時間がかかります。\n\n全スレの書き込み履歴の削除を実行しますか？",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    DBTREE::clear_all_post_history();

    // ビューの表示更新
    CORE::core_set_command( "redraw_article" );

    std::list< std::string > list_urls = BOARD::get_admin()->get_URLs();
    std::list< std::string >::iterator it = list_urls.begin();
    for( ; it != list_urls.end(); ++it ) CORE::core_set_command( "update_board", *it );
}


//
// 画像キャッシュクリア
//
void Core::slot_delete_all_images()
{
    DBIMG::delete_all_files();
    IMAGE::get_admin()->set_command( "close_uncached_views" );
}


//
// 実況
//
void Core::slot_live_start_stop()
{
    std::string url = ARTICLE::get_admin()->get_current_url();
    ARTICLE::get_admin()->set_command( "live_start_stop", url );
}


//
// 現在開いている板のキャッシュ内のログ検索
//
void Core::slot_search_cache_board()
{
    std::string url = BOARD::get_admin()->get_current_url();
    if( ! url.empty() ) CORE::core_set_command( "open_article_searchlog", url );
}


//
// キャッシュ内のログ検索
//
void Core::slot_search_cache()
{
    CORE::core_set_command( "open_article_searchalllog" );
}


//
// スレタイ検索
//
void Core::slot_search_title()
{
    CORE::core_set_command( "open_article_searchtitle" );
}


//
// サイドバーの全更新チェック
//
void Core::slot_check_update_root()
{
    CORE::core_set_command( "check_update_root", "" );
}


//
// サイドバーを全更新チェックしてタブで開く
//
void Core::slot_check_update_open_root()
{
    CORE::core_set_command( "check_update_open_root", "" );
}


//
// 更新チェックをキャンセル
//
void Core::slot_cancel_check_update()
{
    CORE::core_set_command( "cancel_check_update", "" );
}


//
// お気に入りの編集
//
void Core::slot_edit_favorite()
{
    CORE::core_set_command( "edit_favorite","" );
}


//
// 書き込みログ
//
void Core::slot_show_postlog()
{
    CORE::core_set_command( "open_article_postlog" );
}


//
// 開いている板にdatをインポート
//
void Core::slot_import_dat()
{
    std::string url_board = BOARD::get_admin()->get_current_url();

#ifdef _DEBUG
    std::cout << "Core::slot_import_dat url = " << url_board << std::endl;
#endif

    if( ! url_board.empty() ) CORE::core_set_command( "import_dat", url_board, "show_diag" );
}


//
// サポートBBS
//
void Core::slot_show_bbs()
{
    CORE::core_set_command( "open_board" , DBTREE::url_subject( ENVIRONMENT::get_jdbbs() ), "true" );
}


//
// 過去ログ
//
void Core::slot_show_old2ch()
{
    CORE::core_set_command( "open_board" , DBTREE::url_subject( ENVIRONMENT::get_jd2chlog() ), "true" );
}


//
// マニュアル
//
void Core::slot_show_manual()
{
    open_by_browser( ENVIRONMENT::get_jdhelp() );
}


//
// about
//
void Core::slot_show_about()
{
    SKELETON::AboutDiag about( "JDについて" );
    about.run();
}
