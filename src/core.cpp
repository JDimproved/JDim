// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "core.h"
#include "command.h"
#include "winmain.h"
#include "session.h"
#include "global.h"
#include "dndmanager.h"
#include "usrcmdmanager.h"
#include "searchmanager.h"
#include "historymenu.h"
#include "login2ch.h"
#include "prefdiagfactory.h"
#include "controlutil.h"
#include "controlid.h"
#include "colorid.h"
#include "jdversion.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"
#include "config/keyconfig.h"
#include "config/mouseconfig.h"
#include "config/buttonconfig.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "dbtree/interface.h"
#include "dbimg/imginterface.h"

#include "bbslist/bbslistadmin.h"
#include "board/boardadmin.h"
#include "article/articleadmin.h"
#include "image/imageadmin.h"
#include "message/messageadmin.h"

#include <sstream>

using namespace CORE;

enum
{
    PAGE_ARTICLE = 0,
    PAGE_IMAGE,
    PAGE_BOARD
};


Core* instance_core;


Core* CORE::get_instance()
{
    return instance_core;
}


// 全ビューをフォーカスアウト
#define FOCUS_OUT_ALL() do{ \
ARTICLE::get_admin()->set_command( "focus_out" ); \
BOARD::get_admin()->set_command( "focus_out" ); \
BBSLIST::get_admin()->set_command( "focus_out" ); \
IMAGE::get_admin()->set_command( "focus_out" ); \
}while(0)


//////////////////////////////////////////////////////


Core::Core( WinMain& win_main )
    : m_win_main( win_main ),
      m_imagetab_shown( 0 ),
      m_button_go( Gtk::Stock::JUMP_TO ),
      m_button_search_cache( Gtk::Stock::FIND ),
      m_button_bbslist( ICON::DIR ),
      m_button_favorite( ICON::FAVORITE ),
      m_button_board( ICON::BOARD ),
      m_button_thread( ICON::THREAD ),
      m_button_image( ICON::IMAGE ),
      m_enable_menuslot( true ),
      m_boot( true ),
      m_init( false )
{
    instance_core = this;
    m_disp.connect( sigc::mem_fun( *this, &Core::exec_command ) );

    m_win_main.signal_window_state_event().connect( sigc::mem_fun( *this, &Core::slot_window_state_event ) );

    // データベースのルート作成
    DBTREE::create_root();
    DBIMG::create_root();

    // ログインマネージャ作成
    LOGIN::get_login2ch();

    // 各管理クラス作成
    BBSLIST::get_admin();
    BOARD::get_admin();
    ARTICLE::get_admin();
    IMAGE::get_admin();
    MESSAGE::get_admin();

    // D&Dマネージャ作成
    CORE::get_dnd_manager();

    // ユーザコマンドマネージャ作成
    CORE::get_usrcmd_manager();

    // ログ検索マネージャ作成
    CORE::get_search_manager();
}


Core::~Core()
{
#ifdef _DEBUG
    std::cout << "Core::~Core\n";
#endif

    // 設定保存
    // セッション情報は WinMain::~WinMain() で保存する
    CONFIG::save_conf();

    // PANEの敷居の位置保存
    SESSION::set_hpane_main_pos( m_hpaned.get_position() );
    SESSION::set_vpane_main_pos( m_vpaned_r.get_position() );
    SESSION::set_hpane_main_r_pos( m_hpaned_r.get_position() );
    SESSION::set_vpane_main_mes_pos( m_vpaned_message.get_position() );

    // ログ検索マネージャ削除
    CORE::delete_search_manager();

    // ユーザコマンドマネージャ削除
    CORE::delete_usrcmd_manager();

    // D&Dマネージャ削除
    CORE::delete_dnd_manager();

    // マウス、キーコンフィグ削除
    CONFIG::delete_keyconfig();
    CONFIG::delete_mouseconfig();
    CONFIG::delete_buttonconfig();

    // ビューを削除する前にswitch_pageをdisconnectしておかないとエラーが出る
    if( m_sigc_switch_page.connected() ) m_sigc_switch_page.disconnect(); 

    // 各管理クラスを削除
    BBSLIST::delete_admin();
    BOARD::delete_admin();
    ARTICLE::delete_admin();
    IMAGE::delete_admin();
    MESSAGE::delete_admin();

    // ログインマネージャ削除
    LOGIN::delete_login2ch();

    // データベース削除
    DBTREE::delete_root();
    DBIMG::delete_root();
}



Gtk::Widget* Core::get_toplevel()
{
    return m_win_main.get_toplevel();
}


//
// 実行
//
// init = true なら初回起動
//
void Core::run( bool init )
{
    // メインメニューの設定
    m_action_group = Gtk::ActionGroup::create();

    // File
    m_action_group->add( Gtk::Action::create( "Menu_File", "ファイル(_F)" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "Online", "オンライン", std::string(), SESSION::is_online() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_online ) );
    m_action_group->add( Gtk::Action::create( "ReloadList", "板一覧再読込"), sigc::mem_fun( *this, &Core::slot_reload_list ) );
    m_action_group->add( Gtk::Action::create( "SearchCache", "キャッシュ内ログ検索"), sigc::mem_fun( *this, &Core::slot_search_cache ) );
    m_action_group->add( Gtk::Action::create( "SaveFavorite", "お気に入り保存"), sigc::mem_fun( *this, &Core::slot_save_favorite ) );
    m_action_group->add( Gtk::Action::create( "Quit", "終了" ), sigc::mem_fun(*this, &Core::slot_quit ) );


    // ログイン
    m_action_group->add( Gtk::Action::create( "Menu_Login", "ログイン(_L)" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "Login2ch", "2chにログイン", std::string(), false ),
                        sigc::mem_fun( *this, &Core::slot_toggle_login2ch ) );
    m_action_group->add( Gtk::Action::create( "SetupPasswd", "設定" ), sigc::mem_fun( *this, &Core::slot_setup_passwd ) );


    // 表示
    m_action_group->add( Gtk::Action::create( "Menu_View", "表示(_V)" ) );    
    m_action_group->add( Gtk::Action::create( "Show_Board", "スレ一覧に切替" ), sigc::mem_fun(*this, &Core::switch_board ) );
    m_action_group->add( Gtk::Action::create( "Show_Thread", "スレビューに切替" ), sigc::mem_fun(*this, &Core::switch_article ) );
    m_action_group->add( Gtk::Action::create( "Show_Image", "画像ビューに切替" ), sigc::mem_fun(*this, &Core::switch_image ) );

    m_action_group->add( Gtk::Action::create( "Toolbar_Menu", "ツールバー" ) );
    m_action_group->add( Gtk::ToggleAction::create( "Toolbar", "表示", std::string(), SESSION::show_toolbar() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbar ) );

    m_action_group->add( Gtk::Action::create( "Sidebar_Menu", "サイドバー" ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_BBS", "板一覧", std::string(), SESSION::show_sidebar() ),
                         sigc::mem_fun( *this, &Core::switch_bbslist ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_FAVORITE", "お気に入り", std::string(), SESSION::show_sidebar() ),
                         sigc::mem_fun( *this, &Core::switch_favorite ) );

    // pane 設定
    Gtk::RadioButtonGroup radiogroup;
    Glib::RefPtr< Gtk::RadioAction > raction0 = Gtk::RadioAction::create( radiogroup, "2Pane", "2pane" );
    Glib::RefPtr< Gtk::RadioAction > raction1 = Gtk::RadioAction::create( radiogroup, "3Pane", "3pane" );
    Glib::RefPtr< Gtk::RadioAction > raction2 = Gtk::RadioAction::create( radiogroup, "v3Pane", "縦3pane" );

    switch( SESSION::get_mode_pane() ){
        case SESSION::MODE_2PANE: raction0->set_active( true ); break;
        case SESSION::MODE_3PANE: raction1->set_active( true ); break;
        case SESSION::MODE_V3PANE: raction2->set_active( true ); break;
    }

    m_action_group->add( raction0, sigc::mem_fun( *this, &Core::slot_toggle_2pane ) );
    m_action_group->add( raction1, sigc::mem_fun( *this, &Core::slot_toggle_3pane ) );
    m_action_group->add( raction2, sigc::mem_fun( *this, &Core::slot_toggle_v3pane ) );

    // 埋め込みmessage
    m_action_group->add( Gtk::ToggleAction::create( "EmbMes", "書き込みビューを埋め込み表示", std::string(), SESSION::get_embedded_mes() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_embedded_mes ) );

    // 埋め込みImage
    m_action_group->add( Gtk::ToggleAction::create( "EmbImg", "画像ビューを埋め込み表示", std::string(), SESSION::get_embedded_img() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_embedded_img ) );

    // ツールバー位置
    Gtk::RadioButtonGroup rg_toolbar;
    raction0 = Gtk::RadioAction::create( rg_toolbar, "ToolbarPos0", "メニューバーの下に表示する" );
    raction1 = Gtk::RadioAction::create( rg_toolbar, "ToolbarPos1", "サイドバーの右に表示する" );

    switch( SESSION::toolbar_pos() ){
        case SESSION::TOOLBAR_NORMAL: raction0->set_active( true ); break;
        case SESSION::TOOLBAR_RIGHT: raction1->set_active( true ); break;
    }

    m_action_group->add( raction0, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_NORMAL ) );
    m_action_group->add( raction1, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_RIGHT ) );

    // 設定
    m_action_group->add( Gtk::Action::create( "Menu_Config", "設定(_C)" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "OldArticle", "スレ一覧に過去ログも表示", std::string(), CONFIG::get_show_oldarticle() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_oldarticle ) );

    m_action_group->add( Gtk::ToggleAction::create( "RestoreViews", "起動時に開いていたビューを復元", std::string(),
                                                    ( CONFIG::get_restore_board()
                                                      & CONFIG::get_restore_board()
                                                      & CONFIG::get_restore_board() ) ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_views ) );

    m_action_group->add( Gtk::ToggleAction::create( "SavePostLog", "書き込みログを保存(暫定仕様)", std::string(), CONFIG::get_save_postlog() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_save_postlog ) );


    m_action_group->add( Gtk::Action::create( "Mouse_Menu", "マウス設定" ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleTab", "シングルクリックでタブを開く", std::string(),
                                                    ! CONFIG::get_buttonconfig()->tab_midbutton()  ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabbutton ) );

    m_action_group->add( Gtk::ToggleAction::create( "TogglePopupWarp", "シングルクリックで多重ポップアップモードに移行する", std::string(),
                                                    CONFIG::get_buttonconfig()->is_popup_warpmode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_popupwarpmode ) );


    m_action_group->add( Gtk::Action::create( "Key_Menu", "キーボード設定" ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleEmacsMode", "書き込みビューをEmacs風のキーバインドにする", std::string(),
                                                    CONFIG::get_keyconfig()->is_emacs_mode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_emacsmode ) );

    m_action_group->add( Gtk::Action::create( "FontColorPref", "フォントと色の設定" ), sigc::mem_fun( *this, &Core::slot_setup_fontcolor ) );


    m_action_group->add( Gtk::Action::create( "SetupProxy", "プロキシ" ), sigc::mem_fun( *this, &Core::slot_setup_proxy ) );
    m_action_group->add( Gtk::Action::create( "SetupBrowser", "Webブラウザ" ), sigc::mem_fun( *this, &Core::slot_setup_browser ) );

    m_action_group->add( Gtk::Action::create( "SetupAbone", "全体あぼ〜ん" ), sigc::mem_fun( *this, &Core::slot_setup_abone ) );
    m_action_group->add( Gtk::Action::create( "SetupAboneThread", "全体スレあぼ〜ん" ), sigc::mem_fun( *this, &Core::slot_setup_abone_thread ) );

    m_action_group->add( Gtk::ToggleAction::create( "TranspChainAbone", "デフォルトで透明/連鎖あぼ〜ん", std::string(),
                                                    ( CONFIG::get_abone_transparent() && CONFIG::get_abone_chain() ) ),
                                                    sigc::mem_fun( *this, &Core::slot_toggle_abone_transp_chain ) );

    m_action_group->add( Gtk::ToggleAction::create( "UseMosaic", "画像にモザイクをかける", std::string(), CONFIG::get_use_mosaic() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_mosaic ) );
    m_action_group->add( Gtk::Action::create( "DeleteImages", "画像キャッシュクリア" ), sigc::mem_fun( *this, &Core::slot_delete_all_images ) ); 

    // help
    m_action_group->add( Gtk::Action::create( "Menu_Help", "ヘルプ(_H)" ) );    
    m_action_group->add( Gtk::Action::create( "Hp", "ホームページ" ), sigc::mem_fun( *this, &Core::slot_show_hp ) );
    m_action_group->add( Gtk::Action::create( "Bbs", "サポート掲示板" ), sigc::mem_fun( *this, &Core::slot_show_bbs ) );
    m_action_group->add( Gtk::Action::create( "OldLog", "2chスレ過去ログ" ), sigc::mem_fun( *this, &Core::slot_show_old2ch ) );
    m_action_group->add( Gtk::Action::create( "Manual", "オンラインマニュアル" ), sigc::mem_fun( *this, &Core::slot_show_manual ) );
    m_action_group->add( Gtk::Action::create( "About", "JDについて" ), sigc::mem_fun( *this, &Core::slot_show_about ) );
    

    m_ui_manager = Gtk::UIManager::create();    
    m_ui_manager->insert_action_group( m_action_group );

    Glib::ustring str_ui = 
        "<ui>"
        "<menubar name='menu_bar'>"

    // ファイル
        "<menu action='Menu_File'>"
        "<menuitem action='Online'/>"
        "<separator/>"
        "<menuitem action='SearchCache'/>"    
        "<separator/>"
        "<menuitem action='SaveFavorite'/>"
        "<separator/>"
        "<menuitem action='ReloadList'/>"
        "<separator/>"
        "<menuitem action='Quit'/>"
        "</menu>"

    // ログイン
        "<menu action='Menu_Login'>"
        "<menuitem action='Login2ch'/>"
        "<menuitem action='SetupPasswd'/>"
        "</menu>"

    // 表示
        "<menu action='Menu_View'>"
        "<menuitem action='Show_Board'/>"
        "<menuitem action='Show_Thread'/>"
        "<menuitem action='Show_Image'/>"
        "<separator/>"
        "<menu action='Toolbar_Menu'>"
        "<menuitem action='Toolbar'/>"
        "<separator/>"
        "<menuitem action='ToolbarPos0'/>"
        "<menuitem action='ToolbarPos1'/>"
        "</menu>"
        "<menu action='Sidebar_Menu'>"
        "<menuitem action='Show_BBS'/>"
        "<menuitem action='Show_FAVORITE'/>"
        "</menu>"
        "<separator/>"
        "<menuitem action='2Pane'/>"
        "<menuitem action='3Pane'/>"
        "<menuitem action='v3Pane'/>"
        "<separator/>"
        "<menuitem action='EmbMes'/>"
        "<menuitem action='EmbImg'/>"
        "</menu>"

    // 設定
        "<menu action='Menu_Config'>"
        "<menuitem action='OldArticle'/>"
        "<menuitem action='RestoreViews'/>"
        "<menuitem action='SavePostLog'/>"
        "<separator/>"

        "<menu action='Mouse_Menu'>"
        "<menuitem action='ToggleTab'/>"
        "<menuitem action='TogglePopupWarp'/>"
        "</menu>"

        "<separator/>"

        "<menu action='Key_Menu'>"
        "<menuitem action='ToggleEmacsMode'/>"
        "</menu>"

        "<separator/>"

        "<menuitem action='FontColorPref' />"

        "<separator/>"
        "<menuitem action='SetupProxy'/>"
        "<menuitem action='SetupBrowser'/>"
        "<separator/>"
        "<menuitem action='SetupAbone'/>"
        "<menuitem action='SetupAboneThread'/>"
        "<menuitem action='TranspChainAbone'/>"
        "<separator/>"
        "<menuitem action='UseMosaic'/>"    
        "<menuitem action='DeleteImages'/>"
        "</menu>"                         

    // ヘルプ
        "<menu action='Menu_Help'>"
        "<menuitem action='Manual'/>"
        "<separator/>"
        "<menuitem action='Hp'/>"
        "<menuitem action='Bbs'/>"
        "<menuitem action='OldLog'/>"
        "<separator/>"
        "<menuitem action='About'/>"
        "</menu>"                         

        "</menubar>"
        "</ui>";
    m_ui_manager->add_ui_from_string( str_ui );
    m_menubar = dynamic_cast< Gtk::MenuBar* >( m_ui_manager->get_widget("/menu_bar") );
    assert( m_menubar );

    // 板履歴メニュー追加
    m_histmenu_board = Gtk::manage( new HistoryMenuBoard() );
    m_menubar->items().insert( --(--( m_menubar->items().end() )), *m_histmenu_board );

    // スレ履歴メニュー追加
    m_histmenu_thread = Gtk::manage( new HistoryMenuThread() );
    m_menubar->items().insert( --(--( m_menubar->items().end() )), *m_histmenu_thread );


    // メニューにショートカットキーやマウスジェスチャを表示
    Gtk::Menu_Helpers::MenuList& items = m_menubar->items();
    Gtk::Menu_Helpers::MenuList::iterator it_item = items.begin();
    for( ; it_item != items.end(); ++it_item ){
        Gtk::Menu* menu = dynamic_cast< Gtk::Menu* >( (*it_item).get_submenu() );
        CONTROL::set_menu_motion( menu );

        ( *it_item ).signal_activate().connect( sigc::mem_fun( *this, &Core::slot_activate_menubar ) );
    }

    // 初回起動時の設定
    if( init ) first_setup();

    // ツールバー
    create_toolbar();

    // サイドバー
    m_sidebar = BBSLIST::get_admin()->get_widget();
    assert( m_sidebar );

    // ステータスバー
    std::string str_tmp;
#if GTKMMVER <= 240
    m_statbar.pack_start( m_mginfo );
#else
    m_statbar.pack_start( m_label_stat, Gtk::PACK_SHRINK );
    m_statbar.pack_end( m_mginfo, Gtk::PACK_SHRINK );
    m_mginfo.set_width_chars( MAX_MG_LNG * 2 + 16 );
    m_mginfo.set_justify( Gtk::JUSTIFY_LEFT );
#endif
    m_statbar.show_all_children();

    m_stat_scrbar.add( m_statbar );
    m_stat_scrbar.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_stat_scrbar.set_size_request( 8 );

    // その他設定とwidgetのパッキング
    m_notebook.set_show_tabs( false );
    m_vbox_main.set_spacing( 4 );
    m_hpaned.set_mode( SKELETON::HPANED_MODE_LEFT );

    pack_widget( false );

    m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );
    m_hpaned.sig_show_hide_leftpane().connect( sigc::mem_fun( *this, &Core::slot_show_hide_leftpane ) );

    m_win_main.add( m_vbox_main );
    m_win_main.signal_focus_out_event().connect( sigc::mem_fun(*this, &Core::slot_focus_out_event ) );
    m_win_main.signal_focus_in_event().connect( sigc::mem_fun(*this, &Core::slot_focus_in_event ) );
    m_win_main.show_all_children();

    // 各管理クラスが開いていたURLを復元
    core_set_command( "restore_views" );
}



//
// widget のパック
//
void Core::pack_widget( bool unpack )
{
    m_enable_menuslot = false;

    int mode_pane = SESSION::get_mode_pane();

    if( unpack ){
        SESSION::set_hpane_main_pos( m_hpaned.get_position() );
        SESSION::set_vpane_main_pos( m_vpaned_r.get_position() );
        SESSION::set_hpane_main_r_pos( m_hpaned_r.get_position() );
        SESSION::set_vpane_main_mes_pos( m_vpaned_message.get_position() );
    }

    if( SESSION::get_embedded_mes() ){ // 埋め込みmessage

        // 書き込みウィンドウを閉じる
        MESSAGE::get_admin()->set_command_immediately( "close_window" );

        m_vpaned_message.add_remove1( unpack, *ARTICLE::get_admin()->get_widget() );
        m_vpaned_message.add_remove2( unpack, *MESSAGE::get_admin()->get_widget() );

        m_notebook.append_remove_page( unpack, m_vpaned_message, "スレッド" );
    }
    else{

        // 書き込みウィンドウ表示
        MESSAGE::get_admin()->set_command_immediately( "open_window" );

        m_notebook.append_remove_page( unpack, *ARTICLE::get_admin()->get_widget(), "スレッド" );
    }

    if( SESSION::get_embedded_img() ){ // 埋め込みimage

        // 画像ウィンドウを閉じる
        IMAGE::get_admin()->set_command_immediately( "close_window" );

        m_notebook.append_remove_page( unpack, *IMAGE::get_admin()->get_widget(), "画像" );
    }
    else{

        // 画像ウィンドウ表示
        IMAGE::get_admin()->set_command_immediately( "open_window" );
    }

    // 画像インジケータ
    if( unpack ) hide_imagetab();

    // 2ペーン
    if( mode_pane == SESSION::MODE_2PANE ){ 

        m_notebook.append_remove_page( unpack, *BOARD::get_admin()->get_widget(), "スレ一覧" );

        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT ) m_vbox_article.pack_remove_start( unpack, m_toolbar, Gtk::PACK_SHRINK );
        m_vbox_article.pack_remove_start( unpack, m_notebook );

        m_hpaned.add_remove1( unpack, *m_sidebar );
        m_hpaned.add_remove2( unpack, m_vbox_article );
    }

    // 3ペーン
    else if( mode_pane == SESSION::MODE_3PANE ){ 

        m_vbox_article.pack_remove_start( unpack, m_notebook );

        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT ){

            m_vbox_board.pack_remove_start( unpack, m_toolbar, Gtk::PACK_SHRINK );
            m_vbox_board.pack_remove_start( unpack, *BOARD::get_admin()->get_widget() );

            m_vpaned_r.add_remove1( unpack, m_vbox_board );
            m_vpaned_r.add_remove2( unpack, m_vbox_article );
        }
        else{
            m_vpaned_r.add_remove1( unpack, *BOARD::get_admin()->get_widget() );
            m_vpaned_r.add_remove2( unpack, m_vbox_article );
        }

        m_hpaned.add_remove1( unpack, *m_sidebar );
        m_hpaned.add_remove2( unpack, m_vpaned_r );
    }

    // 縦3ペーン
    else if( mode_pane == SESSION::MODE_V3PANE ){ 

        m_vbox_article.pack_remove_start( unpack, m_notebook );

        m_hpaned_r.add_remove1( unpack, *BOARD::get_admin()->get_widget() );
        m_hpaned_r.add_remove2( unpack, m_vbox_article );

        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT ){

            m_vbox_articleboard.pack_remove_start( unpack, m_toolbar, Gtk::PACK_SHRINK );
            m_vbox_articleboard.pack_remove_start( unpack, m_hpaned_r );

            m_hpaned.add_remove1( unpack, *m_sidebar );
            m_hpaned.add_remove2( unpack, m_vbox_articleboard );
        }
        else{
            m_hpaned.add_remove1( unpack, *m_sidebar );
            m_hpaned.add_remove2( unpack, m_hpaned_r );
        }
    }

    // メインwindowのパッキング
    m_vbox_main.pack_remove_end( unpack, m_stat_scrbar, Gtk::PACK_SHRINK );
    m_vbox_main.pack_remove_end( unpack, m_hpaned );
    if( SESSION::toolbar_pos() == SESSION::TOOLBAR_NORMAL ) m_vbox_main.pack_remove_end( unpack, m_toolbar, Gtk::PACK_SHRINK );
    m_vbox_main.pack_remove_end( unpack, *m_menubar, Gtk::PACK_SHRINK );

    if( ! unpack ){

        m_vbox_main.show_all_children();

        // ペーンの位置設定
        m_vpaned_r.set_position( SESSION::vpane_main_pos() );
        m_hpaned_r.set_position( SESSION::hpane_main_r_pos() );
        m_vpaned_message.set_position( SESSION::vpane_main_mes_pos() );

        // 埋め込みmessage
        if( SESSION::get_embedded_mes() ){
            if( MESSAGE::get_admin()->empty() ) m_vpaned_message.toggle_maximize( 1 );
        }

        // 画像インジケータ
        if( ! IMAGE::get_admin()->empty() ) show_imagetab();

        // サイドバーの位置設定
        m_hpaned.set_position( SESSION::hpane_main_pos() );
    }

    m_enable_menuslot = true;
}



//
// ツールバー作成
//
void Core::create_toolbar()
{
    m_button_bbslist.signal_clicked().connect( sigc::mem_fun( *this, &Core::switch_bbslist ) );
    m_button_favorite.signal_clicked().connect( sigc::mem_fun( *this, &Core::switch_favorite ) ); 
    m_button_board.signal_clicked().connect( sigc::mem_fun( *this, &Core::switch_board ) );
    m_button_thread.signal_clicked().connect( sigc::mem_fun( *this, &Core::switch_article ) );
    m_button_image.signal_clicked().connect( sigc::mem_fun( *this, &Core::switch_image ) );
    m_button_search_cache.signal_clicked().connect( sigc::mem_fun( *this, &Core::slot_search_cache ) );
    m_entry_url.signal_activate().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );
    m_button_go.signal_clicked().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );

    m_toolbar_vbox.pack_start( m_button_bbslist, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_button_favorite, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_button_board, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_button_thread, Gtk::PACK_SHRINK );
    if( CONFIG::get_use_image_view() ) m_toolbar_vbox.pack_start( m_button_image, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_vspr_toolbar_1, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_entry_url );
    m_toolbar_vbox.pack_start( m_button_go, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_vspr_toolbar_2, Gtk::PACK_SHRINK );
    m_toolbar_vbox.pack_start( m_button_search_cache, Gtk::PACK_SHRINK );
    m_toolbar.add( m_toolbar_vbox );
    m_toolbar.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_toolbar.set_size_request( 8 );

    m_tooltip.set_tip( m_button_go, "移動" );
    m_tooltip.set_tip( m_button_search_cache,"キャッシュ内の全ログ検索 " );
    m_tooltip.set_tip( m_button_bbslist, "板一覧" );
    m_tooltip.set_tip( m_button_favorite, "お気に入り" );
    m_tooltip.set_tip( m_button_board,
                       "スレ一覧\n\n"
                       + CONTROL::get_label_motion( CONTROL::ToggleArticle ) );
    m_tooltip.set_tip( m_button_thread,
                       "スレビュー\n\n"
                       + CONTROL::get_label_motion( CONTROL::ToggleArticle ) );
    m_tooltip.set_tip( m_button_image,"画像ビュー\n\nスレビュー切替 "
                       + CONTROL::get_motion( CONTROL::ToggleArticle ) + " , " + CONTROL::get_motion( CONTROL::Left ) );
}



//
// 初回起動時のセットアップ
//
void Core::first_setup()
{
    m_init = true;

    show_setupdiag( "JDセットアップへようこそ\n\nはじめにネットワークの設定をおこなって下さい" );

    slot_setup_proxy();
    slot_setup_browser();

    show_setupdiag( "JDセットアップ\n\nフォントと色の設定をおこなって下さい" );

    slot_setup_fontcolor();

    show_setupdiag( "JDセットアップ\n\nその他の設定は起動後に設定メニューからおこなって下さい" );
    show_setupdiag( "JDセットアップ完了\n\nOKを押すとJDを起動して板のリストをロードします\nリストが表示されるまでしばらくお待ち下さい" );

    m_init = false;
}



//
// セットアップ中ダイアログ表示
//
void Core::show_setupdiag( const std::string& msg )
{
    SKELETON::MsgDiag* mdiag = new SKELETON::MsgDiag( msg );
    mdiag->set_title( "JDセットアップ" );
    mdiag->set_keep_above( true );
    mdiag->set_skip_taskbar_hint( false );
    mdiag->run();
    delete mdiag;
}



//
// SIGHUPを受け取った
//
// 時間のかかる処理は行わないこと
//
void Core::shutdown()
{
    // 設定保存
    CONFIG::save_conf();

    ARTICLE::get_admin()->shutdown();
    BOARD::get_admin()->shutdown();
    BBSLIST::get_admin()->shutdown();
    IMAGE::get_admin()->shutdown();
}


//
// メインタイトルセット
//
void Core::set_maintitle()
{
    if( m_boot ) return;

    std::string title;

    if( m_title.empty() ){

#ifdef JDVERSION_CVS
        title = std::string( "JD - " ) + "cvs." + std::string( __DATE__ ) + "-" + std::string( __TIME__ );
#else
        title = std::string( "JD - " ) + std::string( JDVERSIONSTR );
#endif

    } else title = "JD - " + m_title;

    if( LOGIN::get_login2ch()->login_now() ) title +=" [ ログイン中 ]";
    if( ! SESSION::is_online() ) title += " [ オフライン ]";
    m_win_main.set_title( title );
}



//
// メニューバーがアクティブになったときに呼ばれるスロット
//
void Core::slot_activate_menubar()
{
    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // サイドバー
    Glib::RefPtr< Gtk::Action > act = m_action_group->get_action( "Show_BBS" );
    Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){

        if( SESSION::show_sidebar() && BBSLIST::get_admin()->get_current_url() == URL_BBSLISTVIEW ) tact->set_active( true );
        else tact->set_active( false );
    }

    act = m_action_group->get_action( "Show_FAVORITE" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){

        if( SESSION::show_sidebar() && BBSLIST::get_admin()->get_current_url() == URL_FAVORITEVIEW ) tact->set_active( true );
        else tact->set_active( false );
    }


    // ログイン
    act = m_action_group->get_action( "Login2ch" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
    if( tact ){

        if( LOGIN::get_login2ch()->login_now() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 表示->スレ一覧に切替 (アクティブ状態を切り替える)
    act = m_action_group->get_action( "Show_Board" );
    if( BOARD::get_admin()->empty() ) act->set_sensitive( false );
    else act->set_sensitive( true );

    // 表示->スレビューに切替 (アクティブ状態を切り替える)
    act = m_action_group->get_action( "Show_Thread" );
    if( ARTICLE::get_admin()->empty() ) act->set_sensitive( false );
    else act->set_sensitive( true );

    // 表示->画像ビューに切替 (アクティブ状態を切り替える)
    act = m_action_group->get_action( "Show_Image" );
    if( CONFIG::get_use_image_view() && ! IMAGE::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );

    m_enable_menuslot = true;
}



//
// 書き込みログを保存
//
void Core::slot_toggle_save_postlog()
{
    CONFIG::set_save_postlog( ! CONFIG::get_save_postlog() );
}


//
// 画像モザイクon/off
//
void Core::slot_toggle_use_mosaic()
{
    CONFIG::set_use_mosaic( ! CONFIG::get_use_mosaic() );

    SKELETON::MsgDiag mdiag( "次に開いた画像から有効になります" );
    mdiag.run();
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
// フォントと色の設定
//
void Core::slot_setup_fontcolor()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_FONTCOLOR, "" );
    pref->run();
    delete pref;
}


//
// プロキシ設定
//
void Core::slot_setup_proxy()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_PROXY, "" );
    pref->run();
    delete pref;
}


//
// あぼーん設定
//
void Core::slot_setup_abone()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_GLOBALABONE, "" );
    pref->run();
    delete pref;
}


//
// スレあぼーん設定
//
void Core::slot_setup_abone_thread()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_GLOBALABONETHREAD, "" );
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


//
// パスワード設定
//
void Core::slot_setup_passwd()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_PASSWD, "" );
    pref->run();
    delete pref;
}



//
// ブラウザ設定
//
void Core::slot_setup_browser()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_BROWSER, "" );
    pref->run();
    delete pref;
}



//
// HP
//
void Core::slot_show_hp()
{
    open_by_browser( JDURL );
}


//
// サポートBBS
//
void Core::slot_show_bbs()
{
    CORE::core_set_command( "open_board" , DBTREE::url_subject( JDBBS ), "true" );
}


//
// 過去ログ
//
void Core::slot_show_old2ch()
{
    CORE::core_set_command( "open_board" , DBTREE::url_subject( JD2CHLOG ), "true" );
}


//
// マニュアル
//
void Core::slot_show_manual()
{
    open_by_browser( std::string( JDHELP ) );
}


//
// about
//
void Core::slot_show_about()
{
    std::stringstream ss;
    ss << "バージョン "
#ifdef JDVERSION_CVS
       << "cvs." + std::string( __DATE__ ) + "-" + std::string( __TIME__ )
#else
       << JDVERSIONSTR 
#endif
       << std::endl << std::endl << JDCOPYRIGHT;
    SKELETON::MsgDiag mdiag( ss.str() );
    mdiag.run();
}
    


//
// 終了
//
void Core::slot_quit()
{
    m_win_main.hide();
}



//
// 板リスト再読込
//
void Core::slot_reload_list()
{
    DBTREE::download_bbsmenu();
    CORE::core_set_command( "set_status","", "loading...." );        
}



//
// お気に入り保存
//
void Core::slot_save_favorite()
{
    CORE::core_set_command( "save_favorite","" );
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
    if( LOGIN::get_login2ch()->login_now() ){
        LOGIN::get_login2ch()->logout();
        set_maintitle();
    }

    // ログオフ中ならログイン開始
    else LOGIN::get_login2ch()->start_login();
}



//
// ツールバー表示切替え
//
void Core::slot_toggle_toolbar()
{
    SESSION::set_show_toolbar( !SESSION::show_toolbar() );

    if( SESSION::show_toolbar() ){

        Gtk::Widget* menubar = m_ui_manager->get_widget("/menu_bar");
        assert( menubar );

        m_vbox_main.remove( *menubar );
        m_vbox_main.pack_end( m_toolbar, Gtk::PACK_SHRINK );
        m_vbox_main.pack_end( *menubar, Gtk::PACK_SHRINK );
    }
    else m_vbox_main.remove( m_toolbar );

    m_win_main.show_all_children();
}


//
// サイドバー表示切替え
//
void Core::slot_toggle_sidebar()
{
#ifdef _DEBUG
    std::cout << "Core::slot_toggle_sidebar focus = " << SESSION::focused_admin() << std::endl;
#endif
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;
    if( SESSION::focused_admin() == SESSION::FOCUS_SIDEBAR
        && BOARD::get_admin()->empty()
        && ARTICLE::get_admin()->empty() 
        && IMAGE::get_admin()->empty() ) return;

    m_hpaned.show_hide_leftpane();
}


//
// キャッシュ内のログ検索
//
void Core::slot_search_cache()
{
    CORE::core_set_command( "open_article_searchcache", "dummyurl" , "", "false", "all" );
}


//
// サイドバーの表示が切り替わったときに呼ばれる
//
void Core::slot_show_hide_leftpane( bool show )
{
#ifdef _DEBUG
    std::cout << "slot_show_hide_leftpane show = " << show << std::endl;
#endif

    SESSION::set_show_sidebar( show );

    // 表示されたらbbslistをフォーカス
    if( SESSION::show_sidebar() ) switch_sidebar();

    // 非表示になったときは SESSION::focused_admin_sidebar() で指定されるadminにフォーカスを移す
    else{

#ifdef _DEBUG
        std::cout << "focused_admin = " << SESSION::focused_admin_sidebar() << std::endl;
#endif

        if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_BOARD ) switch_board();
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_ARTICLE ) switch_article();
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_IMAGE ) switch_image();
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_MESSAGE ) switch_message();
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_NO ){

            if( ! BOARD::get_admin()->empty() ) switch_board();
            else if( ! ARTICLE::get_admin()->empty() ) switch_article();
            else if( ! IMAGE::get_admin()->empty() ) switch_image();
        }
    }
}



//
// ツールバーの表示モード
//
void Core::slot_toggle_toolbarpos( int pos )
{
    if( SESSION::toolbar_pos() == pos ) return;

    pack_widget( true );
    SESSION::set_toolbar_pos( pos );
    pack_widget( false );

    restore_focus( true );
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

    restore_focus( true );
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

    restore_focus( true );
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

    restore_focus( true );
}


//
// 埋め込みmessageビュー
//
void Core::slot_toggle_embedded_mes()
{
    pack_widget( true );
    SESSION::set_embedded_mes( ! SESSION::get_embedded_mes() );
    pack_widget( false );

    restore_focus( true );
}


//
// 埋め込みimageビュー
//
void Core::slot_toggle_embedded_img()
{
    pack_widget( true );
    SESSION::set_embedded_img( ! SESSION::get_embedded_img() );
    pack_widget( false );

    restore_focus( true );
}


//
// 過去ログ表示切替え
//
void Core::slot_toggle_oldarticle()
{
    CONFIG::set_show_oldarticle( ! CONFIG::get_show_oldarticle() );

    SKELETON::MsgDiag mdiag( "次に開いた板から有効になります" );
    mdiag.run();
}



//
// タブで開くボタンを入れ替える
//
void Core::slot_toggle_tabbutton()
{
    CONFIG::get_buttonconfig()->toggle_tab_button();
}


//
// クリックで多重ポップアップモードに移行
//
void Core::slot_toggle_popupwarpmode()
{
    CONFIG::get_buttonconfig()->toggle_popup_warpmode();
}


//
// editview を emacs風のキーバインドにする
void Core::slot_toggle_emacsmode()
{
    CONFIG::get_keyconfig()->toggle_emacs_mode();
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
// コマンドセット
//
// 他のadminクラスに委譲する場合はこの関数で、coreが実行するコマンドはexec_command()で処理
//
void Core::set_command( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "Core::set_command : " << command.command << " " << command.url
              << " " << command.arg1 << " " << command.arg2 << " " << command.arg3 << " " << command.arg4 << std::endl;
#endif
    
    ////////////////////////////
    // article系のコマンド

    // メインビュー
    if( command.command  == "open_article" ) {

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url,

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           command.arg1, // "true"ならtabで開く
                                           "false", // url 開いてるかチェックする
                                           command.arg2, // 開き方のモード ( Admin::open_view 参照 )

                                           "MAIN" // メインモードでarticleを開く
            );

        // ジャンプ( empty ならジャンプしない )
        if( ! command.arg3.empty() ) ARTICLE::get_admin()->set_command( "goto_num", command.url, command.arg3 );

        // 履歴更新
        set_history_article( command.url );

        return;
    }

    // メインビューを複数開く
    // 
    if( command.command  == "open_article_list" ) {

        ARTICLE::get_admin()->set_command( "open_list",
                                           std::string(),

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           command.arg1 // datファイルのURLを空白で区切って指定
            );

        return;
    }

    // キーワードで抽出( AND/OR )
    else if( command.command  == "open_article_keyword" ) { 

        std::string mode_str = "KEYWORD";
        if( command.arg2 == "true" ) mode_str = "KEYWORD_OR";  // OR 抽出
        
        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           mode_str, // キーワード抽出モード

                                           command.arg1 // query
            );
        return;
    }

    // レス抽出
    else if( command.command  == "open_article_res" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "RES", // レス抽出モード
                                     
                                           command.arg1, // レス番号 ( from-to )
                                           command.arg2  // ジャンプ番号( empty ならジャンプしない )
            );

        return;
    }


    // 名前 で抽出
    else if( command.command  == "open_article_name" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "NAME", // 名前抽出モード
                                     
                                           command.arg1 // 名前
            );
        return;
    }


    // ID で抽出
    else if( command.command  == "open_article_id" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "ID", // ID 抽出モード
                                     
                                           command.arg1 // ユーザID
            );
        return;
    }

    // ブックマークで抽出
    else if( command.command  == "open_article_bm" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "BM" //　ブックマーク抽出モード
            );
        return;
    }

    // URL抽出
    else if( command.command  == "open_article_url" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "URL" // URL抽出モード
            );
        return;
    }

    // 参照抽出
    else if( command.command  == "open_article_refer" ) { 

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "REF", // 参照抽出モード
                                     
                                           command.arg1 // 対象レス番号
            );
        return;
    }

    // ログ検索
    else if( command.command  == "open_article_searchcache" ) { 

        std::string mode_str = "SEARCHCACHE";
        if( command.arg2 == "true" ) mode_str = "SEARCHCACHE_OR";  // OR 検索
        
        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           mode_str, // モード

                                           command.arg1, // query
                                           command.arg3  // "all" の時は全ログを検索
            );

        return;
    }

    // datが更新されたときにローダから呼ばれる
    else if( command.command  == "update_article" ){  

        ARTICLE::get_admin()->set_command( "update_view", command.url );
        return;
    }

    // datが更新が終わったときにローダから呼ばれる
    else if( command.command  == "update_article_finish" ){ 

        ARTICLE::get_admin()->set_command( "update_finish", command.url );
        return;
    }

    // articleの削除
    else if( command.command == "delete_article" ){

        ARTICLE::get_admin()->set_command( "close_view", command.url,
                                           "true" // command.url を含む全てのビューを閉じる
            );

        DBTREE::delete_article( command.url );
        return;
    }

    // 全articleviewの再レイアウト
    else if( command.command == "relayout_all_article" ){
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }

    // 全articleviewのフォントの初期化
    else if( command.command == "init_font_all_article" ){
        ARTICLE::get_admin()->set_command( "init_font" );
    }


    ////////////////////////////
    // board系のコマンド

    else if( command.command  == "open_board" ){

        BOARD::get_admin()->set_command( "open_view",
                                         command.url,

                                         command.arg1,  // "true" ならtabで開く
                                         "false", // url 開いてるかチェック
                                         command.arg2 // 開き方のモード ( Admin::open_view 参照 )
            );

        set_history_board( command.url );
        return;
    }

    // 複数開く
    // 
    if( command.command  == "open_board_list" ) {

        BOARD::get_admin()->set_command( "open_list",
                                         std::string(),

                                         // 以下 COMMAND_ARGS::arg1, arg2,....
                                         command.arg1 // datファイルのURLを空白で区切って指定
            );

        return;
    }

    else if( command.command  == "update_board" ){

        BOARD::get_admin()->set_command( "update_view", command.url );
        return;
    }

    else if( command.command  == "update_board_item" ){

        BOARD::get_admin()->set_command( "update_item", command.url,
                                         command.arg1 // スレID
            );
        return;
    }
   
    // 全boardviewの再レイアウト
    else if( command.command == "relayout_all_board" ){
        BOARD::get_admin()->set_command( "relayout_all" );
    }


    ////////////////////////////
    // bbslist系のコマンド

    else if( command.command  == "update_bbslist" ){

        BBSLIST::get_admin()->set_command( "update_view", URL_BBSLISTVIEW );
        return;
    }
    else if( command.command  == "update_bbslist_item" ){

        BBSLIST::get_admin()->set_command( "update_item", URL_BBSLISTVIEW );
        return;
    }

    else if( command.command  == "append_favorite" ){

        BBSLIST::get_admin()->set_command( "append_favorite", command.url, command.arg1, command.arg2 );
        return;
    }
    else if( command.command  == "update_favorite_item" ){

        BBSLIST::get_admin()->set_command( "update_item", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "save_favorite" ){

        BBSLIST::get_admin()->set_command( "save_favorite" );
        return;
    }

    // 全bbslistviewの再レイアウト
    else if( command.command == "relayout_all_bbslist" ){
        BBSLIST::get_admin()->set_command( "relayout_all" );
    }


    ////////////////////////////
    // image系のコマンド

    else if( command.command == "open_image" ){

        show_imagetab();

        // キャッシュに無かったらロード
        if( ! DBIMG::is_cached( command.url ) ) DBIMG::download_img( command.url );

        IMAGE::get_admin()->set_command( "open_view", command.url );
        return;
    }
    else if( command.command == "delete_image" ){

        DBIMG::delete_cache( command.url );
        return;
    }
    else if( command.command == "close_image_view" ){

        // ダイアログから削除したときにフォーカスが外れるので
        // フォーカス状態を回復してからcloseする
        restore_focus( false );

        IMAGE::get_admin()->set_command( "close_view", command.url );
        return;
    }

    ////////////////////////////
    // message系

    else if( command.command == "open_message" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( "オフラインです" );
            mdiag.run();
        }
        else{

            if( SESSION::get_embedded_mes() ) m_vpaned_message.toggle_maximize( 0 );
            MESSAGE::get_admin()->set_command( "open_view", command.url, command.arg1 );
        }
    }

    else if( command.command == "create_new_thread" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( "オフラインです" );
            mdiag.run();
        }
        else if( DBTREE::url_bbscgi_new( command.url ).empty() ){
            SKELETON::MsgDiag mdiag( "この板では新スレを立てることは出来ません" );
            mdiag.run();
        }
        else MESSAGE::get_admin()->set_command( "create_new_thread", command.url, command.arg1 );
    }

    ////////////////////////////
    // 再描画系

    // command.url を含むviewを全て再描画
    else if( command.command == "redraw" ){

        ARTICLE::get_admin()->set_command( "redraw", command.url );
        BOARD::get_admin()->set_command( "redraw", command.url );
        BBSLIST::get_admin()->set_command( "redraw", command.url );
        IMAGE::get_admin()->set_command( "redraw", command.url );
        MESSAGE::get_admin()->set_command( "redraw", command.url );
        return;
    }

    // 表示中のbbslist viewを再描画
    else if( command.command  == "redraw_bbslist" ) {

        BBSLIST::get_admin()->set_command( "redraw_current_view" );
        return;
    }

    // 表示中のboard viewを再描画
    else if( command.command  == "redraw_board" ) {

        BOARD::get_admin()->set_command( "redraw_current_view" );
        return;
    }

    // 表示中のarticle viewを再描画
    else if( command.command  == "redraw_article" ) {

        ARTICLE::get_admin()->set_command( "redraw_current_view" );
        return;
    }

    // 表示中のmessage viewを再描画
    else if( command.command  == "redraw_message" ) {

        MESSAGE::get_admin()->set_command( "redraw_current_view" );
        return;
    }

    // command.url を含むarticle viewを全て再レイアウトして再描画
    else if( command.command == "relayout_article" ){

        ARTICLE::get_admin()->set_command( "relayout_views", command.url );
        return;
    }

    // 表示中のimage viewを再描画
    else if( command.command  == "redraw_image" ) {

        IMAGE::get_admin()->set_command( "redraw_current_view" );
        return;
    }

    // 表示中のviewを全部再描画
    else if( command.command  == "redraw_all" ) {

        ARTICLE::get_admin()->set_command( "redraw_current_view" );
        BOARD::get_admin()->set_command( "redraw_current_view" );
        BBSLIST::get_admin()->set_command( "redraw_current_view" );
        IMAGE::get_admin()->set_command( "redraw_current_view" );

        return;
    }

    // フォーカス回復
    else if( command.command == "restore_focus" ){

        restore_focus( true );
        return;
    }

    ////////////////////////////
    // Coreが自前で処理するコマンド( Core::exec_command() で処理 )

    m_list_command.push_back( command );
    m_disp.emit();
}


// coreが自前でする処理
void Core::exec_command()
{
    if( m_list_command.size() == 0 ) return;
    
    COMMAND_ARGS command = m_list_command.front();
    m_list_command.pop_front();

#ifdef _DEBUG
    std::cout << "Core::exec_command : " << command.command << " " << command.url
              << " " << command.arg1 << " " << command.arg2 << " " << command.arg3 << " " << command.arg4 << std::endl;
#endif

    // 各管理クラスが開いていたURLを復元
    if( command.command == "restore_views" ){

        // bbslist は無条件でリストア
        // 板一覧がロードされてない時はここでロードされる
        BBSLIST::get_admin()->set_command( "restore" );

        // その他は設定されていたらリストア
        if( CONFIG::get_restore_board() ) BOARD::get_admin()->set_command( "restore" );
        if( CONFIG::get_restore_article() ) ARTICLE::get_admin()->set_command( "restore" );
        if( CONFIG::get_restore_image() && SESSION::image_URLs().size() ){

            show_imagetab();

            IMAGE::get_admin()->set_command( "restore" );
        }
    }

    // 各ビューのタブ幅調整
    else if( command.command == "adjust_tabwidth" ){

        BOARD::get_admin()->set_command( "adjust_tabwidth" );
        ARTICLE::get_admin()->set_command( "adjust_tabwidth" );
    }

    // history 登録
    else if( command.command  == "set_history_article" ) set_history_article( command.url );
        
    else if( command.command  == "set_history_board" ) set_history_board( command.url );

    else if( command.command  == "update_history" ){
        if( m_histmenu_thread ) m_histmenu_thread->update();
        if( m_histmenu_board ) m_histmenu_board->update();
    }

    // ビューの切替え
    else if( command.command  == "switch_article" ) switch_article();

    else if( command.command  == "switch_board" ) switch_board();

    else if( command.command  == "switch_sidebar" ) switch_sidebar( command.url );

    else if( command.command  == "switch_image" ) switch_image();

    else if( command.command  == "switch_message" ) switch_message();

    else if( command.command  == "toggle_article" ) toggle_article();

    else if( command.command  == "switch_leftview" ) switch_leftview();

    else if( command.command  == "switch_rightview" ) switch_rightview();

    // サイドバー表示/非表示
    else if( command.command  == "toggle_sidebar" ) slot_toggle_sidebar();

    // 2chへのログイン処理が完了した
    else if( command.command  == "login2ch_finished" ) set_maintitle();

    // あるnotebookが空になった
    else if( command.command  == "empty_page" ) empty_page( command.url );

    // あるadminのnotebookのページが切り替わった
    else if( command.command  == "switch_page" ) switch_page( command.url );

    // タイトル、URL、ステータスなどの表示
    else if( command.command  == "set_title" ){
        m_title = command.arg1;
        set_maintitle();
    }

    else if( command.command  == "set_url" ){
        m_entry_url.set_text( command.url );
    }

    else if( command.command  == "set_status" ){
#if GTKMMVER <= 240
        m_statbar.push( command.arg1 );
#else
        m_label_stat.set_text( command.arg1 );
#endif        
    }

    // マウスジェスチャ
    else if( command.command  == "set_mginfo" ){
        m_mginfo.set_text( command.arg1 );
    }

    // bbsmenu再読み込み
    else if( command.command == "reload_bbsmenu" ){
        slot_reload_list();
    }

    // グローバルあぼーん(名前)
    else if( command.command == "set_global_abone_name" ){

        std::list< std::string >list_tmp =  CONFIG::get_list_abone_name();
        list_tmp.push_back( command.arg1 );
        CONFIG::set_list_abone_name( list_tmp );
        DBTREE::update_abone_all_article();
        CORE::core_set_command( "relayout_all_article" );
    }

    // グローバルあぼーん(名前)
    else if( command.command == "set_global_abone_word" ){

        std::list< std::string >list_tmp =  CONFIG::get_list_abone_word();
        list_tmp.push_back( command.arg1 );
        CONFIG::set_list_abone_word( list_tmp );
        DBTREE::update_abone_all_article();
        CORE::core_set_command( "relayout_all_article" );
    }

    // ユーザコマンド実行
    else if( command.command  == "exec_usr_cmd" ){

        CORE::get_usrcmd_manager()->exec( atoi( command.arg1.c_str() ), // コマンド番号
                                          command.url, // URL
                                          command.arg2, // Link
                                          command.arg3 // 選択文字
            );
    }

    // URL のオープン関係

    // 常に外部ブラウザで開く場合
    else if( command.command  == "open_url_browser" ) open_by_browser( command.url );

    // タイプによって判定する場合
    else if( command.command  == "open_url" ){

        // プロトコルが指定されていなかったら"http://"を仮定する
        command.url = MISC::remove_space( command.url );
        if( command.url.find( "http://" ) != 0
            && command.url.find( "https://" ) != 0
            && command.url.find( "ftp://" ) != 0 ){

            command.url = "http://" + command.url;
        }

        int num_from, num_to;
        std::string url_dat = DBTREE::url_dat( command.url, num_from, num_to );
        std::string url_subject = DBTREE::url_subject( command.url );
       
        // datの場合ビューで開く
        if( ! url_dat.empty() ){

#ifdef _DEBUG
            std::cout << "exec : open_article url = " << url_dat << std::endl;
#endif
            
            if( num_from ) CORE::core_set_command( "open_article" , url_dat, "left", // 現在表示中のビューの左に表示する
                                                   "", MISC::itostr( num_from ) );
            else CORE::core_set_command( "open_article" , url_dat, "left", // 現在表示中のビューの左に表示する
                                         "" );
        }

        // 掲示板のベースURLの場合
        else if( ! url_subject.empty() ){

#ifdef _DEBUG
            std::cout << "exec : open_board url = " << url_subject << std::endl;
#endif

            CORE::core_set_command( "open_board" , url_subject, "true" );
        }

        // 画像の場合
        else if( DBIMG::is_loadable( command.url ) && CONFIG::get_use_image_view() ){

            if( ! SESSION::is_online() ){
                SKELETON::MsgDiag mdiag( "オフラインです" );
                mdiag.run();
            }
            else{
                // キャッシュに無かったらロード
                if( ! DBIMG::is_cached( command.url ) ) DBIMG::download_img( command.url );

                CORE::core_set_command( "open_image", command.url );
                CORE::core_set_command( "switch_image" );
            }
        }

        // その他
        else open_by_browser( command.url );
    }

    // ある admin クラスのコマンドが空になった
    else if( command.command  == "empty_command" ){}

    // 起動中
    if( m_boot && ! m_init ){

        // coreがコマンドを全て実行して、かつ全てのadminクラスがブートした
        if( m_list_command.size() == 0
            && ! BBSLIST::get_admin()->is_booting()
            && ! BOARD::get_admin()->is_booting()
            && ! ARTICLE::get_admin()->is_booting()
            && ! IMAGE::get_admin()->is_booting() ){

            // 起動完了
            m_boot = false;
            exec_command_after_boot();
        }
    }
}



//
// 起動処理完了後に実行する処理
//
void Core::exec_command_after_boot()
{
#ifdef _DEBUG
    std::cout << "Core::exec_command_after_boot\n";
#endif

    restore_focus( true );

    // サイドバー表示状態変更
    if( ! SESSION::show_sidebar() ) m_hpaned.show_hide_leftpane();

    // ツールバー表示切り替え
    if( ! SESSION::show_toolbar() ) m_vbox_main.remove( m_toolbar );

    // 埋め込みmessage表示切り替え
    if( SESSION::get_embedded_mes() ){
        if( MESSAGE::get_admin()->empty() ) m_vpaned_message.toggle_maximize( 1 );
    }

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &Core::slot_timeout), 0 );
    sigc::connection conn = Glib::signal_timeout().connect( slot_timeout, TIMER_TIMEOUT );

    // 2chログイン
    if( SESSION::login2ch() ) slot_toggle_login2ch();

    // タイトル表示
    set_maintitle();

#ifdef _DEBUG
    std::cout << "\n\n----------- boot fin --------------\n\n";
#endif
}


//
// フォーカス回復
//
// force : true の時は強制的に回復(処理が重い)
//
void Core::restore_focus( bool force )
{
    int admin = SESSION::focused_admin();

#ifdef _DEBUG
    std::cout << "Core::restore_focus admin = " << admin << std::endl;
#endif

    // 画像ウィンドウが表示されているときは画像ウィンドウのフォーカスを外す
    if( ! SESSION::get_embedded_img() ) IMAGE::get_admin()->set_command( "focus_out" );

    if( ! force ){

        // フォーカス状態回復
        switch( admin )
        {
            case SESSION::FOCUS_SIDEBAR: BBSLIST::get_admin()->set_command( "restore_focus" ); break;
            case SESSION::FOCUS_BOARD: BOARD::get_admin()->set_command( "restore_focus" ); break;
            case SESSION::FOCUS_ARTICLE: ARTICLE::get_admin()->set_command( "restore_focus" ); break;
            case SESSION::FOCUS_IMAGE: IMAGE::get_admin()->set_command( "restore_focus" ); break;
            case SESSION::FOCUS_MESSAGE: MESSAGE::get_admin()->set_command( "restore_focus" ); break;
        }

    } else {
        
        int admin_sidebar = SESSION::focused_admin_sidebar();
        SESSION::set_focused_admin( SESSION::FOCUS_NO );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );

        // adminの表示状態回復
        set_right_current_page( SESSION::notebook_main_page() );

        // フォーカス状態回復
        switch( admin ){
            case SESSION::FOCUS_SIDEBAR: switch_sidebar(); break;
            case SESSION::FOCUS_BOARD: switch_board(); break;
            case SESSION::FOCUS_ARTICLE: switch_article(); break;
            case SESSION::FOCUS_IMAGE: switch_image(); break;
            case SESSION::FOCUS_MESSAGE: switch_message(); break;
        }
        SESSION::set_focused_admin( admin );
        SESSION::set_focused_admin_sidebar( admin_sidebar );
    }
}




//
// メインタイマー
//
// TIMER_TIMEOUT msec毎に呼び出される
//
bool Core::slot_timeout( int timer_number )
{
    // 各管理クラスにクロック入力する
    BBSLIST::get_admin()->clock_in();    
    BOARD::get_admin()->clock_in();
    ARTICLE::get_admin()->clock_in();
    IMAGE::get_admin()->clock_in();
    MESSAGE::get_admin()->clock_in();

    // Panedにクロック入力
    m_vpaned_r.clock_in();
    m_vpaned_message.clock_in();
   
    return true;
}


//
// 右ペーンのnotebookのタブの切替え
//
void Core::slot_switch_page( GtkNotebookPage*, guint )
{
    int page = get_right_current_page();

#ifdef _DEBUG
    std::cout << "Core::slot_switch_page " << page << std::endl;
#endif

    switch( page ){

        case PAGE_ARTICLE: switch_article(); break;

        case PAGE_IMAGE: switch_image(); break;

        case PAGE_BOARD: switch_board(); break;
    }
}


// 右ペーンのnotebookのページ番号
int Core::get_right_current_page()
{
    int mode = SESSION::get_mode_pane();
    int page = m_notebook.get_current_page();

    if( mode == SESSION::MODE_2PANE ){

        // 2paneで画像ビューをウィンドウ表示している場合
        if( ! SESSION::get_embedded_img() && page == 1 ) page = PAGE_BOARD;
    }

    return page;
}


// 右ペーンのnotebookのページをセット
void Core::set_right_current_page( int page )
{
    if( get_right_current_page() == page ) return;

    // 画像ビューをウィンドウ表示している場合
    if( ! SESSION::get_embedded_img() && page == PAGE_IMAGE ) return;

    if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

        // 2paneで画像ビューをウィンドウ表示している場合
        if( ! SESSION::get_embedded_img() && page == PAGE_BOARD ) page = 1;
    }
    else if( page == PAGE_BOARD ) return; // 2pane以外ではboardはnotebookに含まれない

    m_notebook.set_current_page( page );
    SESSION::set_notebook_main_page( get_right_current_page() );
}



//
// フォーカスアウトイベント
//
bool Core::slot_focus_out_event( GdkEventFocus* )
{
#ifdef _DEBUG
    std::cout << "Core::slot_focus_out_event admin = " << SESSION::focused_admin() << std::endl;
#endif

    SESSION::set_focus_win_main( false );
    FOCUS_OUT_ALL();

    return true;
}


//
// フォーカスインイベント
//
bool Core::slot_focus_in_event( GdkEventFocus* )
{
#ifdef _DEBUG
    std::cout << "Core::slot_focus_in_event admin = " << SESSION::focused_admin() << std::endl;
#endif

    SESSION::set_focus_win_main( true );
    restore_focus( false );

    return true;
}


//
// メインウィンドウの状態が変わった
//
bool Core::slot_window_state_event( GdkEventWindowState* event )
{
#ifdef _DEBUG
    std::cout << "Core::slot_window_state_event\n";
#endif     

    SESSION::set_iconified_win_main( event->new_window_state & GDK_WINDOW_STATE_ICONIFIED );

    // タブ幅調整
    CORE::core_set_command( "adjust_tabwidth" );

    return true;
}


//
// URL entryでenterを押した
//
void Core::slot_active_url()
{
    std::string url = m_entry_url.get_text();
    if( !url.empty() ) CORE::core_set_command( "open_url", url );
}



//
// あるadminがemptyになったので他のadminにスイッチ
//
// url : empty になったadmin
//
void Core::empty_page( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Core::empty_page url = " << url << std::endl;
#endif

    bool emb_img = SESSION::get_embedded_img();
    int focused_admin = SESSION::FOCUS_NO;

    // emptyになったadminとフォーカスされているadminが異なる場合は
    // フォーカスを移動しない
    if( SESSION::focused_admin() == SESSION::FOCUS_SIDEBAR )
        focused_admin = SESSION::FOCUS_SIDEBAR;
    else if( SESSION::focused_admin() == SESSION::FOCUS_BOARD && ! BOARD::get_admin()->empty() )
        focused_admin = SESSION::FOCUS_BOARD;
    else if( SESSION::focused_admin() == SESSION::FOCUS_ARTICLE && ! ARTICLE::get_admin()->empty() )
        focused_admin = SESSION::FOCUS_ARTICLE;
    else if( SESSION::focused_admin() == SESSION::FOCUS_IMAGE && ! IMAGE::get_admin()->empty() )
        focused_admin = SESSION::FOCUS_IMAGE;
    else if( SESSION::focused_admin() == SESSION::FOCUS_MESSAGE && ! MESSAGE::get_admin()->empty() )
        focused_admin = SESSION::FOCUS_MESSAGE;

    // 埋め込み画像ビューが空になった
    if( url == URL_IMAGEADMIN && emb_img ){

        hide_imagetab();

        // 空でないadminを前に出す
        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

            if( get_right_current_page() == PAGE_IMAGE ){
                if( ! ARTICLE::get_admin()->empty() ) switch_article();
                else if( ! BOARD::get_admin()->empty() ) switch_board();
            }
        }
        else if( ! ARTICLE::get_admin()->empty() ) switch_article();

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){

            if( ! ARTICLE::get_admin()->empty() ) focused_admin = SESSION::FOCUS_ARTICLE;
            else if( ! BOARD::get_admin()->empty() ) focused_admin = SESSION::FOCUS_BOARD;
            else{
                focused_admin = SESSION::FOCUS_SIDEBAR;
                SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
            }
        }
    }

    // articleビューが空になった
    else if( url == URL_ARTICLEADMIN ){

        // 空でないadminを前に出す
        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

            if( get_right_current_page() == PAGE_ARTICLE ){
                if( emb_img && BOARD::get_admin()->empty() && ! IMAGE::get_admin()->empty() ) switch_image();
                else if( ! BOARD::get_admin()->empty() ) switch_board();
            }
        }
        else if( emb_img && ! IMAGE::get_admin()->empty() ) switch_image();

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){

            if( ! BOARD::get_admin()->empty() ) focused_admin = SESSION::FOCUS_BOARD;
            else{
                focused_admin = SESSION::FOCUS_SIDEBAR;
                SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
            }
        }
    }

    // boardビューが空になった
    else if( url == URL_BOARDADMIN ){

        // 空でないadminを前に出す
        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

            if( get_right_current_page() == PAGE_BOARD ){
                if( ! ARTICLE::get_admin()->empty() ) switch_article();
                else if( emb_img && ! IMAGE::get_admin()->empty() ) switch_image();
            }
        }

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){
            focused_admin = SESSION::FOCUS_SIDEBAR;
            SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
        }
    }

    // 埋め込みmessageビューが空になった
    if( url == URL_MESSAGEADMIN && SESSION::get_embedded_mes() ){

        m_vpaned_message.toggle_maximize( 1 );

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){

            if( ! ARTICLE::get_admin()->empty() ) focused_admin = SESSION::FOCUS_ARTICLE;
            else if( ! BOARD::get_admin()->empty() ) focused_admin = SESSION::FOCUS_BOARD;
            else{
                focused_admin = SESSION::FOCUS_SIDEBAR;
                SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
            }
        }
    }

    // 切り替え実行
    switch( focused_admin ){

        case SESSION::FOCUS_SIDEBAR: switch_sidebar(); break;
        case SESSION::FOCUS_BOARD: switch_board(); break;
        case SESSION::FOCUS_ARTICLE: switch_article(); break;
        case SESSION::FOCUS_IMAGE: switch_image(); break;
        case SESSION::FOCUS_MESSAGE: switch_message(); break;
    }
}



//
// あるadminのnotebookのページがスイッチした
//
// url : adminのurl
//
void Core::switch_page( const std::string& url )
{
    if( url == URL_BBSLISTADMIN && SESSION::focused_admin() == SESSION::FOCUS_SIDEBAR ) switch_sidebar();
    else if( url == URL_BOARDADMIN && SESSION::focused_admin() == SESSION::FOCUS_BOARD ) switch_board();
    else if( url == URL_ARTICLEADMIN && SESSION::focused_admin() == SESSION::FOCUS_ARTICLE ) switch_article();
    else if( url == URL_IMAGEADMIN && SESSION::focused_admin() == SESSION::FOCUS_IMAGE ) switch_image();
    else if( url == URL_MESSAGEADMIN && SESSION::focused_admin() == SESSION::FOCUS_MESSAGE ) switch_message();
}


//
// ビューのトグルボタンを上げ下げする
//
void Core::set_toggle_view_button()
{
    m_enable_menuslot = false;

    switch( SESSION::focused_admin() ){

        case SESSION::FOCUS_SIDEBAR:

            if( BBSLIST::get_admin()->get_current_page() == 0 ){
                m_button_bbslist.set_active( true );
                m_button_favorite.set_active( false );
            }
            else{
                m_button_bbslist.set_active( false );
                m_button_favorite.set_active( true );
            }
            m_button_board.set_active( false );
            m_button_thread.set_active( false );
            m_button_image.set_active( false );
            break;
            
        case SESSION::FOCUS_BOARD:

            if( ! BOARD::get_admin()->empty() ){
                m_button_bbslist.set_active( false );
                m_button_favorite.set_active( false );
                m_button_board.set_active( true );
                m_button_thread.set_active( false );
                m_button_image.set_active( false );
            }
            else m_button_board.set_active( false );

            break;

        case SESSION::FOCUS_ARTICLE:

            if( ! ARTICLE::get_admin()->empty() ){
                m_button_bbslist.set_active( false );
                m_button_favorite.set_active( false );
                m_button_board.set_active( false );
                m_button_thread.set_active( true );
                m_button_image.set_active( false );
            } 
            else m_button_thread.set_active( false );

            break;

        case SESSION::FOCUS_IMAGE:

            if( ! IMAGE::get_admin()->empty() ){
                m_button_bbslist.set_active( false );
                m_button_favorite.set_active( false );
                m_button_board.set_active( false );
                m_button_thread.set_active( false );
                m_button_image.set_active( true );
            }
            else m_button_image.set_active( false );

            break;

        case SESSION::FOCUS_MESSAGE:
                m_button_bbslist.set_active( false );
                m_button_favorite.set_active( false );
                m_button_board.set_active( false );
                m_button_thread.set_active( false );
                m_button_image.set_active( false );

            break;
    }

    m_enable_menuslot = true;
}


//
// ビューのトグルボタンのsensitive状態を切り替える
//
void Core::set_sensitive_view_button()
{
    m_enable_menuslot = false;

    // スレ一覧ボタンの切り替え
    if( BOARD::get_admin()->empty() ) m_button_board.set_sensitive( false );
    else m_button_board.set_sensitive( true );

    // スレビューボタンの切り替え
    if( ARTICLE::get_admin()->empty() ) m_button_thread.set_sensitive( false );
    else m_button_thread.set_sensitive( true );

    // 画像ビューボタンの切り替え
    if( SESSION::get_embedded_img() && IMAGE::get_admin()->empty() ) m_button_image.set_sensitive( false );
    else m_button_image.set_sensitive( true );

    m_enable_menuslot = true;
}


//
// 右ペーンの最大化表示切り替え
//
void Core::toggle_maximize_rightpane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_3PANE ){

        // スレ一覧を最大化
        if( ! BOARD::get_admin()->empty() &&
            ( ARTICLE::get_admin()->empty() && IMAGE::get_admin()->empty() ) ) m_vpaned_r.toggle_maximize( 1 );

        // スレビューを最大化
        else if( BOARD::get_admin()->empty() &&
                 ( ! ARTICLE::get_admin()->empty() || ! IMAGE::get_admin()->empty() ) ) m_vpaned_r.toggle_maximize( 2 );

        // 戻す
        else if( ! BOARD::get_admin()->empty() &&
                 ( ! ARTICLE::get_admin()->empty() || ! IMAGE::get_admin()->empty() ) ) m_vpaned_r.toggle_maximize( 0 );
    }
}


//
// 各viewにスイッチ
//
void Core::switch_article()
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_article\n";
#endif

    if( ! ARTICLE::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_ARTICLE ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
            set_right_current_page( PAGE_ARTICLE );
        }

        ARTICLE::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_ARTICLE );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_ARTICLE );

        if( SESSION::get_embedded_img() ) SESSION::set_img_shown( false );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    m_win_main.present();
}


void Core::switch_board()
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_board\n";
#endif

    if( ! BOARD::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_BOARD ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
            set_right_current_page( PAGE_BOARD );
        }

        BOARD::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_BOARD );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_BOARD );

        // 3paneの時はboardに切り替えても(フォーカスアウトしても)
        // 画像は表示されたままの時があることに注意
        if( SESSION::get_embedded_img() && SESSION::get_mode_pane() == SESSION::MODE_2PANE ) SESSION::set_img_shown( false );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    m_win_main.present();
}


void Core::switch_bbslist()
{
    if( SESSION::focused_admin() == SESSION::FOCUS_SIDEBAR
        && BBSLIST::get_admin()->get_current_url() == URL_BBSLISTVIEW ) slot_toggle_sidebar();
    else switch_sidebar( URL_BBSLISTVIEW );
}


void Core::switch_favorite()
{
#ifdef _DEBUT
    std::cout << "Core::switch_favorite\n";
#endif    
    if( SESSION::focused_admin() == SESSION::FOCUS_SIDEBAR
        && BBSLIST::get_admin()->get_current_url() == URL_FAVORITEVIEW ) slot_toggle_sidebar();
    else switch_sidebar( URL_FAVORITEVIEW );
}


//
// url は表示するページ( URL_BBSLISTVIEW or URL_FAVORITEVIEW )
// urlが空の時はフォーカスを移すだけ
//
void Core::switch_sidebar( const std::string& url )
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_sidebar url - " << url << std::endl;
#endif

    if( ! BBSLIST::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_SIDEBAR ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );

            // サイドバーが隠れていたら開く
            if( ! SESSION::show_sidebar() ) slot_toggle_sidebar();
        }

        if( ! url.empty() ) BBSLIST::get_admin()->set_command( "switch_view", url );

        BBSLIST::get_admin()->set_command( "focus_current_view" ); 
        SESSION::set_focused_admin( SESSION::FOCUS_SIDEBAR );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    m_win_main.present();
}


void Core::switch_image()
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_image\n";
#endif

    if( ! IMAGE::get_admin()->empty() ){

        if( SESSION::get_embedded_img() ){ // 埋め込み画像ビュー

            if( SESSION::focused_admin() != SESSION::FOCUS_IMAGE ){

                FOCUS_OUT_ALL();
                ARTICLE::get_admin()->set_command( "delete_popup" );
                set_right_current_page( PAGE_IMAGE );

                // 画像強制表示
                IMAGE::get_admin()->set_command( "show_image" );
            }

            SESSION::set_focused_admin( SESSION::FOCUS_IMAGE );
            SESSION::set_focused_admin_sidebar( SESSION::FOCUS_IMAGE );
        }

        IMAGE::get_admin()->set_command( "focus_current_view" );
        if( SESSION::get_embedded_img() ) SESSION::set_img_shown( true );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( SESSION::get_embedded_img() ) m_win_main.present();
}


void Core::switch_message()
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;
    if( ! SESSION::get_embedded_mes() ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_message\n";
#endif

    if( ! MESSAGE::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_MESSAGE ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
        }

        MESSAGE::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_MESSAGE );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_MESSAGE );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( SESSION::get_embedded_mes() ) m_win_main.present();
}


// 2paneの時にboard <-> article 切替え
void Core::toggle_article()
{
    // 画像ウィンドウが表示されている場合
    if( ! SESSION::get_embedded_img() && SESSION::is_img_shown() ) switch_article(); 
    else if( SESSION::focused_admin() == SESSION::FOCUS_ARTICLE ) switch_board();
    else switch_article();
}


// 左移動
void Core::switch_leftview()
{
    int next_admin = SESSION::focused_admin();

    // 画像ウィンドウが表示されている
    if( ! SESSION::get_embedded_img() && SESSION::is_img_shown() ) next_admin = SESSION::FOCUS_IMAGE;

    for(;;){

        if( next_admin == SESSION::FOCUS_IMAGE ) next_admin = SESSION::FOCUS_ARTICLE;
        else if( next_admin == SESSION::FOCUS_ARTICLE ) next_admin = SESSION::FOCUS_BOARD;
        else if( next_admin == SESSION::FOCUS_BOARD ) next_admin = SESSION::FOCUS_SIDEBAR;
        else break;

        if( next_admin == SESSION::FOCUS_SIDEBAR && ! BBSLIST::get_admin()->empty() ){
            switch_sidebar();
            break;
        }
        else if( next_admin == SESSION::FOCUS_BOARD && ! BOARD::get_admin()->empty() ){
            switch_board();
            break;
        }
        else if( next_admin == SESSION::FOCUS_ARTICLE && ! ARTICLE::get_admin()->empty() ){
            switch_article();
            break;
        }
    }
}


// 右移動
void Core::switch_rightview()
{
    int next_admin = SESSION::focused_admin();
    
    for(;;){

        if( next_admin == SESSION::FOCUS_SIDEBAR ) next_admin = SESSION::FOCUS_BOARD;
        else if( next_admin == SESSION::FOCUS_BOARD ) next_admin = SESSION::FOCUS_ARTICLE;
        else if( next_admin == SESSION::FOCUS_ARTICLE ) next_admin = SESSION::FOCUS_IMAGE;
        else break;

        if( next_admin == SESSION::FOCUS_BOARD && ! BOARD::get_admin()->empty() ){
            switch_board();
            break;
        }
        else if( next_admin == SESSION::FOCUS_ARTICLE && ! ARTICLE::get_admin()->empty() ){
            switch_article();
            break;
        }
        else if( next_admin == SESSION::FOCUS_IMAGE && ! IMAGE::get_admin()->empty() ){
            switch_image();
            break;
        }
    }
}



// ブラウザで開く
void Core::open_by_browser( const std::string& url )
{
    std::string command_openurl = CONFIG::get_command_openurl();
    if( !command_openurl.empty() ){

        std::string tmp_url = url;

        // urlの先頭と最後のの " を取る
        while( *tmp_url.c_str() == '\"' ) tmp_url = tmp_url.substr( 1 );
        while( tmp_url.c_str()[ tmp_url.length()-1 ] == '\"' ) tmp_url = tmp_url.substr( 0, tmp_url.length()-1 );

        command_openurl = MISC::replace_str( command_openurl, "%LINK", tmp_url );
        command_openurl = MISC::replace_str( command_openurl, "%s", tmp_url );

#ifdef _DEBUG
        std::cout << "spawn url = " << tmp_url << " command = " << command_openurl << std::endl;
#endif
        Glib::spawn_command_line_async( command_openurl );
    }
}



// history セット
void Core::set_history_article( const std::string& url )
{
    if( m_histmenu_thread ) m_histmenu_thread->append( url, DBTREE::article_subject( url ), TYPE_THREAD );
}


void Core::set_history_board( const std::string& url )
{
    if( m_histmenu_board ) m_histmenu_board->append( url, DBTREE::board_name( url ), TYPE_BOARD );
}


//
// 画像インジケータ表示
//
void Core::show_imagetab()
{
    if( SESSION::get_embedded_img() && ! m_imagetab_shown ){

        // ツールバーの位置がサイドバーの右側の時はツールバーの下に挿入する
        int pos = 0;
        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT && SESSION::get_mode_pane() == SESSION::MODE_2PANE ) pos = 1;

        m_vbox_article.pack_start( IMAGE::get_admin()->tab(), Gtk::PACK_SHRINK );
        m_vbox_article.reorder_child( IMAGE::get_admin()->tab(), pos );

        m_win_main.show_all_children();
        m_imagetab_shown = true;
    }
}


//
// 画像インジケータを隠す
//
void Core::hide_imagetab()
{
    if( m_imagetab_shown ){

        m_vbox_article.remove( IMAGE::get_admin()->tab() );

        m_win_main.show_all_children();
        m_imagetab_shown = false;
    }
}
