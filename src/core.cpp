// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "core.h"
#include "command.h"
#include "winmain.h"
#include "session.h"
#include "global.h"
#include "dndmanager.h"
#include "historymenu.h"
#include "login2ch.h"
#include "prefdiagfactory.h"
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


enum
{
    MODE_2PANE = 0,
    MODE_3PANE,
    MODE_V3PANE
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



Core::Core( WinMain& win_main )
    : m_win_main( win_main ),
      m_imagetab_shown( 0 ),
      m_button_go( Gtk::Stock::JUMP_TO, "移動" ),
      m_focused_admin( FOCUS_NO )
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
}


Core::~Core()
{
#ifdef _DEBUG
    std::cout << "Core::~Core\n";
#endif

    // 設定保存
    CONFIG::save_conf();

    // PANEの敷居の位置保存
    SESSION::set_hpane_main_pos( m_hpaned.get_position() );
    SESSION::set_vpane_main_pos( m_vpaned.get_position() );
    SESSION::set_hpane_main_r_pos( m_hpaned_r.get_position() );

    // メインnotebookのページ番号
    SESSION::set_notebook_main_page( m_notebook.get_current_page() );

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
    m_action_group->add( Gtk::Action::create( "ReloadList", "板リスト再読込"), sigc::mem_fun( *this, &Core::slot_reload_list ) );
    m_action_group->add( Gtk::Action::create( "SaveFavorite", "お気に入り保存"), sigc::mem_fun( *this, &Core::slot_save_favorite ) );
    m_action_group->add( Gtk::Action::create( "Quit", "終了" ), sigc::mem_fun(*this, &Core::slot_quit ) );


    // ログイン
    m_action_group->add( Gtk::Action::create( "Menu_Login", "_Login" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "Login2ch", "2chにログイン", std::string(), false ),
                        sigc::mem_fun( *this, &Core::slot_toggle_login2ch ) );
    m_action_group->add( Gtk::Action::create( "SetupPasswd", "設定" ), sigc::mem_fun( *this, &Core::slot_setup_passwd ) );


    // 表示
    m_action_group->add( Gtk::Action::create( "Menu_View", "表示(_V)" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "Urlbar", "アドレスバー", std::string(), SESSION::show_urlbar() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_urlbar ) );

    // pane 設定
    Gtk::RadioButtonGroup radiogroup;
    Glib::RefPtr< Gtk::RadioAction > raction0 = Gtk::RadioAction::create( radiogroup, "2Pane", "2pane" );
    Glib::RefPtr< Gtk::RadioAction > raction1 = Gtk::RadioAction::create( radiogroup, "3Pane", "3pane" );
    Glib::RefPtr< Gtk::RadioAction > raction2 = Gtk::RadioAction::create( radiogroup, "v3Pane", "縦3pane" );

    switch( SESSION::get_mode_pane() ){
        case MODE_2PANE: raction0->set_active( true ); break;
        case MODE_3PANE: raction1->set_active( true ); break;
        case MODE_V3PANE: raction2->set_active( true ); break;
    }

    m_action_group->add( raction0, sigc::mem_fun( *this, &Core::slot_toggle_2pane ) );
    m_action_group->add( raction1, sigc::mem_fun( *this, &Core::slot_toggle_3pane ) );
    m_action_group->add( raction2, sigc::mem_fun( *this, &Core::slot_toggle_v3pane ) );

    // 設定
    m_action_group->add( Gtk::Action::create( "Menu_Config", "設定(_C)" ) );    
    m_action_group->add( Gtk::ToggleAction::create( "OldArticle", "スレ一覧に過去ログも表示", std::string(), CONFIG::get_show_oldarticle() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_oldarticle ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleTab", "デフォルトでタブで開く", std::string(),
                                                    ! CONFIG::get_buttonconfig()->tab_midbutton()  ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabbutton ) );

    m_action_group->add( Gtk::ToggleAction::create( "RestoreBoard", "起動時にスレ一覧を復元", std::string(), CONFIG::get_restore_board() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_board ) );
    m_action_group->add( Gtk::ToggleAction::create( "RestoreArticle", "起動時にスレッドを復元", std::string(), CONFIG::get_restore_article() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_article ) );
    m_action_group->add( Gtk::ToggleAction::create( "RestoreImage", "起動時に画像を復元", std::string(), CONFIG::get_restore_image() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_image ) );

    m_action_group->add( Gtk::Action::create( "ColorChar", "スレ文字色" ), sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
    m_action_group->add( Gtk::Action::create( "ColorSepa", "新着セパレータ色" ), sigc::mem_fun( *this, &Core::slot_changecolor_separator ) );
    m_action_group->add( Gtk::Action::create( "ColorBack", "スレ背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
    m_action_group->add( Gtk::Action::create( "ColorBackPopup", "ポップアップ背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back_popup ) );
    m_action_group->add( Gtk::Action::create( "ColorBackTree", "ツリー背景色" ), sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );

    m_action_group->add( Gtk::Action::create( "FontTree", "板一覧フォント" ), sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
    m_action_group->add( Gtk::Action::create( "FontTreeBoard", "スレ一覧フォント" ), sigc::mem_fun( *this, &Core::slot_changefont_tree_board ) );
    m_action_group->add( Gtk::Action::create( "FontMenu", "スレフォント" ), sigc::mem_fun( *this, &Core::slot_changefont_main ) );
    m_action_group->add( Gtk::Action::create( "FontPopup", "ポップアップフォント" ), sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
    m_action_group->add( Gtk::Action::create( "SetupProxy", "プロキシ" ), sigc::mem_fun( *this, &Core::slot_setup_proxy ) );
    m_action_group->add( Gtk::Action::create( "SetupBrowser", "Webブラウザ" ), sigc::mem_fun( *this, &Core::slot_setup_browser ) );

    m_action_group->add( Gtk::ToggleAction::create( "UseMosaic", "画像にモザイクをかける", std::string(), CONFIG::get_use_mosaic() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_mosaic ) );
    m_action_group->add( Gtk::Action::create( "DeleteImages", "画像キャッシュクリア" ), sigc::mem_fun( *this, &Core::slot_delete_all_images ) ); 

    // help
    m_action_group->add( Gtk::Action::create( "Menu_Help", "ヘルプ(_H)" ) );    
    m_action_group->add( Gtk::Action::create( "Hp", "ホームページ" ), sigc::mem_fun( *this, &Core::slot_show_hp ) );
    m_action_group->add( Gtk::Action::create( "Bbs", "サポート掲示板" ), sigc::mem_fun( *this, &Core::slot_show_bbs ) );
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
        "<menuitem action='SaveFavorite'/>"
        "<separator/>"
        "<menuitem action='ReloadList'/>"
        "<separator/>"
        "<menuitem action='Quit'/>"
        "</menu>"
/*
        "<menu action='Menu_Login'>"
        "<menuitem action='Login2ch'/>"
        "<menuitem action='SetupPasswd'/>"
        "</menu>"
*/
        "<menu action='Menu_View'>"
        "<menuitem action='Urlbar'/>"
        "<separator/>"
        "<menuitem action='2Pane'/>"
        "<menuitem action='3Pane'/>"
        "<menuitem action='v3Pane'/>"
        "</menu>"

        "<menu action='Menu_Config'>"
        "<menuitem action='OldArticle'/>"
        "<menuitem action='ToggleTab'/>"
        "<separator/>"
        "<menuitem action='RestoreBoard'/>"
        "<menuitem action='RestoreArticle'/>"
        "<menuitem action='RestoreImage'/>"
        "<separator/>"
        "<menuitem action='FontMenu'/>"
        "<menuitem action='FontPopup'/>"
        "<menuitem action='FontTree'/>"
        "<menuitem action='FontTreeBoard'/>"
        "<separator/>"
        "<menuitem action='ColorChar'/>"
//        "<menuitem action='ColorSepa'/>"
        "<menuitem action='ColorBack'/>"
        "<menuitem action='ColorBackPopup'/>"
        "<menuitem action='ColorBackTree'/>"
        "<separator/>"
        "<menuitem action='SetupProxy'/>"
        "<menuitem action='SetupBrowser'/>"
        "<separator/>"
        "<menuitem action='UseMosaic'/>"    
        "<menuitem action='DeleteImages'/>"
        "</menu>"                         


        "<menu action='Menu_Help'>"
        "<menuitem action='Manual'/>"
        "<separator/>"
        "<menuitem action='Hp'/>"
        "<menuitem action='Bbs'/>"
        "<separator/>"
        "<menuitem action='About'/>"
        "</menu>"                         

        "</menubar>"
        "</ui>";
    m_ui_manager->add_ui_from_string( str_ui );
    Gtk::MenuBar* menubar = dynamic_cast< Gtk::MenuBar* >( m_ui_manager->get_widget("/menu_bar") );
    assert( menubar );

    // 履歴メニュー追加
    m_histmenu = Gtk::manage( new HistoryMenu() );
    menubar->items().insert( --(--( menubar->items().end() )), *m_histmenu );


    // 初回起動時の設定
    if( init ){

        Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( "JDセットアップへようこそ\n\nはじめにネットワークの設定をおこなって下さい。" );
        mdiag->run();
        delete mdiag;

        slot_setup_proxy();
        slot_setup_browser();

        mdiag = new Gtk::MessageDialog( "JDセットアップ\n\nスレ、ポップアップ、ツリービューの順にフォントの設定をおこなって下さい。" );
        mdiag->run();
        delete mdiag;

        slot_changefont_main();
        slot_changefont_popup();
        slot_changefont_tree();

        mdiag = new Gtk::MessageDialog( "JDセットアップ\n\nその他の設定は起動後に設定メニューからおこなって下さい" );
        mdiag->run();
        delete mdiag;
        
        mdiag = new Gtk::MessageDialog( "JDセットアップ完了\n\nOKを押すとJDを起動して板のリストをロードします。\nリストが表示されるまでしばらくお待ち下さい。" );
        mdiag->run();
        delete mdiag;
    }

    // 各 widget 作成

    int mode_pane = SESSION::get_mode_pane();

    // 2pane
    if( mode_pane == MODE_2PANE ){

        m_notebook.append_page( BOARD::get_admin()->get_gtknotebook(), "スレ一覧" );
        m_notebook.append_page( ARTICLE::get_admin()->get_gtknotebook(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        m_vbox.pack_start( m_notebook );

        m_hpaned.add1( BBSLIST::get_admin()->get_gtknotebook() );
        m_hpaned.add2( m_vbox );
    }

    // 3ペーン
    else if( mode_pane == MODE_3PANE ){

        m_notebook.append_page( ARTICLE::get_admin()->get_gtknotebook(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        m_vbox.pack_start( m_notebook );

        m_vpaned.add1( BOARD::get_admin()->get_gtknotebook() );
        m_vpaned.add2( m_vbox );

        m_hpaned.add1( BBSLIST::get_admin()->get_gtknotebook() );
        m_hpaned.add2( m_vpaned );
    }

    // 縦3ペーン
    else{

        m_notebook.append_page( ARTICLE::get_admin()->get_gtknotebook(), "スレッド" );
        m_notebook.append_page( IMAGE::get_admin()->view(), "画像" );
        m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );

        m_vbox.pack_start( m_notebook );

        m_hpaned_r.add1( BOARD::get_admin()->get_gtknotebook() );
        m_hpaned_r.add2( m_vbox );

        m_hpaned.add1( BBSLIST::get_admin()->get_gtknotebook() );
        m_hpaned.add2( m_hpaned_r );
    }

    m_hpaned.set_position( SESSION::hpane_main_pos() );
    m_vpaned.set_position( SESSION::vpane_main_pos() );
    m_hpaned_r.set_position( SESSION::hpane_main_r_pos() );

    m_entry_url.signal_activate().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );
    m_button_go.signal_clicked().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );

    m_urlbar.pack_start( m_entry_url );
    m_urlbar.pack_start( m_button_go, Gtk::PACK_SHRINK );

    m_tooltip.set_tip( m_button_go, "移動" );

    std::string str_tmp;
#ifdef USE_GTKMM24
    m_statbar.pack_start( m_mginfo );
#else
    m_statbar.pack_start( m_mginfo, Gtk::PACK_SHRINK );
    m_mginfo.set_width_chars( MAX_MG_LNG * 2 + 16 );
    m_mginfo.set_justify( Gtk::JUSTIFY_LEFT );
#endif
    m_statbar.show_all_children();
    
    // メインwindowのパッキング
    m_vbox_main.set_spacing( 4 );
    m_vbox_main.pack_end( m_statbar, Gtk::PACK_SHRINK );
    m_vbox_main.pack_end( m_hpaned );
    if( SESSION::show_urlbar() ) m_vbox_main.pack_end( m_urlbar, Gtk::PACK_SHRINK );
    m_vbox_main.pack_end( *menubar, Gtk::PACK_SHRINK );

    m_win_main.add( m_vbox_main );
    m_win_main.signal_focus_out_event().connect( sigc::mem_fun(*this, &Core::slot_focus_out_event ) );
    m_win_main.signal_focus_in_event().connect( sigc::mem_fun(*this, &Core::slot_focus_in_event ) );
    m_win_main.show_all_children();
    m_statbar.push( "" );    

    // slot 作って Glib::signal_timeout() にコネクト
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &Core::slot_timeout), 0 );
    sigc::connection conn = Glib::signal_timeout().connect( slot_timeout, TIMER_TIMEOUT );

    // 各管理クラスが開いていたURLを復元
    core_set_command( "restore_views" );

    // タイトル表示
    SESSION::set_online( !SESSION::is_online() ); // slot_toggle_online()で反転する
    slot_toggle_online();
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
    std::string title = std::string( "JD - " ) + std::string( JDVERSIONSTR );
    if( LOGIN::get_login2ch()->login_now() ) title +=" [ ログイン中 ]";
    if( ! SESSION::is_online() ) title += " [ オフライン ]";
    m_win_main.set_title( title );
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
    ss << "バージョン " << JDVERSIONSTR << std::endl << std::endl << JDCOPYRIGHT;
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
}


//
// 2chにログイン
//
void Core::slot_toggle_login2ch()
{
#ifdef _DEBUG
    std::cout << "Core::slot_toggle_login2ch\n";
#endif

    if(  LOGIN::get_login2ch()->login_now() ){
        LOGIN::get_login2ch()->logout();
        set_maintitle();
    }
    else LOGIN::get_login2ch()->start_login();
}



//
// URLバー表示切替え
//
void Core::slot_toggle_urlbar()
{
    SESSION::set_show_urlbar( !SESSION::show_urlbar() );

    if( SESSION::show_urlbar() ){

        Gtk::Widget* menubar = m_ui_manager->get_widget("/menu_bar");
        assert( menubar );

        m_vbox_main.remove( *menubar );
        m_vbox_main.pack_end( m_urlbar, Gtk::PACK_SHRINK );
        m_vbox_main.pack_end( *menubar, Gtk::PACK_SHRINK );
    }
    else m_vbox_main.remove( m_urlbar );

    m_win_main.show_all_children();
}


//
// 2paneモード
//
void Core::slot_toggle_2pane()
{
    if( SESSION::get_mode_pane() == MODE_2PANE ) return;
    SESSION::set_mode_pane( 0 );

    Gtk::MessageDialog mdiag( "JDの再起動後に2paneになります\n\nJDを再起動してください" );
    mdiag.run();
}



//
// 3paneモード
//
void Core::slot_toggle_3pane()
{
    if( SESSION::get_mode_pane() == MODE_3PANE ) return;
    SESSION::set_mode_pane( MODE_3PANE );

    Gtk::MessageDialog mdiag( "JDの再起動後に3paneになります\n\nJDを再起動してください" );
    mdiag.run();
}


//
// 縦3paneモード
//
void Core::slot_toggle_v3pane()
{
    if( SESSION::get_mode_pane() == MODE_V3PANE ) return;
    SESSION::set_mode_pane( MODE_V3PANE );

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

    Gtk::MessageDialog mdiag( "JDの再起動後に有効になります\n\nJDを再起動してください" );
    mdiag.run();
}


//
// 起動時にviewを復元
//
void Core::slot_toggle_restore_board()
{
    CONFIG::set_restore_board( ! CONFIG::get_restore_board() );
}

void Core::slot_toggle_restore_article()
{
    CONFIG::set_restore_article( ! CONFIG::get_restore_article() );
}

void Core::slot_toggle_restore_image()
{
    CONFIG::set_restore_image( ! CONFIG::get_restore_image() );
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
                                           "false", // url 開いてるかチェック
                                           "false", // "true"ならオフラインで開く

                                           "MAIN", // メインモード

                                           command.arg2  // ジャンプ番号( empty ならジャンプしない )
            );

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
                                           "false", // "true"ならオフラインで開く

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
                                           "false", // "true"ならオフラインで開く

                                           "RES", // レス抽出モード
                                     
                                           command.arg1, // レス番号 ( from-to )
                                           command.arg2  // ジャンプ番号( empty ならジャンプしない )
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
                                           "false", // "true"ならオフラインで開く

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
                                           "false", // "true"ならオフラインで開く

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
                                           "false", // "true"ならオフラインで開く

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
                                           "false", // "true"ならオフラインで開く

                                           "REF", // 参照抽出モード
                                     
                                           command.arg1 // 対象レス番号
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

    else if( command.command == "delete_article" ){

        ARTICLE::get_admin()->set_command( "close_view", command.url,
                                           "true" // command.url を含む全てのビューを閉じる
            );

        DBTREE::delete_article( command.url );
        return;
    }


    ////////////////////////////
    // board系のコマンド

    else if( command.command  == "open_board" ){

        BOARD::get_admin()->set_command( "open_view",
                                         command.url,

                                         command.arg1,  // "true" ならtabで開く
                                         "false", // url 開いてるかチェック
                                         "false" // "true"ならオフラインで開く
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

    else if( command.command  == "save_favorite" ){

        BBSLIST::get_admin()->set_command( "save_favorite" );
        return;
    }


    ////////////////////////////
    // image系のコマンド

    else if( command.command == "open_image" ){

        // 画像インジケータ表示
        if( !m_imagetab_shown ){
            m_vbox.pack_start( IMAGE::get_admin()->tab(), Gtk::PACK_SHRINK );
            m_vbox.reorder_child( IMAGE::get_admin()->tab(), 0 );
            m_win_main.show_all_children();
            m_imagetab_shown = true;
        }

        // キャッシュに無かったらロード
        if( ! DBIMG::is_cached( command.url ) ) DBIMG::download_img( command.url );

        IMAGE::get_admin()->set_command( "open_view", command.url );
        return;
    }
    else if( command.command == "delete_image" ){

        IMAGE::get_admin()->set_command( "close_view", command.url );
        DBIMG::delete_cache( command.url );
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

    bool is2pane = ( SESSION::get_mode_pane() == MODE_2PANE );

    // 各管理クラスが開いていたURLを復元
    if( command.command == "restore_views" ){

        // bbslist は無条件でリストア
        // 板一覧がロードされてない時はここでロードされる
        BBSLIST::get_admin()->set_command( "restore" );

        // その他は設定されていたらリストア
        if( CONFIG::get_restore_board() ) BOARD::get_admin()->set_command( "restore" );
        if( CONFIG::get_restore_article() ) ARTICLE::get_admin()->set_command( "restore" );
        if( CONFIG::get_restore_image() && SESSION::image_URLs().size() ){

            // 画像インジケータ表示
            m_vbox.pack_start( IMAGE::get_admin()->tab(), Gtk::PACK_SHRINK );
            m_vbox.reorder_child( IMAGE::get_admin()->tab(), 0 );
            m_win_main.show_all_children();
            m_imagetab_shown = true;

            IMAGE::get_admin()->set_command( "restore" );
        }

        int notebook_page = SESSION::notebook_main_page();
        if( is2pane ){
            if( notebook_page == 0 ) core_set_command( "switch_board" );
            else if( notebook_page == 2 ) core_set_command( "switch_image" );
        }
        else if( notebook_page == 1 ) core_set_command( "switch_image" );
    }

    // 各ビューのタブ幅調整
    else if( command.command == "adjust_tabwidth" ){

        BOARD::get_admin()->set_command( "adjust_tabwidth" );
        ARTICLE::get_admin()->set_command( "adjust_tabwidth" );
    }

    // history 登録
    else if( command.command  == "set_history_article" ) set_history_article( command.url );
        
    else if( command.command  == "set_history_board" ) set_history_board( command.url );
    
    else if( command.command  == "switch_article" ) switch_article();

    else if( command.command  == "switch_board" ) switch_board();

    else if( command.command  == "switch_bbslist" ) switch_bbslist();

    else if( command.command  == "switch_image" ) switch_image();

    else if( command.command  == "toggle_article" ) toggle_article();

    // 2chへのログイン処理が完了した
    else if( command.command  == "login2ch_finished" ){

        // ログインに失敗したらメニューのチェックを外しておく
        if( ! LOGIN::get_login2ch()->login_now() ){

            Glib::RefPtr< Gtk::ToggleAction > tact
            = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( m_action_group->get_action( "Login2ch" ) );
            if( tact && tact->get_active() ){

                LOGIN::get_login2ch()->set_login_now( true ); 
                tact->set_active( false );
            }
        }

        set_maintitle();
    }

    // あるnotebookが空になった
    else if( command.command  == "empty_page" ) {

        FOCUS_OUT_ALL();

        // 画像ビューの場合
        if( command.url == URL_IMAGEADMIN ){

            // 画像インジケータを隠す
            if( m_imagetab_shown ){
                m_vbox.remove( IMAGE::get_admin()->tab() );
                m_win_main.show_all_children();
                m_imagetab_shown = false;
            }
           
            if( is2pane ) m_notebook.set_current_page( 1 );
            else m_notebook.set_current_page( 0 );
            ARTICLE::get_admin()->set_command( "focus_current_view" );
        }

        // articleビューの場合
        else if( command.url == URL_ARTICLEADMIN ){
            if( is2pane ) m_notebook.set_current_page( 0 );
            BOARD::get_admin()->set_command( "focus_current_view" );
        }

        // boardビューの場合
        else if( command.url == URL_BOARDADMIN ) BBSLIST::get_admin()->set_command( "focus_current_view" );
    }

    // URL、ステータスなどの表示
    else if( command.command  == "set_url" ){
        m_entry_url.set_text( command.url );
    }

    else if( command.command  == "set_status" ){
        m_statbar.push( command.arg1 );
    }

    // マウスジェスチャ
    else if( command.command  == "set_mginfo" ){
        m_mginfo.set_text( command.arg1 );
    }

    // bbsmenu再読み込み
    else if( command.command == "reload_bbsmenu" ){
        slot_reload_list();
    }

    // URL のオープン関係

    // 常に外部ブラウザで開く場合
    else if( command.command  == "open_url_browser" ) open_by_browser( command.url );

    // google 検索
    else if( command.command == "search_google" ){

        std::string query = "http://www.google.co.jp/search?hl=ja&q=";
        query += MISC::url_encode( command.arg1.c_str(), strlen( command.arg1.c_str() ) );
        query += "&btnG=Google+%E6%A4%9C%E7%B4%A2&lr=";
       
#ifdef _DEBUG
        std::cout << "exec : search_google = " << query << std::endl;
#endif

        open_by_browser( query );
    }

    // タイプによって判定する場合
    else if( command.command  == "open_url" ){

        command.url = MISC::remove_space( command.url );
        if( command.url.find( "http://" ) != 0 && command.url.find( "https://" ) != 0 ) command.url = "http://" + command.url;

        int num_from, num_to;
        std::string url_dat = DBTREE::url_dat( command.url, num_from, num_to );
        std::string url_subject = DBTREE::url_subject( command.url );
       
        // datの場合ビューで開く
        if( ! url_dat.empty() ){

#ifdef _DEBUG
            std::cout << "exec : open_article url = " << url_dat << std::endl;
#endif
            if( num_from ) CORE::core_set_command( "open_article" , url_dat, "true", MISC::itostr( num_from ) );
            else CORE::core_set_command( "open_article" , url_dat, "true" );
        }

        // 掲示板のベースURLの場合
        else if( ! url_subject.empty() ){

#ifdef _DEBUG
            std::cout << "exec : open_board url = " << url_subject << std::endl;
#endif

            CORE::core_set_command( "open_board" , url_subject, "true" );
        }

        // 画像の場合
        else if( DBIMG::is_loadable( command.url ) ){

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
    
    return true;
}


//
// タブの切替え
//
void Core::slot_switch_page( GtkNotebookPage*, guint page )
{
#ifdef _DEBUG
    std::cout << "Core::slot_switch_page " << page << std::endl;
#endif

    FOCUS_OUT_ALL();

    if( SESSION::get_mode_pane() == MODE_2PANE ){
        if( page == 0 ) BOARD::get_admin()->set_command( "focus_current_view" );
        else if( page == 1 ) ARTICLE::get_admin()->set_command( "focus_current_view" );
        else if( page == 2 ) IMAGE::get_admin()->set_command( "focus_current_view" );
    }
    else{
        if( page == 0 ) ARTICLE::get_admin()->set_command( "focus_current_view" );
        else if( page == 1 ) IMAGE::get_admin()->set_command( "focus_current_view" );
    }
}


//
// フォーカスアウトイベント
//

bool Core::slot_focus_out_event( GdkEventFocus* )
{
    // 現在フォーカスされているビュー番号を保存
    if( BBSLIST::get_admin()->has_focus() ) m_focused_admin = FOCUS_BBSLIST;
    else if( BOARD::get_admin()->has_focus() ) m_focused_admin = FOCUS_BOARD;
    else if( ARTICLE::get_admin()->has_focus() ) m_focused_admin = FOCUS_ARTICLE;
    else if( IMAGE::get_admin()->has_focus() ) m_focused_admin = FOCUS_IMAGE;
    else m_focused_admin = FOCUS_NO;

    FOCUS_OUT_ALL();

#ifdef _DEBUG
    std::cout << "Core::slot_focus_out_event admin = " << m_focused_admin << std::endl;
#endif

    return true;
}


//
// フォーカスインイベント
//
bool Core::slot_focus_in_event( GdkEventFocus* )
{
#ifdef _DEBUG
    std::cout << "Core::slot_focus_in_event admin = " << m_focused_admin << std::endl;
#endif

    // フォーカス状態回復
    switch( m_focused_admin )
    {
        case FOCUS_BBSLIST: BBSLIST::get_admin()->set_command( "restore_focus" ); break;
        case FOCUS_BOARD: BOARD::get_admin()->set_command( "restore_focus" ); break;
        case FOCUS_ARTICLE: ARTICLE::get_admin()->set_command( "restore_focus" ); break;
        case FOCUS_IMAGE: IMAGE::get_admin()->set_command( "restore_focus" ); break;
    }
    m_focused_admin = FOCUS_NO;

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
// 各viewにスイッチ
//
void Core::switch_article()
{
    if( ARTICLE::get_admin()->empty() ) return;

    FOCUS_OUT_ALL();
    ARTICLE::get_admin()->set_command( "delete_popup" );

    m_focused_admin = FOCUS_ARTICLE;
    if( SESSION::get_mode_pane() == MODE_2PANE ) m_notebook.set_current_page( 1 );
    else m_notebook.set_current_page( 0 );
    ARTICLE::get_admin()->set_command( "focus_current_view" );
}


void Core::switch_board()
{
    if( BOARD::get_admin()->empty() ) return;

    FOCUS_OUT_ALL();
    ARTICLE::get_admin()->set_command( "delete_popup" );

    m_focused_admin = FOCUS_BOARD;
    if( SESSION::get_mode_pane() == MODE_2PANE ) m_notebook.set_current_page( 0 );
    BOARD::get_admin()->set_command( "focus_current_view" );
}


void Core::switch_bbslist()
{
    if( BBSLIST::get_admin()->empty() ) return;

    FOCUS_OUT_ALL();
    ARTICLE::get_admin()->set_command( "delete_popup" );
       
    m_focused_admin = FOCUS_BBSLIST;
    BBSLIST::get_admin()->set_command( "focus_current_view" );
}


void Core::switch_image()
{
    if( IMAGE::get_admin()->empty() ) return;

    FOCUS_OUT_ALL();
    ARTICLE::get_admin()->set_command( "delete_popup" );

    m_focused_admin = FOCUS_IMAGE;
    if( SESSION::get_mode_pane() == MODE_2PANE ) m_notebook.set_current_page( 2 );
    else m_notebook.set_current_page( 1 );
    IMAGE::get_admin()->set_command( "focus_current_view" );
}


// 2paneの時にboard <-> article 切替え
void Core::toggle_article()
{
    if( SESSION::get_mode_pane() == MODE_2PANE ){

        if( m_notebook.get_current_page() == 1 ) switch_board();
        else switch_article();
    }
}


// ブラウザで開く
void Core::open_by_browser( const std::string& url )
{
    std::string command_openurl = CONFIG::get_command_openurl();
    if( !command_openurl.empty() ){
        command_openurl = MISC::replace_str( command_openurl, "%s", url );
#ifdef _DEBUG
        std::cout << "spawn url = " << url << " command = " << command_openurl << std::endl;
#endif
        Glib::spawn_command_line_async( command_openurl );
    }
}



// history セット
void Core::set_history_article( const std::string& url )
{
    m_histmenu->append( url, DBTREE::article_subject( url ), TYPE_THREAD );
}


void Core::set_history_board( const std::string& url )
{
    m_histmenu->append( url, DBTREE::board_name( url ), TYPE_BOARD );
}
