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
#include "jdversion.h"

#include "config/globalconf.h"
#include "config/keyconfig.h"
#include "config/mouseconfig.h"
#include "config/buttonconfig.h"

#include "jdlib/miscutil.h"

#include "dbtree/interface.h"
#include "dbimg/imginterface.h"

#include "bbslist/bbslistadmin.h"
#include "board/boardadmin.h"
#include "article/articleadmin.h"
#include "image/imageadmin.h"
#include "message/messageadmin.h"

#include <sstream>

using namespace CORE;


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

    // メインnotebookのページ番号
    SESSION::set_notebook_main_page( m_notebook.get_current_page() );

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

    // ツールバー位置
    Gtk::RadioButtonGroup rg_toolbar;
    raction0 = Gtk::RadioAction::create( rg_toolbar, "ToolbarPos0", "メニューバーの下に表示する" );
    raction1 = Gtk::RadioAction::create( rg_toolbar, "ToolbarPos1", "サイドバーの右に表示する" );

    switch( SESSION::toolbar_pos() ){
        case 0: raction0->set_active( true ); break;
        case 1: raction1->set_active( true ); break;
    }

    m_action_group->add( raction0, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), 0 ) );
    m_action_group->add( raction1, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), 1 ) );

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

    m_action_group->add( Gtk::Action::create( "Color_Menu", "色" ) );
    m_action_group->add( Gtk::Action::create( "ColorChar", "スレ文字色" ), sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
    m_action_group->add( Gtk::Action::create( "ColorSepa", "新着セパレータ色" ), sigc::mem_fun( *this, &Core::slot_changecolor_separator ) );
    m_action_group->add( Gtk::Action::create( "ColorBack", "スレ背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
    m_action_group->add( Gtk::Action::create( "ColorBackPopup", "ポップアップ背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back_popup ) );
    m_action_group->add( Gtk::Action::create( "ColorBackTree", "ツリー背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );


    m_action_group->add( Gtk::Action::create( "Font_Menu", "フォント" ) );
    m_action_group->add( Gtk::Action::create( "FontTree", "板一覧" ), sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
    m_action_group->add( Gtk::Action::create( "FontTreeBoard", "スレ一覧" ), sigc::mem_fun( *this, &Core::slot_changefont_tree_board ) );
    m_action_group->add( Gtk::Action::create( "FontMenu", "スレッド" ), sigc::mem_fun( *this, &Core::slot_changefont_main ) );
    m_action_group->add( Gtk::Action::create( "FontPopup", "ポップアップ" ), sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
    m_action_group->add( Gtk::Action::create( "FontMessage", "書き込みウィンドウ" ), sigc::mem_fun( *this, &Core::slot_changefont_message ) );
    m_action_group->add( Gtk::ToggleAction::create( "StrictCharWidth", "フォント幅の近似計算を厳密におこなう",
                                                    std::string(), CONFIG::get_strict_char_width() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_strict_charwidth ) );


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

        "<menu action='Menu_Login'>"
        "<menuitem action='Login2ch'/>"
        "<menuitem action='SetupPasswd'/>"
        "</menu>"

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
        "</menu>"

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

        "<menu action='Font_Menu'>"
        "<menuitem action='FontMenu'/>"
        "<menuitem action='FontPopup'/>"
        "<menuitem action='FontTree'/>"
        "<menuitem action='FontTreeBoard'/>"
        "<menuitem action='FontMessage'/>"
        "<separator/>"
        "<menuitem action='StrictCharWidth' />"
        "</menu>"

        "<separator/>"

        "<menu action='Color_Menu'>"
        "<menuitem action='ColorChar'/>"
//        "<menuitem action='ColorSepa'/>"
        "<menuitem action='ColorBack'/>"
        "<menuitem action='ColorBackPopup'/>"
        "<menuitem action='ColorBackTree'/>"
        "</menu>"

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
    Gtk::MenuBar* menubar = dynamic_cast< Gtk::MenuBar* >( m_ui_manager->get_widget("/menu_bar") );
    assert( menubar );

    // 板履歴メニュー追加
    m_histmenu_board = Gtk::manage( new HistoryMenuBoard() );
    menubar->items().insert( --(--( menubar->items().end() )), *m_histmenu_board );

    // スレ履歴メニュー追加
    m_histmenu_thread = Gtk::manage( new HistoryMenuThread() );
    menubar->items().insert( --(--( menubar->items().end() )), *m_histmenu_thread );


    // メニューにショートカットキーやマウスジェスチャを表示
    Gtk::Menu_Helpers::MenuList& items = menubar->items();
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

    // 左右ペーン
    int mode_pane = SESSION::get_mode_pane();
    m_notebook.set_show_tabs( false );

    if( mode_pane == SESSION::MODE_2PANE ){ // 2ペーン

        m_notebook.append_page( *BOARD::get_admin()->get_widget(), "スレ一覧" );
        m_notebook.append_page( *ARTICLE::get_admin()->get_widget(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        if( SESSION::toolbar_pos() == 1 ) m_vbox_article.pack_start( m_toolbar, Gtk::PACK_SHRINK );
        m_vbox_article.pack_start( m_notebook );

        m_hpaned.add1( *m_sidebar );
        m_hpaned.add2( m_vbox_article );
    }

    else if( mode_pane == SESSION::MODE_3PANE ){ // 3ペーン

        m_notebook.append_page( *ARTICLE::get_admin()->get_widget(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        m_vbox_article.pack_start( m_notebook );

        if( SESSION::toolbar_pos() == 1 ){

            m_vbox_board.pack_start( m_toolbar, Gtk::PACK_SHRINK );
            m_vbox_board.pack_start( *BOARD::get_admin()->get_widget() );

            m_vpaned_r.add1( m_vbox_board );
            m_vpaned_r.add2( m_vbox_article );
        }
        else{
            m_vpaned_r.add1( *BOARD::get_admin()->get_widget() );
            m_vpaned_r.add2( m_vbox_article );
        }

        m_hpaned.add1( *m_sidebar );
        m_hpaned.add2( m_vpaned_r );
    }

    else if( mode_pane == SESSION::MODE_V3PANE ){ // 縦3ペーン

        m_notebook.append_page( *ARTICLE::get_admin()->get_widget(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        m_vbox_article.pack_start( m_notebook );

        m_hpaned_r.add1( *BOARD::get_admin()->get_widget() );
        m_hpaned_r.add2( m_vbox_article );

        if( SESSION::toolbar_pos() == 1 ){

            m_vbox_articleboard.pack_start( m_toolbar, Gtk::PACK_SHRINK );
            m_vbox_articleboard.pack_start( m_hpaned_r );

            m_hpaned.add1( *m_sidebar );
            m_hpaned.add2( m_vbox_articleboard );
        }
        else{
            m_hpaned.add1( *m_sidebar );
            m_hpaned.add2( m_hpaned_r );
        }
    }

    // ペーンの位置設定
    m_vpaned_r.set_position( SESSION::vpane_main_pos() );
    m_hpaned_r.set_position( SESSION::hpane_main_r_pos() );

    // サイドバーの位置設定
    m_hpaned.set_position( SESSION::hpane_main_pos() );
    m_hpaned.sig_show_hide_leftpane().connect( sigc::mem_fun( *this, &Core::slot_show_hide_leftpane ) );

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
    
    // メインwindowのパッキング
    m_vbox_main.set_spacing( 4 );

    Gtk::ScrolledWindow *scrbar = Gtk::manage( new Gtk::ScrolledWindow() );
    scrbar->add( m_statbar );
    scrbar->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    scrbar->set_size_request( 8 );

    m_vbox_main.pack_end( *scrbar, Gtk::PACK_SHRINK );
    m_vbox_main.pack_end( m_hpaned );
    if( SESSION::toolbar_pos() == 0 ) m_vbox_main.pack_end( m_toolbar, Gtk::PACK_SHRINK );

    m_vbox_main.pack_end( *menubar, Gtk::PACK_SHRINK );

    m_win_main.add( m_vbox_main );
    m_win_main.signal_focus_out_event().connect( sigc::mem_fun(*this, &Core::slot_focus_out_event ) );
    m_win_main.signal_focus_in_event().connect( sigc::mem_fun(*this, &Core::slot_focus_in_event ) );
    m_win_main.show_all_children();

    // 各管理クラスが開いていたURLを復元
    core_set_command( "restore_views" );
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

    show_setupdiag( "JDセットアップ\n\nスレ、ポップアップ、ツリービューの順にフォントの設定をおこなって下さい" );

    slot_changefont_main();

    CONFIG::set_fontname_popup( CONFIG::get_fontname_main() );
    slot_changefont_popup();

    CONFIG::set_fontname_tree( CONFIG::get_fontname_popup() );
    slot_changefont_tree();

    CONFIG::set_fontname_tree_board( CONFIG::get_fontname_tree() );
    CONFIG::set_fontname_message( CONFIG::get_fontname_main() );

    show_setupdiag( "JDセットアップ\n\nその他の設定は起動後に設定メニューからおこなって下さい" );
    show_setupdiag( "JDセットアップ完了\n\nOKを押すとJDを起動して板のリストをロードします\nリストが表示されるまでしばらくお待ち下さい" );

    m_init = false;
}



//
// セットアップ中ダイアログ表示
//
void Core::show_setupdiag( const std::string& msg )
{
    Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( msg );
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
// フォント幅の近似計算を厳密におこなう
//
void Core::slot_toggle_strict_charwidth()
{
    CONFIG::set_strict_char_width( ! CONFIG::get_strict_char_width() );

    if( CONFIG::get_strict_char_width() ){
        Gtk::MessageDialog mdiag( "スレビューのフォント幅の近似を厳密におこないます\n\nパフォーマンスが低下しますので通常は設定しないでください" );
        mdiag.run();
    }

    ARTICLE::get_admin()->set_command( "init_font" );
    ARTICLE::get_admin()->set_command( "relayout_all" );
}



//
// 画像モザイクon/off
//
void Core::slot_toggle_use_mosaic()
{
    CONFIG::set_use_mosaic( ! CONFIG::get_use_mosaic() );

    Gtk::MessageDialog mdiag( "次に開いた画像から有効になります" );
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
// ツリーフォント(板一覧)変更
//
void Core::slot_changefont_tree()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname_tree() );
    diag.set_title( "板一覧フォント" );
    if( diag.run() == Gtk::RESPONSE_OK ){
        CONFIG::set_fontname_tree( diag.get_font_name() );
        BBSLIST::get_admin()->set_command( "relayout_all" );
    }
}


//
// ツリーフォント(スレ一覧)変更
//
void Core::slot_changefont_tree_board()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname_tree_board() );
    diag.set_title( "スレ一覧フォント" );
    if( diag.run() == Gtk::RESPONSE_OK ){
        CONFIG::set_fontname_tree_board( diag.get_font_name() );
        BOARD::get_admin()->set_command( "relayout_all" );
    }
}



//
// メインフォント変更
//
void Core::slot_changefont_main()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname_main() );
    diag.set_title( "スレフォント" );
    if( diag.run() == Gtk::RESPONSE_OK ){
        CONFIG::set_fontname_main( diag.get_font_name() );
        ARTICLE::get_admin()->set_command( "init_font" );
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}


//
// ポップアップフォント変更
//
void Core::slot_changefont_popup()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname_popup() );
    diag.set_title( "ポップアップフォント" );
    if( diag.run() == Gtk::RESPONSE_OK ){
        CONFIG::set_fontname_popup( diag.get_font_name() );
        ARTICLE::get_admin()->set_command( "init_font" );
    }
}


//
// 書き込みフォント変更
//
void Core::slot_changefont_message()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname_message() );
    diag.set_title( "書き込みウィンドウフォント" );
    if( diag.run() == Gtk::RESPONSE_OK ){
        CONFIG::set_fontname_message( diag.get_font_name() );
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }
}


//
// ツリー背景色変更
//
void Core::slot_changecolor_back_tree()
{
    int rgb[ 3 ];
    if( open_color_diag( "ツリー背景色", CONFIG::get_color_back_tree(), rgb ) ){
        CONFIG::set_color_back_tree( rgb );
        CONFIG::set_color_back_tree_board( rgb );
        BBSLIST::get_admin()->set_command( "relayout_all" );
        BOARD::get_admin()->set_command( "relayout_all" );
    }
}


//
// 背景色変更
//
void Core::slot_changecolor_back()
{
    int rgb[ 3 ];
    if( open_color_diag( "スレ背景", CONFIG::get_color_back(), rgb ) ){
        CONFIG::set_color_back( rgb );
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}


//
// 文字色変更
//
void Core::slot_changecolor_char()
{
    int rgb[ 3 ];
    if( open_color_diag( "文字色", CONFIG::get_color_char(), rgb ) ){
        CONFIG::set_color_char( rgb );
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}


//
// 新着セパレータ色変更
//
void Core::slot_changecolor_separator()
{
    int rgb[ 3 ];
    if( open_color_diag( "新着セパレータ", CONFIG::get_color_separator(), rgb ) ){
        CONFIG::set_color_separator( rgb );
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}



//
// ポップアップ背景色変更
//
void Core::slot_changecolor_back_popup()
{
    int rgb[ 3 ];
    if( open_color_diag( "ポップアップ背景", CONFIG::get_color_back_popup(), rgb ) ){
        CONFIG::set_color_back_popup( rgb );
        ARTICLE::get_admin()->set_command( "relayout_all" );
    }
}



//
// 色選択ダイアログを開く
//
bool Core::open_color_diag( std::string title, const int* rgb, int* rgb_out )
{
    Gdk::Color color;
    color.set_rgb( rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );

    Gtk::ColorSelectionDialog diag( title );
    diag.get_colorsel()->set_current_color( color );
    if( diag.run() == Gtk::RESPONSE_OK ){

        color = diag.get_colorsel()->get_current_color();
        rgb_out[ 0 ] = color.get_red();
        rgb_out[ 1 ] = color.get_green();
        rgb_out[ 2 ] = color.get_blue();

        return true;
    }

    return false;
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
    open_by_browser( std::string( JDURL ) + "README.txt" );
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
    Gtk::MessageDialog mdiag( ss.str() );
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
    SESSION::set_toolbar_pos( pos );

    Gtk::MessageDialog mdiag( "JDの再起動後に位置が変わります\n\nJDを再起動してください" );
    mdiag.run();
}


//
// 2paneモード
//
void Core::slot_toggle_2pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ) return;
    SESSION::set_mode_pane( 0 );

    Gtk::MessageDialog mdiag( "JDの再起動後に2paneになります\n\nJDを再起動してください" );
    mdiag.run();
}



//
// 3paneモード
//
void Core::slot_toggle_3pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_3PANE ) return;
    SESSION::set_mode_pane( SESSION::MODE_3PANE );

    Gtk::MessageDialog mdiag( "JDの再起動後に3paneになります\n\nJDを再起動してください" );
    mdiag.run();
}


//
// 縦3paneモード
//
void Core::slot_toggle_v3pane()
{
    if( SESSION::get_mode_pane() == SESSION::MODE_V3PANE ) return;
    SESSION::set_mode_pane( SESSION::MODE_V3PANE );

    Gtk::MessageDialog mdiag( "JDの再起動後に縦3paneになります\n\nJDを再起動してください" );
    mdiag.run();
}




//
// 過去ログ表示切替え
//
void Core::slot_toggle_oldarticle()
{
    CONFIG::set_show_oldarticle( ! CONFIG::get_show_oldarticle() );

    Gtk::MessageDialog mdiag( "次に開いた板から有効になります" );
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
   

    ////////////////////////////
    // bbslist系のコマンド

    else if( command.command  == "update_bbslist" ){

        BBSLIST::get_admin()->set_command( "update_view", URL_BBSLISTVIEW );
        return;
    }
    else if( command.command  == "append_favorite" ){

        BBSLIST::get_admin()->set_command( "append_favorite", command.url, command.arg1, command.arg2 );
        return;
    }
    else if( command.command  == "update_favorite" ){

        BBSLIST::get_admin()->set_command( "update_view", URL_FAVORITEVIEW );
        return;
    }

    else if( command.command  == "save_favorite" ){

        BBSLIST::get_admin()->set_command( "save_favorite" );
        return;
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
        if( SESSION::focused_admin() == SESSION::FOCUS_IMAGE ) IMAGE::get_admin()->set_command( "restore_focus" );

        IMAGE::get_admin()->set_command( "close_view", command.url );
        return;
    }

    ////////////////////////////
    // message系

    else if( command.command == "open_message" ){

        if( ! SESSION::is_online() ){
            Gtk::MessageDialog mdiag( "オフラインです" );
            mdiag.run();
        }
        else MESSAGE::get_admin()->set_command( "open_view", command.url, command.arg1 );
    }

    else if( command.command == "create_new_thread" ){

        if( ! SESSION::is_online() ){
            Gtk::MessageDialog mdiag( "オフラインです" );
            mdiag.run();
        }
        else if( DBTREE::url_bbscgi_new( command.url ).empty() ){
            Gtk::MessageDialog mdiag( "この板では新スレを立てることは出来ません" );
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
                Gtk::MessageDialog mdiag( "オフラインです" );
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

        // coreと全てのadminクラスのコマンドの実行が終わった
        if( m_list_command.size() == 0
            && ! BBSLIST::get_admin()->has_commands()
            && ! BOARD::get_admin()->has_commands()
            && ! ARTICLE::get_admin()->has_commands()
            && ! IMAGE::get_admin()->has_commands() ){

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

    int admin = SESSION::focused_admin();
    int admin_sidebar = SESSION::focused_admin_sidebar();
    SESSION::set_focused_admin( SESSION::FOCUS_NO );
    SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );

    // adminの表示状態回復
    m_notebook.set_current_page( SESSION::notebook_main_page() );

    // フォーカス状態回復
    switch( admin ){
        case SESSION::FOCUS_SIDEBAR: switch_sidebar(); break;
        case SESSION::FOCUS_BOARD: switch_board(); break;
        case SESSION::FOCUS_ARTICLE: switch_article(); break;
        case SESSION::FOCUS_IMAGE: switch_image(); break;
    }
    SESSION::set_focused_admin( admin );
    SESSION::set_focused_admin_sidebar( admin_sidebar );

    // サイドバー表示状態変更
    if( ! SESSION::show_sidebar() ) m_hpaned.show_hide_leftpane();

    // ツールバー表示切り替え
    if( ! SESSION::show_toolbar() ) m_vbox_main.remove( m_toolbar );

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
    
    return true;
}


//
// 右ペーンのnotebookのタブの切替え
//
void Core::slot_switch_page( GtkNotebookPage*, guint page )
{
#ifdef _DEBUG
    std::cout << "Core::slot_switch_page " << page << std::endl;
#endif

    if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

        switch( page){

            case 0: switch_board(); break;

            case 1: switch_article(); break;

            case 2: switch_image(); break;
        }
    }
    else{

        switch( page ){

            case 0: switch_article(); break;

            case 1: switch_image(); break;
        }
    }
}


//
// フォーカスアウトイベント
//
bool Core::slot_focus_out_event( GdkEventFocus* )
{
    FOCUS_OUT_ALL();

#ifdef _DEBUG
    std::cout << "Core::slot_focus_out_event admin = " << SESSION::focused_admin() << std::endl;
#endif

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

    // フォーカス状態回復
    switch( SESSION::focused_admin() )
    {
        case SESSION::FOCUS_SIDEBAR: BBSLIST::get_admin()->set_command( "restore_focus" ); break;
        case SESSION::FOCUS_BOARD: BOARD::get_admin()->set_command( "restore_focus" ); break;
        case SESSION::FOCUS_ARTICLE: ARTICLE::get_admin()->set_command( "restore_focus" ); break;
        case SESSION::FOCUS_IMAGE: IMAGE::get_admin()->set_command( "restore_focus" ); break;
    }

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

    // 画像ビューが空になった
    if( url == URL_IMAGEADMIN ){

        hide_imagetab();

        // 空でないadminを前に出す
        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

            if( m_notebook.get_current_page() == 2 ){
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

            if( m_notebook.get_current_page() == 1 ){
                if( BOARD::get_admin()->empty() && ! IMAGE::get_admin()->empty() ) switch_image();
                else if( ! BOARD::get_admin()->empty() ) switch_board();
            }
        }
        else if( ! IMAGE::get_admin()->empty() ) switch_image();

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

            if( m_notebook.get_current_page() == 0 ){
                if( ! ARTICLE::get_admin()->empty() ) switch_article();
                else if( ! IMAGE::get_admin()->empty() ) switch_image();
            }
        }

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){
            focused_admin = SESSION::FOCUS_SIDEBAR;
            SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
        }
    }

    // 切り替え実行
    switch( focused_admin ){

        case SESSION::FOCUS_SIDEBAR: switch_sidebar(); break;
        case SESSION::FOCUS_BOARD: switch_board(); break;
        case SESSION::FOCUS_ARTICLE: switch_article(); break;
        case SESSION::FOCUS_IMAGE: switch_image(); break;
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
    }

    m_enable_menuslot = true;
}


//
// ビューのトグルボタンのアクティブ状態を切り替える
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
    if( IMAGE::get_admin()->empty() ) m_button_image.set_sensitive( false );
    else m_button_image.set_sensitive( true );

    m_enable_menuslot = true;
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

            if( SESSION::get_mode_pane() == SESSION::MODE_2PANE && m_notebook.get_current_page() != 1 ) m_notebook.set_current_page( 1 );
            else if( SESSION::get_mode_pane() != SESSION::MODE_2PANE && m_notebook.get_current_page() != 0 ) m_notebook.set_current_page( 0 );
        }

        ARTICLE::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_ARTICLE );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_ARTICLE );

        SESSION::set_img_shown( false );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
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

            if( SESSION::get_mode_pane() == SESSION::MODE_2PANE && m_notebook.get_current_page() != 0 ) m_notebook.set_current_page( 0 );
        }

        BOARD::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_BOARD );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_BOARD );

        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ) SESSION::set_img_shown( false );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
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
}


void Core::switch_image()
{
    if( m_boot ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_image\n";
#endif

    if( ! IMAGE::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_IMAGE ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );

            if( SESSION::get_mode_pane() == SESSION::MODE_2PANE && m_notebook.get_current_page() != 2 ) m_notebook.set_current_page( 2 );
            else if( SESSION::get_mode_pane() != SESSION::MODE_2PANE && m_notebook.get_current_page() != 1 ) m_notebook.set_current_page( 1 );

            // 画像強制表示
            IMAGE::get_admin()->set_command( "show_image" );
        }

        IMAGE::get_admin()->set_command( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_IMAGE );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_IMAGE );

        SESSION::set_img_shown( true );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
}


// 2paneの時にboard <-> article 切替え
void Core::toggle_article()
{
    if( SESSION::focused_admin() == SESSION::FOCUS_ARTICLE ) switch_board();
    else switch_article();
}


// 左移動
void Core::switch_leftview()
{
    int next_admin = SESSION::focused_admin() -1;
    
    while( next_admin >= SESSION::FOCUS_SIDEBAR ){

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

        --next_admin;
    }
}


// 右移動
void Core::switch_rightview()
{
    int next_admin = SESSION::focused_admin() +1;
    
    while( next_admin <= SESSION::FOCUS_IMAGE ){

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

        ++next_admin;
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
    if( ! m_imagetab_shown ){

        int pos = 0;
        if( SESSION::toolbar_pos() == 1 && SESSION::get_mode_pane() == SESSION::MODE_2PANE ) pos = 1;
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
