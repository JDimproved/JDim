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
#include "loginacorn.h"
#include "winmain.h"
#include "fontid.h"
#include "colorid.h"
#include "global.h"
#include "sharedbuffer.h"

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
// URLを開く
//
void Core::slot_openurl()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_OPENURL, "" );
    pref->run();
}


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


/**
 * @brief どんぐりシステム メールアドレス登録警備員にログイン
 */
void Core::slot_toggle_loginacorn()
{
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_loginacorn\n";
#endif

    // ログイン中ならログアウト
    if( CORE::get_loginacorn()->login_now() ) CORE::get_loginacorn()->logout();

    // ログオフ中ならログイン開始
    else CORE::get_loginacorn()->start_login();

    set_maintitle();
}


//
// 板リスト再読込
//
void Core::slot_reload_list()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( nullptr, "オフラインです" );
        mdiag.run();
        return;
    }

    DBTREE::download_bbsmenu();
    CORE::core_set_command( "set_status","", "板一覧再読み込み中...." );        
}


//
// 終了
//
void Core::slot_quit()
{
    m_win_main.hide();
}


//
// スレ一覧のsinceの表示モード
void Core::slot_toggle_since( const int mode )
{
    if( ! m_enable_menuslot ) return;
    if( SESSION::get_col_since_time() == mode ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_since mode = " << mode << std::endl;
#endif

    SESSION::set_col_since_time( mode );
    DBTREE::reset_all_since_date();
    BOARD::get_admin()->set_command( "relayout_all" );
}


//
// スレ一覧の最終書込の表示モード
void Core::slot_toggle_write( const int mode )
{
    if( ! m_enable_menuslot ) return;
    if( SESSION::get_col_write_time() == mode ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_write mode = " << mode << std::endl;
#endif

    SESSION::set_col_write_time( mode );
    DBTREE::reset_all_write_date();
    BOARD::get_admin()->set_command( "relayout_all" );
}


//
// スレ一覧の最終取得の表示モード
void Core::slot_toggle_access( const int mode )
{
    if( ! m_enable_menuslot ) return;
    if( SESSION::get_col_access_time() == mode ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_access mode = " << mode << std::endl;
#endif

    SESSION::set_col_access_time( mode );
    DBTREE::reset_all_access_date();
    BOARD::get_admin()->set_command( "relayout_all" );
}


//
// メインツールバー表示切り替え
//
void Core::slot_toggle_toolbarmain()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    pack_widget( true );
    SESSION::set_show_main_toolbar( ! SESSION::get_show_main_toolbar() );
    pack_widget( false );

    restore_focus( true, false );

}


//
// メインツールバーの表示位置
//
void Core::slot_toggle_toolbarpos( const int pos )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::slot_toggle_toolbarpos pos = " << pos << " / " << SESSION::get_toolbar_pos() << std::endl;
#endif

    if( ! SESSION::get_show_main_toolbar() ) SESSION::set_toolbar_pos( pos );
    else{

        pack_widget( true );
        SESSION::set_toolbar_pos( pos );
        pack_widget( false );

        restore_focus( true, false );
    }
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
// フルスクリーン
//
void Core::slot_toggle_fullscreen()
{
    if( ! m_enable_menuslot ) return;

    if( SESSION::is_full_win_main() ) m_win_main.unfullscreen();
    else m_win_main.fullscreen();
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
// messgeビューのwrap切り替え
//
void Core::slot_toggle_msg_wrap()
{
    CORE::core_set_command( "toggle_message_wrap", "" );
}


//
// imageビュー表示設定
//
void Core::slot_toggle_imgview( const int mode )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    // imageビューの状態
    int current_mode = IMGVIEW_NO;

    if( CONFIG::get_use_image_view() ){
        if( SESSION::get_embedded_img() ) current_mode = IMGVIEW_EMB;
        else current_mode = IMGVIEW_WINDOW;
    }

    if( current_mode == mode ) return;

    // imageビュー使用切り替え
    if( mode == IMGVIEW_NO ){
        CONFIG::set_use_image_view( false );
        IMAGE::get_admin()->set_command( "close_all_views" );
    }
    else {
        CONFIG::set_use_image_view( true );
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
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BOARDITEM_COLUM, "" );
    pref->run();
}


//
// ツールバーのアイコン(メインツールバー)の表示項目
//
void Core::slot_setup_mainitem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_MAINITEM, "" );
    pref->run();
}


//
// ツールバーのアイコン(サイドバー)の表示項目
//
void Core::slot_setup_sidebaritem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_SIDEBARITEM, "" );
    pref->run();
}


//
// ツールバーのアイコン(スレ一覧)の表示項目
//
void Core::slot_setup_boarditem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BOARDITEM, "" );
    pref->run();
}


//
// ツールバーのアイコン(スレビュー)の表示項目
//
void Core::slot_setup_articleitem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_ARTICLEITEM, "" );
    pref->run();
}


//
// ツールバーのアイコン(ログ/スレタイ検索)の表示項目
//
void Core::slot_setup_searchitem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_SEARCHITEM, "" );
    pref->run();
}


//
// ツールバーのアイコン(書き込みビュー)の表示項目
//
void Core::slot_setup_msgitem()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_MSGITEM, "" );
    pref->run();
}



//
// コンテキストメニュー(スレ一覧)の表示項目
//
void Core::slot_setup_boarditem_menu()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BOARDITEM_MENU, "" );
    pref->run();
}

//
// コンテキストメニュー(スレビュー)の表示項目
//
void Core::slot_setup_articleitem_menu()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_ARTICLEITEM_MENU, "" );
    pref->run();
}


//
// 板一覧のプロパティ
//
void Core::slot_bbslist_pref()
{
    BBSLIST::get_admin()->set_command( "show_preferences", URL_BBSLISTVIEW );
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
// 起動時にviewを復元
//
void Core::slot_toggle_restore_views()
{
    const bool status{ CONFIG::get_restore_board() && CONFIG::get_restore_article() && CONFIG::get_restore_image() };

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

    SKELETON::MsgDiag mdiag( nullptr, "次に書き込みビューを開いた時から有効になります" );
    mdiag.run();
}


//
// ツリービューの選択を表示中のビューと同期する
//
void Core::slot_toggle_select_item_sync()
{
    if( CONFIG::get_select_item_sync() != 0 )
        CONFIG::set_select_item_sync( 0 ); // 同期しない
    else
        CONFIG::set_select_item_sync( 1 ); // 同期する
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

    SKELETON::MsgDiag mdiag( nullptr, "次に開いた画像から有効になります" );
    mdiag.run();
}


/**
 * @brief まちBBSのスレ表示でホスト情報とIDを切り替え
 */
void Core::slot_toggle_show_machi_id()
{
    CONFIG::set_show_machi_id( ! CONFIG::get_show_machi_id() );

    // ノードツリーはホスト情報またはIDのどちらかしか保持していない
    // そのため全ArticleViewのノードツリーを再構築してレイアウトし直す
    CORE::core_set_command( "relayout_all_article", "", "completely" );
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
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_MOUSE, "" );
    pref->run();
}


//
// キーボード詳細設定
//
void Core::slot_setup_key()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_KEY, "" );
    pref->run();
}


//
// マウスボタン詳細設定
//
void Core::slot_setup_button()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BUTTON, "" );
    pref->run();
}


//
// メインフォント変更
//
void Core::slot_changefont_main()
{
    Gtk::FontChooserDialog diag;
    diag.set_font( CONFIG::get_fontname( FONT_MAIN ) );
    diag.set_title( "スレビューフォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_MAIN, diag.get_font() );
        ARTICLE::get_admin()->set_command( "init_font" );
        ARTICLE::get_admin()->set_command( "relayout_all" );

        CONFIG::set_fontname( FONT_MESSAGE, diag.get_font() );
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }
}

//
// メールのフォント変更
//
void Core::slot_changefont_mail()
{
    Gtk::FontChooserDialog diag;
    diag.set_font( CONFIG::get_fontname( FONT_MAIL ) );
    diag.set_title( "メール欄のフォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_MAIL, diag.get_font() );
        ARTICLE::get_admin()->set_command( "init_font" );
        ARTICLE::get_admin()->set_command( "relayout_all" );

        CONFIG::set_fontname( FONT_MESSAGE, diag.get_font() );
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }
}

//
// ポップアップフォント変更
//
void Core::slot_changefont_popup()
{
    Gtk::FontChooserDialog diag;
    diag.set_font( CONFIG::get_fontname( FONT_POPUP ) );
    diag.set_title( "ポップアップフォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_POPUP, diag.get_font() );
        ARTICLE::get_admin()->set_command( "init_font" );
    }
}

 
//
// 板／スレ一覧のフォント変更
//
void Core::slot_changefont_tree()
{
    Gtk::FontChooserDialog diag;
    diag.set_font( CONFIG::get_fontname( FONT_BBS ) );
    diag.set_title( "板／スレ一覧フォント" );
    diag.set_transient_for( *CORE::get_mainwindow() );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_fontname( FONT_BBS, diag.get_font() );
        BBSLIST::get_admin()->set_command( "relayout_all" );

        CONFIG::set_fontname( FONT_BOARD, diag.get_font() );
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
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_FONTCOLOR, "" );
    pref->run();
}


//
// プロキシ設定
//
void Core::slot_setup_proxy()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_PROXY, "" );
    pref->run();
}


//
// ブラウザ設定
//
void Core::slot_setup_browser()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BROWSER, URL_BROWSER );
    pref->run();
}


//
// パスワード設定
//
void Core::slot_setup_passwd()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_PASSWD, "" );
    pref->run();
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
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_GLOBALABONE, "" );
    pref->run();
}


//
// スレあぼーん設定
//
void Core::slot_setup_abone_thread()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_GLOBALABONETHREAD, "" );
    pref->run();
}


//
// 透明/連鎖あぼーん切り替え
//
void Core::slot_toggle_abone_transp_chain()
{
    const bool status{ CONFIG::get_abone_chain() && CONFIG::get_abone_transparent() };

    CONFIG::set_abone_transparent( ! status );
    CONFIG::set_abone_chain( ! status );

    // あぼーん情報更新
    DBTREE::update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


//
// NG正規表現によるあぼーん時に大小と全半角文字の違いを無視する
//
void Core::slot_toggle_abone_icase_wchar()
{
    const bool status{ CONFIG::get_abone_icase() && CONFIG::get_abone_wchar() };

    CONFIG::set_abone_icase( ! status );
    CONFIG::set_abone_wchar( ! status );

    // あぼーん情報更新
    DBTREE::update_abone_thread();

    DBTREE::update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


/**
 * @brief (実験的な機能) あぼーんしたレスの判定理由の表示を切り替え
 */
void Core::slot_toggle_show_abone_reason()
{
    CONFIG::set_show_abone_reason( ! CONFIG::get_show_abone_reason() );

    CORE::core_set_command( "relayout_all_article" );
}


// 実況設定
void Core::slot_setup_live()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_LIVE, "" );
    pref->run();
}


//
// ユーザコマンドの編集
//
void Core::slot_usrcmd_pref()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_USRCMD, URL_USRCMD );
    pref->run();
}


//
// リンクフィルタの編集
//
void Core::slot_filter_pref()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_LINKFILTER, URL_LINKFILTER );
    pref->run();
}


//
// 置換文字列の編集
//
void Core::slot_replace_pref()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_REPLACESTR, URL_REPLACESTR );
    pref->run();
}


//
// about:config
//
void Core::slot_aboutconfig()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_ABOUTCONFIG, URL_ABOUTCONFIG );
    pref->run();
}



// プライバシー情報のクリア
void Core::slot_clear_privacy()
{
    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_PRIVACY, URL_PRIVACY );
    pref->run();
}


//
// 書き込みログのクリア
//
void Core::slot_clear_post_log()
{
    SKELETON::MsgDiag mdiag( nullptr, "書き込みログを削除しますか？",
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
    SKELETON::MsgDiag mdiag( nullptr, "全スレの書き込み履歴を削除します。\n\nある板、または特定のスレの履歴を削除するには板またはスレのプロパティから行って下さい。\n\nまた全スレの書き込み履歴の削除には時間がかかります。\n\n全スレの書き込み履歴の削除を実行しますか？",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    DBTREE::clear_all_post_history();

    // ビューの表示更新
    CORE::core_set_command( "redraw_article" );

    const std::vector<std::string>& list_urls = BOARD::get_admin()->get_URLs();
    for( const std::string& url : list_urls ) CORE::core_set_command( "update_board", url );
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
    const std::string url = BOARD::get_admin()->get_current_url();
    if( ! url.empty() ) CORE::core_set_command( "open_article_searchlog", url, "", "noexec" );
}


//
// キャッシュ内のログ検索
//
void Core::slot_search_cache()
{
    CORE::core_set_command( "open_article_searchlog", URL_SEARCH_ALLBOARD, "", "noexec" );
}


//
// 現在開いている板のキャッシュ内のログ一覧表示
//
void Core::slot_show_cache_board()
{
    const std::string url = DBTREE::url_boardbase( BOARD::get_admin()->get_current_url() );
    if( ! url.empty() ) CORE::core_set_command( "open_board_showlog", url );
}


//
// キャッシュ内の全ログ一覧表示
//
void Core::slot_show_cache()
{
    CORE::core_set_command( "open_board_showalllog" );
}


//
// スレタイ検索
//
void Core::slot_search_title()
{
    CORE::core_set_command( "open_article_searchtitle", "", "", "noexec" );
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
// サイドバーをスレ一覧に表示
//
void Core::slot_show_sidebarboard()
{
    if( SESSION::get_sidebar_current_url() != URL_BBSLISTVIEW
        && SESSION::get_sidebar_current_url() != URL_HISTBOARDVIEW
        ){

        const std::string tab = "newtab";
        const std::string mode = "";
        CORE::core_set_command( "open_sidebar_board", SESSION::get_sidebar_current_url(), tab, mode, "0" );
    }
}


//
// サイドバーの仮想板を作成
//
void Core::slot_create_vboard()
{
    if( SESSION::get_sidebar_current_url() != URL_BBSLISTVIEW
        && SESSION::get_sidebar_current_url() != URL_HISTBOARDVIEW
        ){

        CORE::DATA_INFO_LIST list_info;
        CORE::DATA_INFO info;
        info.type = TYPE_VBOARD;
        info.parent = CORE::get_mainwindow();
        info.url = SESSION::get_sidebar_current_url() + SIDEBAR_SIGN + "0";
        info.name = SESSION::get_sidebar_dirname( SESSION::get_sidebar_current_url(), 0 );
        info.path = Gtk::TreePath( "0" ).to_string();
        list_info.push_back( info );
        CORE::SBUF_set_list( list_info );

        CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
    }
}


//
// サポートBBS
//
void Core::slot_show_bbs()
{
    CORE::core_set_command( "open_board", DBTREE::url_boardbase( ENVIRONMENT::get_jdbbs() ), "newtab" );
}


//
// 過去ログ
//
void Core::slot_show_old2ch()
{
    CORE::core_set_command( "open_board", DBTREE::url_boardbase( ENVIRONMENT::get_jd2chlog() ), "newtab" );
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
    SKELETON::AboutDiag about( "JDimについて" );
    about.run();
}
