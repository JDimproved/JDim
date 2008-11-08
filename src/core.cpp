
// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "core.h"
#include "maintoolbar.h"
#include "command.h"
#include "winmain.h"
#include "session.h"
#include "global.h"
#include "type.h"
#include "dndmanager.h"
#include "usrcmdmanager.h"
#include "linkfiltermanager.h"
#include "compmanager.h"
#include "searchmanager.h"
#include "aamanager.h"
#include "dispatchmanager.h"
#include "cssmanager.h"
#include "updatemanager.h"
#include "login2ch.h"
#include "loginbe.h"
#include "prefdiagfactory.h"
#include "colorid.h"
#include "fontid.h"
#include "jdversion.h"
#include "setupwizard.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "history/historymanager.h"

#include "skeleton/msgdiag.h"
#include "skeleton/aboutdiag.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "dbtree/interface.h"
#include "dbimg/imginterface.h"

#include "bbslist/bbslistadmin.h"
#include "board/boardadmin.h"
#include "article/articleadmin.h"
#include "image/imageadmin.h"
#include "message/messageadmin.h"

#include "icons/iconmanager.h"

#include "message/logmanager.h"

#include "sound/soundmanager.h"

#include <sstream>

using namespace CORE;

enum
{
    PAGE_ARTICLE = 0,
    PAGE_IMAGE,
    PAGE_BOARD
};


enum
{
    IMGVIEW_WINDOW = 0,
    IMGVIEW_EMB,
    IMGVIEW_NO
};


Core* instance_core;


Core* CORE::get_instance()
{
    return instance_core;
}


// 全ビューをフォーカスアウト
#define FOCUS_OUT_ALL() do{ \
ARTICLE::get_admin()->set_command_immediately( "focus_out" ); \
BOARD::get_admin()->set_command_immediately( "focus_out" ); \
BBSLIST::get_admin()->set_command_immediately( "focus_out" ); \
IMAGE::get_admin()->set_command_immediately( "focus_out" ); \
MESSAGE::get_admin()->set_command_immediately( "focus_out" ); \
}while(0)


//////////////////////////////////////////////////////


Core::Core( WinMain& win_main )
    : m_win_main( win_main ),
      m_hpaned( SKELETON::PANE_FIXSIZE_PAGE1 ),
      m_vpaned_r( SKELETON::PANE_FIXSIZE_PAGE1 ),
      m_hpaned_r( SKELETON::PANE_FIXSIZE_PAGE1 ),
      m_imagetab_shown( 0 ),
      m_vpaned_message( SKELETON::PANE_FIXSIZE_PAGE2 ),
      m_toolbar( NULL ),
      m_enable_menuslot( true ),
      m_init( false )
{
    // ディスパッチマネージャ作成
    CORE::get_dispmanager();

    instance_core = this;

    // データベースのルート作成
    DBTREE::create_root();
    DBIMG::create_root();

    // 2chログインマネージャ作成
    CORE::get_login2ch();

    // BEログインマネージャ作成
    CORE::get_loginbe();

    // マウス、キー設定読み込み
    CONTROL::load_conf();

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

    // リンクフィルタマネージャ作成
    CORE::get_linkfilter_manager();

    // ログ検索マネージャ作成
    CORE::get_search_manager();

    m_vbox_article.signal_realize().connect( sigc::mem_fun(*this, &Core::slot_realize ) );
    m_vbox_article.signal_style_changed().connect( sigc::mem_fun(*this, &Core::slot_style_changed ) );
}


Core::~Core()
{
#ifdef _DEBUG
    std::cout << "Core::~Core\n";
#endif

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    SESSION::set_quitting( true );

    // 削除リストに登録されているスレを削除
    std::vector< std::string >& dellist =  SESSION::get_delete_list();
    if( dellist.size() ){
        std::vector< std::string >::iterator it = dellist.begin();
        for( ; it != dellist.end(); ++it ){

            // しおりが付いている場合は削除しない
            if( ! DBTREE::is_bookmarked_thread( *it ) && ! DBTREE::get_num_bookmark( *it ) ){
                DBTREE::delete_article( *it, false );
                ARTICLE::get_admin()->set_command_immediately( "unlock_views", *it );
                ARTICLE::get_admin()->set_command_immediately( "close_view", *it, 
                                                               "closeall" // command.url を含む全てのビューを閉じる
                    );
            }
        }
    }

    // 設定保存
    // セッション情報は WinMain::~WinMain() で保存する
    CONFIG::save_conf();
    CONFIG::delete_confitem();

    // PANEの敷居の位置保存
    SESSION::set_hpane_main_pos( m_hpaned.get_ctrl().get_position() );
    SESSION::set_vpane_main_pos( m_vpaned_r.get_ctrl().get_position() );
    SESSION::set_hpane_main_r_pos( m_hpaned_r.get_ctrl().get_position() );
    SESSION::set_vpane_main_mes_pos( m_vpaned_message.get_ctrl().get_position() );

    // ログ検索マネージャ削除
    CORE::delete_search_manager();

    // ユーザコマンドマネージャ削除
    CORE::delete_usrcmd_manager();

    // リンクフィルタマネージャ削除
    CORE::delete_linkfilter_manager();

    // 補完マネージャ削除
    CORE::delete_completion_manager();

    // D&Dマネージャ削除
    CORE::delete_dnd_manager();

    // AA マネージャ削除
    CORE::delete_aamanager();

    // 更新チェックマネージャ削除
    CORE::delete_checkupdate_manager();

    // マウス、キーコンフィグ保存
    CONTROL::save_conf();
    CONTROL::delete_conf();

    // ビューを削除する前にswitch_pageをdisconnectしておかないとエラーが出る
    if( m_sigc_switch_page.connected() ) m_sigc_switch_page.disconnect(); 

    // 各管理クラスを削除
    BBSLIST::delete_admin();
    BOARD::delete_admin();
    ARTICLE::delete_admin();
    IMAGE::delete_admin();
    MESSAGE::delete_admin();

    // 履歴マネージャ削除
    // 内部で SESSION::get_*_URLs() を使用しているので
    // ARTICLEやBOARDなどの管理クラスを削除した後で削除すること
    HISTORY::delete_history_manager();

    // cssマネージャ削除
    CORE::delete_css_manager();

    // 2chログインマネージャ削除
    CORE::delete_login2ch();

    // BEログインマネージャ削除
    CORE::delete_loginbe();

    // データベース削除
    DBTREE::delete_root();
    DBIMG::delete_root();

    // サウンドマネージャ削除
    SOUND::delete_sound_manager();

    // 書き込みログマネージャ削除
    MESSAGE::delete_log_manager();

    // ディスパッチマネージャ削除
    CORE::delete_dispatchmanager();

    // ツールバー削除
    if( m_toolbar ) delete m_toolbar;
}


// 右ペーンのnotebookのparentであるvboxがrealizeしたらnotebookのstyleを変更する
// テーマによっては notebook の中に notebook を配置すると背景色が正しく
// 出ない問題があるため。開発スレ 493 参照
void Core::slot_realize()
{
#ifdef _DEBUG
    std::cout << "Core::slot_realize\n";
#endif

    slot_style_changed( m_vbox_article.get_style() );
}


void Core::slot_style_changed( Glib::RefPtr< Gtk::Style > )
{
    m_notebook_right.set_style( m_vbox_article.get_style() );
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
    m_action_group->add( Gtk::ToggleAction::create( "Online", "オンライン(_O)", std::string(), SESSION::is_online() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_online ) );
    m_action_group->add( Gtk::ToggleAction::create( "Login2ch", "2chにログイン(_L)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_login2ch ) );
    m_action_group->add( Gtk::ToggleAction::create( "LoginBe", "BEにログイン(_B)", std::string(), false ),
                        sigc::mem_fun( *this, &Core::slot_toggle_loginbe ) );
    m_action_group->add( Gtk::ToggleAction::create( "LoginP2", "p2経由で書き込み(_P)", std::string(), false ),
                        sigc::mem_fun( *this, &Core::slot_toggle_loginp2 ) );
    m_action_group->add( Gtk::Action::create( "ReloadList", "板一覧再読込(_R)"), sigc::mem_fun( *this, &Core::slot_reload_list ) );

    m_action_group->add( Gtk::Action::create( "SaveFavorite", "お気に入り保存(_S)"), sigc::mem_fun( *this, &Core::slot_save_favorite ) );
    m_action_group->add( Gtk::Action::create( "Quit", "終了(_Q)" ),
                         Gtk::AccelKey( "<Ctrl>Q" ), sigc::mem_fun(*this, &Core::slot_quit ) );

    //////////////////////////////////////////////////////

    // 表示
    m_action_group->add( Gtk::Action::create( "Menu_View", "表示(_V)" ) );    

    m_action_group->add( Gtk::Action::create( "Show_Board", "スレ一覧(_B)" ),
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_board ), false ) );
    m_action_group->add( Gtk::Action::create( "Show_Thread", "スレビュー(_T)" ),
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_article ), false ) );
    m_action_group->add( Gtk::Action::create( "Show_Image", "画像ビュー(_I)" ), 
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_image ), false ) );

    // サイドバー
    m_action_group->add( Gtk::Action::create( "Sidebar_Menu", "サイドバー(_S)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_BBS", "板一覧(_B)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_BBSLISTVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_FAVORITE", "お気に入り(_F)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_FAVORITEVIEW, false ) );

    m_action_group->add( Gtk::Action::create( "View_Menu", "詳細設定(_D)" ) );

    // 一般
    m_action_group->add( Gtk::ToggleAction::create( "ShowMenuBar", "メニューバー表示(_S)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_menubar ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToggleFlatButton", "ボタンをフラット表示(_F)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_flat_button ) );
    m_action_group->add( Gtk::ToggleAction::create( "TogglePostMark", "自分が書き込んだレスにマークをつける(_W)",
                                                    std::string(), CONFIG::get_show_post_mark() ),
                         sigc::mem_fun( *this, &Core::toggle_post_mark ) );

    // ツールバー表示
    m_action_group->add( Gtk::Action::create( "Toolbar_Menu", "ツールバー表示(_T)" ) );
    m_action_group->add( Gtk::Action::create( "Toolbar_Main_Menu", "メイン(_M)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToolbarPos0", "メニューバーの下に表示する(_U)", std::string(), false ),
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_NORMAL ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToolbarPos1", "サイドバーの右に表示する(_R)", std::string(), false ),
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_RIGHT ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToolbarBbslist", "サイドバー(_S)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbarbbslist ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToolbarBoard", "スレ一覧(_B)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbarboard ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToolbarArticle", "スレビュー(_A)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbararticle ) );

    // タブ表示
    m_action_group->add( Gtk::Action::create( "Tab_Menu", "タブ表示(_B)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "TabBoard", "スレ一覧(_B)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabboard ) );
    m_action_group->add( Gtk::ToggleAction::create( "TabArticle", "スレビュー(_A)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabarticle ) );

    // pane 設定
    Gtk::RadioButtonGroup radiogroup;
    Glib::RefPtr< Gtk::RadioAction > raction0 = Gtk::RadioAction::create( radiogroup, "2Pane", "２ペイン表示(_2)" );
    Glib::RefPtr< Gtk::RadioAction > raction1 = Gtk::RadioAction::create( radiogroup, "3Pane", "３ペイン表示(_3)" );
    Glib::RefPtr< Gtk::RadioAction > raction2 = Gtk::RadioAction::create( radiogroup, "v3Pane", "縦３ペイン表示(_V)" );

    switch( SESSION::get_mode_pane() ){
        case SESSION::MODE_2PANE: raction0->set_active( true ); break;
        case SESSION::MODE_3PANE: raction1->set_active( true ); break;
        case SESSION::MODE_V3PANE: raction2->set_active( true ); break;
    }

    m_action_group->add( raction0, sigc::mem_fun( *this, &Core::slot_toggle_2pane ) );
    m_action_group->add( raction1, sigc::mem_fun( *this, &Core::slot_toggle_3pane ) );
    m_action_group->add( raction2, sigc::mem_fun( *this, &Core::slot_toggle_v3pane ) );

    // 書き込みビュー
    Gtk::RadioButtonGroup radiogroup_msg;
    m_action_group->add( Gtk::Action::create( "ShowMsgView_Menu", "書き込みビュー(_M)" ) );
    Glib::RefPtr< Gtk::RadioAction > raction_msg0 = Gtk::RadioAction::create( radiogroup_msg, "UseWinMsg", "ウィンドウ表示する(_W)" );
    Glib::RefPtr< Gtk::RadioAction > raction_msg1 = Gtk::RadioAction::create( radiogroup_msg, "UseEmbMsg", "埋め込み表示する(_E)" );

    if( ! SESSION::get_embedded_mes() ) raction_msg0->set_active( true );
    else raction_msg1->set_active( true );

    m_action_group->add( raction_msg0, sigc::mem_fun( *this, &Core::slot_toggle_winmsg ) );
    m_action_group->add( raction_msg1, sigc::mem_fun( *this, &Core::slot_toggle_embmsg ) );

    // 画像表示設定
    m_action_group->add( Gtk::Action::create( "ImageView_Menu", "画像表示設定(_G)" ) );
    m_action_group->add( Gtk::Action::create( "ShowImageView_Menu", "画像ビュー(_V)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseWinImg", "ウィンドウ表示する(_W)", std::string(), false ),
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_WINDOW ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseEmbImg", "埋め込み表示する(_E)", std::string(), false ),
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_EMB ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseImgPopup", "画像ポップアップを表示する(_P)", std::string(), CONFIG::get_use_image_popup() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_imgpopup ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseInlineImg", "インライン画像を表示する(_I)", std::string(), CONFIG::get_use_inline_image() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_inlineimg ) );
    m_action_group->add( Gtk::ToggleAction::create( "ShowSsspIcon", "BEアイコンを表示する(_B)", std::string(), CONFIG::get_show_ssspicon() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_show_ssspicon ) );

    // リスト表示項目設定
    m_action_group->add( Gtk::Action::create( "ListItem_Menu", "リスト項目設定(_L)" ) );
    m_action_group->add( Gtk::Action::create( "SetupBoardItemColumn", "スレ一覧(_T)..." ), sigc::mem_fun( *this, &Core::slot_setup_boarditemcolumn ) );

    // ツールバー項目設定
    m_action_group->add( Gtk::Action::create( "Item_Menu", "ツールバー項目設定(_I)" ) );
    m_action_group->add( Gtk::Action::create( "SetupMainItem", "メイン(_M)..." ), sigc::mem_fun( *this, &Core::slot_setup_mainitem ) );
    m_action_group->add( Gtk::Action::create( "SetupSidebarItem", "サイドバー(_S)..." ), sigc::mem_fun( *this, &Core::slot_setup_sidebaritem ) );
    m_action_group->add( Gtk::Action::create( "SetupBoardItem", "スレ一覧(_B)..." ), sigc::mem_fun( *this, &Core::slot_setup_boarditem ) );
    m_action_group->add( Gtk::Action::create( "SetupArticleItem", "スレビュー(_A)..." ), sigc::mem_fun( *this, &Core::slot_setup_articleitem ) );
    m_action_group->add( Gtk::Action::create( "SetupMsgItem", "書き込みビュー(_W)..." ), sigc::mem_fun( *this, &Core::slot_setup_msgitem ) );

    //////////////////////////////////////////////////////

    // 設定
    m_action_group->add( Gtk::Action::create( "Menu_Config", "設定(_C)" ) );    

    m_action_group->add( Gtk::Action::create( "Property_Menu", "プロパティ(_P)" ) );
    m_action_group->add( Gtk::Action::create( "BoardPref", "表示中の板のプロパティ(_B)..." ), sigc::mem_fun( *this, &Core::slot_board_pref ) );
    m_action_group->add( Gtk::Action::create( "ArticlePref", "表示中のスレのプロパティ(_T)..." ), sigc::mem_fun( *this, &Core::slot_article_pref ) );
    m_action_group->add( Gtk::Action::create( "ImagePref", "表示中の画像のプロパティ(_I)..." ), sigc::mem_fun( *this, &Core::slot_image_pref ) );

    // 一般
    m_action_group->add( Gtk::Action::create( "General_Menu", "一般(_G)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "OldArticle", "スレ一覧に過去ログも表示する(_S)", std::string(), CONFIG::get_show_oldarticle() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_oldarticle ) );

    m_action_group->add( Gtk::ToggleAction::create( "RestoreViews", "前回開いていた各ビューを起動時に復元する(_R)", std::string(),
                                                    ( CONFIG::get_restore_board()
                                                      & CONFIG::get_restore_article()
                                                      & CONFIG::get_restore_image() ) ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_views ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleFoldMessage", "非アクティブ時に書き込みビューを折りたたむ(_C)", std::string(),
                                                    CONFIG::get_fold_message() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_fold_message ) );

    m_action_group->add( Gtk::ToggleAction::create( "SavePostLog", "書き込みログを保存する(_A)", std::string(), CONFIG::get_save_post_log() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_save_post_log ) );
    m_action_group->add( Gtk::ToggleAction::create( "SavePostHist", "書き込み履歴(鉛筆マーク)を保存する(_P)", std::string(), CONFIG::get_save_post_history() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_save_post_history ) );

    m_action_group->add( Gtk::ToggleAction::create( "UseMosaic", "画像にモザイクをかける(_M)", std::string(), CONFIG::get_use_mosaic() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_mosaic ) );

    // マウス／キーボード
    m_action_group->add( Gtk::Action::create( "Mouse_Menu", "マウス／キーボード(_M)" ) );

    bool toggled = CONTROL::is_toggled_tab_button() && CONTROL::is_toggled_tab_key();
    m_action_group->add( Gtk::ToggleAction::create( "ToggleTab", "スレ一覧／スレビューを開く時に常に新しいタブで開く(_T)", std::string(), toggled ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabbutton ) );

    m_action_group->add( Gtk::ToggleAction::create( "TogglePopupWarp", "スレビューでアンカーをクリックして多重ポップアップモードに移行する(_W)", std::string(),
                                                    CONTROL::is_popup_warpmode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_popupwarpmode ) );

    m_action_group->add( Gtk::ToggleAction::create( "ShortMarginPopup", "スレビューでカーソルを移動して多重ポップアップモードに移行する(_M)", std::string(),
                                                    ( CONFIG::get_margin_popup() != CONFIG::CONF_MARGIN_POPUP ) ),
                         sigc::mem_fun( *this, &Core::slot_shortmargin_popup ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleEmacsMode", "書き込みビューのショートカットキーをEmacs風にする(_E)", std::string(),
                                                    CONTROL::is_emacs_mode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_emacsmode ) );

    m_action_group->add( Gtk::Action::create( "MousePref", "マウスジェスチャ詳細設定(_G)..." ), sigc::mem_fun( *this, &Core::slot_setup_mouse ) );
    m_action_group->add( Gtk::Action::create( "KeyPref", "ショートカットキー詳細設定(_R)..." ), sigc::mem_fun( *this, &Core::slot_setup_key ) );
    m_action_group->add( Gtk::Action::create( "ButtonPref", "マウスボタン詳細設定(_B)..." ), sigc::mem_fun( *this, &Core::slot_setup_button ) );

    // フォントと色
    m_action_group->add( Gtk::Action::create( "FontColor_Menu", "フォントと色(_F)" ) );

    m_action_group->add( Gtk::Action::create( "FontMain", "スレビューフォント(_T)..." ), sigc::mem_fun( *this, &Core::slot_changefont_main ) );
    m_action_group->add( Gtk::Action::create( "FontPopup", "ポップアップフォント(_P)..." ), sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
    m_action_group->add( Gtk::Action::create( "FontTree", "板／スレ一覧フォント(_B)..." ), sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
    m_action_group->add( Gtk::Action::create( "ColorChar", "スレビュー文字色(_C)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
    m_action_group->add( Gtk::Action::create( "ColorBack", "スレビュー背景色(_A)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
    m_action_group->add( Gtk::Action::create( "ColorCharTree", "板／スレ一覧文字色(_H)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_char_tree ) );
    m_action_group->add( Gtk::Action::create( "ColorBackTree", "板／スレ一覧背景色(_K)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );
    m_action_group->add( Gtk::Action::create( "FontColorPref", "詳細設定(_R)..." ), sigc::mem_fun( *this, &Core::slot_setup_fontcolor ) );

    // ネットワーク
    m_action_group->add( Gtk::Action::create( "Net_Menu", "ネットワーク(_N)" ) );
    m_action_group->add( Gtk::Action::create( "SetupProxy", "プロキシ(_X)..." ), sigc::mem_fun( *this, &Core::slot_setup_proxy ) );
    m_action_group->add( Gtk::Action::create( "SetupBrowser", "Webブラウザ(_W)..." ), sigc::mem_fun( *this, &Core::slot_setup_browser ) );
    m_action_group->add( Gtk::Action::create( "SetupPasswd", "パスワード(_P)..." ), sigc::mem_fun( *this, &Core::slot_setup_passwd ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToggleIPv6", "IPv6使用(_I)", std::string(),
                                                    CONFIG::get_use_ipv6() ), sigc::mem_fun( *this, &Core::slot_toggle_ipv6 ) );

    // あぼーん
    m_action_group->add( Gtk::Action::create( "Abone_Menu", "あぼ〜ん(_A)" ) );
    m_action_group->add( Gtk::Action::create( "SetupAbone", "全体あぼ〜ん設定(対象: スレビュー)(_V)..." ), sigc::mem_fun( *this, &Core::slot_setup_abone ) );
    m_action_group->add( Gtk::Action::create( "SetupAboneThread", "全体スレあぼ〜ん設定(対象: スレ一覧)(_L)..." ),
                         sigc::mem_fun( *this, &Core::slot_setup_abone_thread ) );

    m_action_group->add( Gtk::ToggleAction::create( "TranspChainAbone", "スレビューで透明／連鎖あぼ〜んをデフォルト設定にする(_T)", std::string(),
                                                    ( CONFIG::get_abone_transparent() && CONFIG::get_abone_chain() ) ),
                                                    sigc::mem_fun( *this, &Core::slot_toggle_abone_transp_chain ) );

    // その他
    m_action_group->add( Gtk::Action::create( "Etc_Menu", "その他(_O)" ) );    
    m_action_group->add( Gtk::Action::create( "LivePref", "実況設定(_L)..." ), sigc::mem_fun( *this, &Core::slot_setup_live ) );
    m_action_group->add( Gtk::Action::create( "UsrCmdPref", "ユーザコマンドの編集(_U)..." ), sigc::mem_fun( *this, &Core::slot_usrcmd_pref ) );
    m_action_group->add( Gtk::Action::create( "FilterPref", "リンクフィルタの編集(_F)..." ), sigc::mem_fun( *this, &Core::slot_filter_pref ) );

    // プライバシー
    m_action_group->add( Gtk::Action::create( "Privacy_Menu", "プライバシー(_R)" ) );    
    m_action_group->add( Gtk::Action::create( "ClearAllPrivacy", "各履歴等の消去(_I)..." ), sigc::mem_fun( *this, &Core::slot_clear_privacy ) );
    m_action_group->add( Gtk::Action::create( "ClearPostLog", "書き込みログの消去(_P)" ), sigc::mem_fun( *this, &Core::slot_clear_post_log ) );
    m_action_group->add( Gtk::Action::create( "ClearPostHist", "書き込み履歴(鉛筆マーク)の消去(_H)" ), sigc::mem_fun( *this, &Core::slot_clear_post_history ) );
    m_action_group->add( Gtk::Action::create( "DeleteImages", "画像キャッシュの消去(_D)..." ), sigc::mem_fun( *this, &Core::slot_delete_all_images ) ); 


    //////////////////////////////////////////////////////

    // ツール
    m_action_group->add( Gtk::Action::create( "Menu_Tool", "ツール(_T)" ) );
    m_action_group->add( Gtk::Action::create( "LiveStartStop", "LiveStartStop"), sigc::mem_fun( *this, &Core::slot_live_start_stop ) );
    m_action_group->add( Gtk::Action::create( "SearchCache_Menu", "キャッシュ内ログ検索(_C)" ) );
    m_action_group->add( Gtk::Action::create( "SearchCacheBoard", "表示中の板のログを検索(_B)"), sigc::mem_fun( *this, &Core::slot_search_cache_board ) );
    m_action_group->add( Gtk::Action::create( "SearchCache", "キャッシュ内の全ログを検索(_A)"), sigc::mem_fun( *this, &Core::slot_search_cache ) );
    m_action_group->add( Gtk::Action::create( "SearchTitle", CONFIG::get_menu_search_title()+"(_T)" ), sigc::mem_fun( *this, &Core::slot_search_title ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdate_Menu", "全お気に入り更新チェック(_U)" ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateRoot", "更新チェックのみ(_R)"), sigc::mem_fun( *this, &Core::slot_check_update_root ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateOpenRoot", "更新されたスレをタブで開く(_T)"),
                         sigc::mem_fun( *this, &Core::slot_check_update_open_root ) );
    m_action_group->add( Gtk::Action::create( "CancelCheckUpdate", "キャンセル(_C)" ),
                         sigc::mem_fun( *this, &Core::slot_cancel_check_update ) );
    m_action_group->add( Gtk::Action::create( "ShowPostlog", "書き込みログの表示(_P)" ), sigc::mem_fun( *this, &Core::slot_show_postlog ) );

    //////////////////////////////////////////////////////

    // help
    m_action_group->add( Gtk::Action::create( "Menu_Help", "ヘルプ(_H)" ) );    
    m_action_group->add( Gtk::Action::create( "Bbs", "サポート掲示板(_B)" ), sigc::mem_fun( *this, &Core::slot_show_bbs ) );
    m_action_group->add( Gtk::Action::create( "OldLog", "2chスレ過去ログ(_L)" ), sigc::mem_fun( *this, &Core::slot_show_old2ch ) );
    m_action_group->add( Gtk::Action::create( "Manual", "オンラインマニュアル(_M)..." ),
                         Gtk::AccelKey( "F1" ), sigc::mem_fun( *this, &Core::slot_show_manual ) );
    m_action_group->add( Gtk::Action::create( "About", "JDについて(_A)..." ), sigc::mem_fun( *this, &Core::slot_show_about ) );
    

    m_ui_manager = Gtk::UIManager::create();    
    m_ui_manager->insert_action_group( m_action_group );

    // アクセラレータの追加
    m_win_main.add_accel_group( m_ui_manager->get_accel_group() );

    Glib::ustring menu_font = 
        "<menu action='FontColor_Menu'>"
        "<menuitem action='FontMain'/>"
        "<menuitem action='FontPopup'/>"
        "<menuitem action='FontTree'/>"
        "<separator/>"
        "<menuitem action='ColorChar'/>"
        "<menuitem action='ColorBack'/>"
        "<menuitem action='ColorCharTree'/>"
        "<menuitem action='ColorBackTree'/>"
        "<separator/>"
        "<menuitem action='FontColorPref'/>"
    "</menu>";

    Glib::ustring str_ui = 
        "<ui>"
        "<menubar name='menu_bar'>"

    // ファイル
        "<menu action='Menu_File'>"
        "<menuitem action='Online'/>"
        "<separator/>"
        "<menuitem action='Login2ch'/>"
        "<menuitem action='LoginBe'/>"
        "<menuitem action='LoginP2'/>"
        "<separator/>"
        "<menuitem action='SaveFavorite'/>"
        "<separator/>"
        "<menuitem action='ReloadList'/>"
        "<separator/>"
        "<menuitem action='Quit'/>"
        "</menu>"

    // 表示
        "<menu action='Menu_View'>"

        "<menu action='Sidebar_Menu'>"
        "<menuitem action='Show_BBS'/>"
        "<menuitem action='Show_FAVORITE'/>"
        "</menu>"
        "<separator/>"

        "<menuitem action='Show_Board'/>"
        "<menuitem action='Show_Thread'/>"
        "<menuitem action='Show_Image'/>"
        "<separator/>"

        "<menuitem action='2Pane'/>"
        "<menuitem action='3Pane'/>"
        "<menuitem action='v3Pane'/>"
        "<separator/>"

        "<menu action='View_Menu'>"

        "<menu action='General_Menu'>"
        "<menuitem action='ShowMenuBar'/>"
        "<menuitem action='ToggleFlatButton'/>"
        "<menuitem action='TogglePostMark'/>"
        "</menu>"
        "<separator/>"

        "<menu action='Tab_Menu'>"
        "<menuitem action='TabBoard'/>"
        "<menuitem action='TabArticle'/>"
        "</menu>"
        "<separator/>"

        "<menu action='Toolbar_Menu'>"
        "<menu action='Toolbar_Main_Menu'>"
        "<menuitem action='ToolbarPos0'/>"
        "<menuitem action='ToolbarPos1'/>"
        "</menu>"
        "<menuitem action='ToolbarBbslist'/>"
        "<menuitem action='ToolbarBoard'/>"
        "<menuitem action='ToolbarArticle'/>"
        "</menu>"
        "<separator/>"

        "<menu action='Item_Menu'>"
        "<menuitem action='SetupMainItem'/>"
        "<menuitem action='SetupSidebarItem'/>"
        "<menuitem action='SetupBoardItem'/>"
        "<menuitem action='SetupArticleItem'/>"
        "<menuitem action='SetupMsgItem'/>"
        "</menu>"
        "<separator/>"

        "<menu action='ListItem_Menu'>"
        "<menuitem action='SetupBoardItemColumn'/>"
        "</menu>"
    "<separator/>"

    + menu_font

    + "<separator/>"
    "<menu action='ImageView_Menu'>"

    "<menu action='ShowImageView_Menu'>"
    "<menuitem action='UseWinImg'/>"    
    "<menuitem action='UseEmbImg'/>"
    "</menu>"

    "<menuitem action='UseImgPopup'/>"    
    "<menuitem action='UseInlineImg'/>"    
    "<menuitem action='ShowSsspIcon'/>"    
    "</menu>"

    "<separator/>"

    "<menu action='ShowMsgView_Menu'>"
    "<menuitem action='UseWinMsg'/>"
    "<menuitem action='UseEmbMsg'/>"
    "</menu>"

        "</menu>"

        "</menu>"

    // ツール
        "<menu action='Menu_Tool'>"

        "<menuitem action='SearchTitle'/>"    
        "<separator/>"

        "<menuitem action='LiveStartStop'/>"    
        "<separator/>"

        "<menu action='SearchCache_Menu'>"
        "<menuitem action='SearchCacheBoard'/>"    
        "<menuitem action='SearchCache'/>"    
        "</menu>"
        "<separator/>"

        "<menu action='CheckUpdate_Menu'>"
        "<menuitem action='CheckUpdateRoot'/>"
        "<menuitem action='CheckUpdateOpenRoot'/>"
        "<separator/>"
        "<menuitem action='CancelCheckUpdate'/>"
        "</menu>"

        "<separator/>"
        "<menuitem action='ShowPostlog'/>"

        "</menu>"

    // 設定
        "<menu action='Menu_Config'>"

        "<menu action='Property_Menu'>"
        "<menuitem action='BoardPref'/>"
        "<menuitem action='ArticlePref'/>"
        "<menuitem action='ImagePref'/>"
        "</menu>"

        "<separator/>"

        "<menu action='General_Menu'>"
        "<menuitem action='OldArticle'/>"
        "<menuitem action='RestoreViews'/>"
        "<menuitem action='ToggleFoldMessage'/>"
        "<separator/>"
        "<menuitem action='SavePostLog'/>"
        "<menuitem action='SavePostHist'/>"
        "<separator/>"
        "<menuitem action='UseMosaic'/>"    
        "</menu>"

        "<separator/>"

        "<menu action='Mouse_Menu'>"
        "<menuitem action='ToggleTab'/>"
        "<menuitem action='TogglePopupWarp'/>"
        "<menuitem action='ShortMarginPopup'/>"
        "<separator/>"
        "<menuitem action='ToggleEmacsMode'/>"
        "<separator/>"
        "<menuitem action='KeyPref'/>"
        "<menuitem action='MousePref'/>"
        "<menuitem action='ButtonPref'/>"
        "</menu>"

    "<separator/>";


    str_ui += menu_font;

    str_ui +=

        "<separator/>"
        "<menu action='Net_Menu'>"
        "<menuitem action='SetupProxy'/>"
        "<menuitem action='SetupBrowser'/>"
        "<menuitem action='SetupPasswd'/>"
        "<separator/>"
        "<menuitem action='ToggleIPv6'/>"
        "</menu>"

        "<separator/>"

        "<menu action='Abone_Menu'>"
        "<menuitem action='SetupAbone'/>"
        "<menuitem action='SetupAboneThread'/>"
        "<separator/>"
        "<menuitem action='TranspChainAbone'/>"
        "</menu>"

        "<separator/>"

    // プライバシー
        "<menu action='Privacy_Menu'>"
        "<menuitem action='ClearAllPrivacy'/>"
        "<separator/>"
        "<menuitem action='ClearPostLog'/>"
        "<menuitem action='ClearPostHist'/>"
        "<separator/>"
        "<menuitem action='DeleteImages'/>"
        "</menu>"

        "<separator/>"

    // その他
        "<menu action='Etc_Menu'>"
        "<menuitem action='LivePref'/>"    
        "<menuitem action='UsrCmdPref'/>"
        "<menuitem action='FilterPref'/>"
        "</menu>"

        "</menu>"                         

    // ヘルプ
        "<menu action='Menu_Help'>"
        "<menuitem action='Manual'/>"
        "<separator/>"
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
    m_menubar->set_size_request( 0 );

    // 履歴メニュー追加
    Gtk::MenuItem* menuitem = Gtk::manage( new Gtk::MenuItem( "履歴(_S)", true ) );
    m_menubar->items().insert( --(--(--( m_menubar->items().end() ))), *menuitem );

    Gtk::Menu* submenu = Gtk::manage( new Gtk::Menu() );
    menuitem->set_submenu( *submenu );

    // 板履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_board() );
    
    // スレ履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_thread() );

    // 最近閉じたスレ履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_close() );

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

    // ツールバー作成
    create_toolbar();
    assert( m_toolbar );

    // サイドバー
    m_sidebar = BBSLIST::get_admin()->get_widget();
    assert( m_sidebar );

    // その他設定とwidgetのパッキング
    m_notebook_right.set_show_tabs( false );
    m_notebook_right.set_show_border( false );
    m_notebook_right.get_style()->set_xthickness( 10 );

    m_hpaned.get_ctrl().set_click_fold( SKELETON::PANE_CLICK_FOLD_PAGE1 );
    m_hpaned.get_ctrl().add_remove1( false, *m_sidebar );

    pack_widget( false );

    m_sigc_switch_page = m_notebook_right.signal_switch_page().connect( sigc::mem_fun( *this, &Core::slot_switch_page ) );
    m_hpaned.get_ctrl().sig_pane_modechanged().connect( sigc::mem_fun( *this, &Core::slot_show_hide_leftpane ) );

    m_win_main.signal_focus_out_event().connect( sigc::mem_fun(*this, &Core::slot_focus_out_event ) );
    m_win_main.signal_focus_in_event().connect( sigc::mem_fun(*this, &Core::slot_focus_in_event ) );
    m_win_main.show_all_children();

    // 各管理クラスが開いていたURLを復元
    core_set_command( "restore_views" );
}


//
// 3paneモードか
//
bool Core::is_3pane()
{
    int mode_pane = SESSION::get_mode_pane();

    return( mode_pane == SESSION::MODE_3PANE || mode_pane == SESSION::MODE_V3PANE );
}


// (bbslistを除く)全adminがemptyか
bool Core::is_all_admin_empty()
{
    bool emp_img = ! ( SESSION::get_embedded_img() && ! IMAGE::get_admin()->empty() );
    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );
    return ( BOARD::get_admin()->empty() && ARTICLE::get_admin()->empty() && emp_mes && emp_img );
}


//
// 右側ペーン取得
//
Gtk::Paned* Core::get_rpane()
{
    Gtk::Paned* paned_r = &m_vpaned_r;
    if( SESSION::get_mode_pane() == SESSION::MODE_V3PANE ) paned_r = &m_hpaned_r;

    return paned_r;
}


//
// 右側ペーンコントロール取得
//
SKELETON::PaneControl* Core::get_rpctrl()
{
    SKELETON::PaneControl* pctrl = &m_vpaned_r.get_ctrl();
    if( SESSION::get_mode_pane() == SESSION::MODE_V3PANE ) pctrl = &m_hpaned_r.get_ctrl();

    return pctrl;
}


//
// widget のパック
//
void Core::pack_widget( bool unpack )
{
    m_enable_menuslot = false;

    int mode_pane = SESSION::get_mode_pane();

    if( unpack ){
        SESSION::set_hpane_main_pos( m_hpaned.get_ctrl().get_position() );
        SESSION::set_vpane_main_pos( m_vpaned_r.get_ctrl().get_position() );
        SESSION::set_hpane_main_r_pos( m_hpaned_r.get_ctrl().get_position() );
        SESSION::set_vpane_main_mes_pos( m_vpaned_message.get_ctrl().get_position() );
    }

    if( SESSION::get_embedded_mes() ){ // 埋め込みmessage

        // 書き込みウィンドウを閉じる
        MESSAGE::get_admin()->set_command_immediately( "close_window" );

        m_vpaned_message.get_ctrl().add_remove1( unpack, *ARTICLE::get_admin()->get_widget() );
        m_vpaned_message.get_ctrl().add_remove2( unpack, *MESSAGE::get_admin()->get_widget() );

        m_notebook_right.append_remove_page( unpack, m_vpaned_message, "スレッド" );
    }
    else{

        // 書き込みウィンドウ表示
        MESSAGE::get_admin()->set_command_immediately( "open_window" );

        m_notebook_right.append_remove_page( unpack, *ARTICLE::get_admin()->get_widget(), "スレッド" );
    }

    if( SESSION::get_embedded_img() ){ // 埋め込みimage

        // 画像ウィンドウを閉じる
        IMAGE::get_admin()->set_command_immediately( "close_window" );

        m_notebook_right.append_remove_page( unpack, *IMAGE::get_admin()->get_widget(), "画像" );
    }
    else{

        // 画像ウィンドウ表示
        IMAGE::get_admin()->set_command_immediately( "open_window" );
    }

    // 画像インジケータ
    if( unpack ) hide_imagetab();

    // 2ペーン
    if( mode_pane == SESSION::MODE_2PANE ){ 

        m_notebook_right.append_remove_page( unpack, *BOARD::get_admin()->get_widget(), "スレ一覧" );

        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT )
            m_vbox_article.pack_remove_start( unpack, *m_toolbar, Gtk::PACK_SHRINK );

        m_vbox_article.pack_remove_start( unpack, m_notebook_right );

        m_hpaned.get_ctrl().add_remove2( unpack, m_vbox_article );
    }

    // 3ペーン
    else if( is_3pane() ){

        m_vbox_article.pack_remove_start( unpack, m_notebook_right );

        get_rpctrl()->add_remove1( unpack, *BOARD::get_admin()->get_widget() );
        get_rpctrl()->add_remove2( unpack, m_vbox_article );

        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT ){

            m_vbox_toolbar.pack_remove_start( unpack, *m_toolbar, Gtk::PACK_SHRINK );
            m_vbox_toolbar.pack_remove_start( unpack, *get_rpane() );

            m_hpaned.get_ctrl().add_remove2( unpack, m_vbox_toolbar );
        }
        else m_hpaned.get_ctrl().add_remove2( unpack, *get_rpane() );
    }

    // メインwindowのパッキング
    m_win_main.pack_remove_end( unpack, m_win_main.get_statbar(), Gtk::PACK_SHRINK );
    m_win_main.pack_remove_end( unpack, m_hpaned );
    if( SESSION::toolbar_pos() == SESSION::TOOLBAR_NORMAL )
        m_win_main.pack_remove_end( unpack, *m_toolbar, Gtk::PACK_SHRINK );
    if( SESSION::show_menubar() ) m_win_main.pack_remove_end( unpack, *m_menubar, Gtk::PACK_SHRINK );

    if( ! unpack ){

        // ペーンの位置設定
        m_vpaned_r.get_ctrl().set_position( SESSION::vpane_main_pos() );
        m_hpaned_r.get_ctrl().set_position( SESSION::hpane_main_r_pos() );
        m_vpaned_message.get_ctrl().set_position( SESSION::vpane_main_mes_pos() );

        // 画像インジケータ
        if( ! IMAGE::get_admin()->empty() ) show_imagetab();

        // サイドバーの位置設定
        m_hpaned.get_ctrl().set_position( SESSION::hpane_main_pos() );

        toggle_maximize_rightpane();
    }

    m_enable_menuslot = true;
}



//
// ツールバー作成
//
void Core::create_toolbar()
{
    if( m_toolbar ) return;

    m_toolbar = new MainToolBar();

    m_toolbar->m_button_bbslist.signal_clicked().connect(
        sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_BBSLISTVIEW, false ) );
    m_toolbar->m_button_favorite.signal_clicked().connect(
        sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_FAVORITEVIEW, false ) );

    m_toolbar->m_button_board.signal_clicked().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_board ), false ) );
    m_toolbar->m_button_thread.signal_clicked().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_article ), false ) );
    m_toolbar->m_button_image.signal_clicked().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_image ), false ) );
    m_toolbar->m_entry_url.signal_activate().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );
    m_toolbar->m_button_go.signal_clicked().connect( sigc::mem_fun( *this, &Core::slot_active_url ) );

    m_toolbar->show_toolbar();
}



//
// 初回起動時のセットアップ
//
void Core::first_setup()
{
    m_init = true;

    SetupWizard wizard;
    wizard.run();

    m_init = false;
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
    CONTROL::save_conf();

    ARTICLE::get_admin()->shutdown();
    BOARD::get_admin()->shutdown();
    BBSLIST::get_admin()->shutdown();
    IMAGE::get_admin()->shutdown();
    MESSAGE::get_admin()->shutdown();
}


//
// メインタイトルセット
//
void Core::set_maintitle()
{
    if( SESSION::is_booting() ) return;

    std::string title;

    if( m_title.empty() ) title = "JD - " + JDVERSIONSTR;
    else title = "JD - " + m_title;

    if( CORE::get_login2ch()->login_now() ) title +=" [ ● ]";
    if( CORE::get_loginbe()->login_now() ) title +=" [ BE ]";
    if( SESSION::loginp2() ) title +=" [ p2 ]";
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

        if( SESSION::show_sidebar() && SESSION::get_bbslist_current_url() == URL_BBSLISTVIEW ) tact->set_active( true );
        else tact->set_active( false );
    }

    act = m_action_group->get_action( "Show_FAVORITE" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){

        if( SESSION::show_sidebar() && SESSION::get_bbslist_current_url() == URL_FAVORITEVIEW ) tact->set_active( true );
        else tact->set_active( false );
    }

    // メニューバー
    act = m_action_group->get_action( "ShowMenuBar" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::show_menubar() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // ボタンのrelief切り替え
    act = m_action_group->get_action( "ToggleFlatButton" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( CONFIG::get_flat_button() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // ツールバー
    act = m_action_group->get_action( "ToolbarPos0" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_NORMAL ) tact->set_active( true );
        else tact->set_active( false );
    }
    act = m_action_group->get_action( "ToolbarPos1" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::toolbar_pos() == SESSION::TOOLBAR_RIGHT ) tact->set_active( true );
        else tact->set_active( false );
    }
    act = m_action_group->get_action( "ToolbarBbslist" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::get_show_bbslist_toolbar() ) tact->set_active( true );
        else tact->set_active( false );
    }
    act = m_action_group->get_action( "ToolbarBoard" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::get_show_board_toolbar() ) tact->set_active( true );
        else tact->set_active( false );
    }
    act = m_action_group->get_action( "ToolbarArticle" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::get_show_article_toolbar() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // タブ
    act = m_action_group->get_action( "TabBoard" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::get_show_board_tab() ) tact->set_active( true );
        else tact->set_active( false );
    }
    act = m_action_group->get_action( "TabArticle" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( SESSION::get_show_article_tab() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 2chログイン
    act = m_action_group->get_action( "Login2ch" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
    if( tact ){

        if( CORE::get_login2ch()->login_now() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // BEログイン
    act = m_action_group->get_action( "LoginBe" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
    if( tact ){

        if( CORE::get_loginbe()->login_now() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // P2ログイン
    act = m_action_group->get_action( "LoginP2" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
    if( tact ){

        if( SESSION::loginp2() ) tact->set_active( true );
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

    // 画像ビュー表示切り替え
    act = m_action_group->get_action( "UseWinImg" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( ! CONFIG::get_use_image_view() ) tact->set_active( false );
        else{
            if( SESSION::get_embedded_img() ) tact->set_active( false );
            else tact->set_active( true );
        }
    }
    act = m_action_group->get_action( "UseEmbImg" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( ! CONFIG::get_use_image_view() ) tact->set_active( false );
        else{
            if( SESSION::get_embedded_img() ) tact->set_active( true );
            else tact->set_active( false );
        }
    }

    // 開いている板のログ検索
    act = m_action_group->get_action( "SearchCacheBoard" );
    if( ! BOARD::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );

    // スレ一覧のプロパティ
    act = m_action_group->get_action( "BoardPref" );
    if( ! BOARD::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );
    
    // スレのプロパティ
    act = m_action_group->get_action( "ArticlePref" );
    if( ! ARTICLE::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );

    // 画像のプロパティ
    act = m_action_group->get_action( "ImagePref" );
    if( ! IMAGE::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );

    // 実況
    act = m_action_group->get_action( "LiveStartStop" );
    if( ! ARTICLE::get_admin()->empty() ) act->set_sensitive( true );
    else act->set_sensitive( false );

    // emacsモード
    act = m_action_group->get_action( "ToggleEmacsMode" );
    tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
    if( tact ){
        if( CONTROL::is_emacs_mode() ) tact->set_active( true );
        else tact->set_active( false );
    }

    m_enable_menuslot = true;
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
// 画像キャッシュクリア
//
void Core::slot_delete_all_images()
{
    DBIMG::delete_all_files();
    IMAGE::get_admin()->set_command( "close_uncached_views" );
}


// プライバシー情報のクリア
void Core::slot_clear_privacy()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_PRIVARY, "" );
    pref->run();
    delete pref;
}

void Core::slot_clear_board()
{
    HISTORY::get_history_manager()->clear_board();
    BOARD::get_admin()->set_command( "clear_viewhistory" );
}

void Core::slot_clear_thread()
{
    HISTORY::get_history_manager()->clear_thread();
    ARTICLE::get_admin()->set_command( "clear_viewhistory" );
}

void Core::slot_clear_close()
{
    HISTORY::get_history_manager()->clear_close();
}

void Core::slot_clear_search()
{
    get_completion_manager()->clear( CORE::COMP_SEARCH_ARTICLE );
    get_completion_manager()->clear( CORE::COMP_SEARCH_BBSLIST );
    get_completion_manager()->clear( CORE::COMP_SEARCH_BOARD );
}

void Core::slot_clear_name()
{
    get_completion_manager()->clear( CORE::COMP_NAME );
}

void Core::slot_clear_mail()
{
    get_completion_manager()->clear( CORE::COMP_MAIL );
}

// 書き込みログのクリア
void Core::slot_clear_post_log()
{
    SKELETON::MsgDiag mdiag( NULL, "書き込みログを削除しますか？",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    MESSAGE::get_log_manager()->clear_post_log();

    // ログ表示を閉じる
    ARTICLE::get_admin()->set_command( "close_view", "postlog" + std::string( POSTLOG_SIGN ), "closeall"  );
}


// 全スレの書き込み履歴(鉛筆マーク)のクリア
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
// メインフォント変更
//
void Core::slot_changefont_main()
{
    Gtk::FontSelectionDialog diag;
    diag.set_font_name( CONFIG::get_fontname( FONT_MAIN ) );
    diag.set_title( "スレビューフォント" );
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
// 色選択ダイアログを開く
// フォントと色の設定
//
bool Core::open_color_diag( std::string title, int id )
{
    Gdk::Color color( CONFIG::get_color( id ) );

    Gtk::ColorSelectionDialog diag( title );
    diag.get_colorsel()->set_current_color( color );
    if( diag.run() == Gtk::RESPONSE_OK ){

        CONFIG::set_color( id, MISC::color_to_str( diag.get_colorsel()->get_current_color() ) );
        return true;
    }

    return false;
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
// キーボード詳細設定
//
void Core::slot_setup_key()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_KEY, "" );
    pref->run();
    delete pref;
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
// マウスボタン詳細設定
//
void Core::slot_setup_button()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BUTTON, "" );
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
// リスト項目(スレ一覧)の設定
//
void Core::slot_setup_boarditemcolumn()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARDITEMCOLUM, "" );
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
// ユーザコマンドの編集
//
void Core::slot_usrcmd_pref()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_USRCMD, "" );
    pref->run();
    delete pref;
}


//
// リンクフィルタの編集
//
void Core::slot_filter_pref()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_LINKFILTER, "" );
    pref->run();
    delete pref;
}



//
// ブラウザ設定
//
void Core::slot_setup_browser()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BROWSER, "" );
    pref->run();
    delete pref;
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
    open_by_browser( JDHELP );
}


//
// about:config
//
void Core::slot_aboutconfig()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ABOUTCONFIG, "" );
    pref->run();
    delete pref;
}


//
// about
//
void Core::slot_show_about()
{
    std::string version = JDVERSIONSTR;
    std::stringstream license;
    const Glib::ustring comments = "JDはLinux用 2ch ブラウザです";
    const Glib::ustring copyright = JDCOPYRIGHT;

    // [ ライセンス表記 ]
    //
    // 以下の文章は和訳を元にバージョン及び住所を訂正した物です。
    // http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
    // http://www.opensource.jp/gpl/gpl.ja.html#SEC4 (和訳)
    license <<
    "JD は GNOME 上で動作する 2ch ブラウザです。\n"
    "\n" <<
    JDCOPYRIGHT << "\n"
    "\n"
    "このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェ"
    "ア財団によって発行された GNU 一般公衆利用許諾契約書(バージョン2)の定める"
    "条件の下で再頒布または改変することができます。\n"
    "\n"
    "このプログラムは有用であることを願って頒布されますが、*全くの無保証* "
    "です。商業可能性の保証や特定の目的への適合性は、言外に示されたものも含"
    "め全く存在しません。詳しくはGNU 一般公衆利用許諾契約書をご覧ください。\n"
    "\n"
    "あなたはこのプログラムと共に、GNU 一般公衆利用許諾契約書の複製物を一部"
    "受け取ったはずです。もし受け取っていなければ、フリーソフトウェア財団ま"
    "で請求してください(宛先は the Free Software Foundation, Inc., 51 "
    "Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA)。\n";

    SKELETON::AboutDiag about( "JDについて" );
    about.set_logo( ICON::get_icon( ICON::JD96 ) );
    about.set_version( version );
    about.set_comments( comments );
    about.set_website( CONFIG::get_url_jdhp() );
    about.set_copyright( copyright );
    about.set_license( license.str() );
    about.run();
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
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    DBTREE::download_bbsmenu();
    CORE::core_set_command( "set_status","", "板一覧再読み込み中...." );        
}


//
// お気に入りルートからの更新チェック( 通常 )
//
void Core::slot_check_update_root()
{
    CORE::core_set_command( "check_update_root", "" );
}


//
// お気に入りルートからの更新チェック( タブで開く )
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
    std::cout << "Core::slot_toggle_p2\n";
#endif

    SESSION::set_loginp2( ! SESSION::loginp2() );

    set_maintitle();
}


//
// メニューバー表示切替え
//
void Core::toggle_menubar()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::toggle_menubar\n";
#endif

    pack_widget( true );
    SESSION::set_show_menubar( ! SESSION::show_menubar() );
    pack_widget( false );

    restore_focus( true, false );

    if( ! SESSION::show_menubar() ){
        SKELETON::MsgDiag mdiag( NULL, "メニューバーを再表示するには\n\n" + CONTROL::get_str_motions( CONTROL::ShowMenuBar ) + "\n\nを押してください" );
        mdiag.run();
    }
}


//
// ボタンのreliefの切り替え
//
void Core::toggle_flat_button()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::toggle_flat_button\n";
#endif

    CONFIG::set_flat_button( ! CONFIG::get_flat_button() );

    ARTICLE::get_admin()->set_command( "update_toolbar_button" );
    BOARD::get_admin()->set_command( "update_toolbar_button" );
    BBSLIST::get_admin()->set_command( "update_toolbar_button" );
    IMAGE::get_admin()->set_command( "update_toolbar_button" );
    MESSAGE::get_admin()->set_command( "update_toolbar_button" );
    m_toolbar->update_button();
}


//
// 書き込みマーク表示切り替え
//
void Core::toggle_post_mark()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

    CONFIG::set_show_post_mark( ! CONFIG::get_show_post_mark() );

    ARTICLE::get_admin()->set_command( "relayout_all" );
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
// 書き込みログ
//
void Core::slot_show_postlog()
{
    CORE::core_set_command( "open_article_postlog" );
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
// サイドバー表示切替え
//
// url を開いているときは閉じる
// 閉じているときは開く
//
// サイドバーの表示が切り替わったら slot_show_hide_leftpane()が呼び出される
//
void Core::toggle_sidebar()
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::toggle_sidebar focus = " << SESSION::focused_admin()
              << " mode = " << m_hpaned.get_ctrl().get_mode()
              << " empty = " << is_all_admin_empty() << std::endl;
#endif

    // 閉じていたらサイドバーを開く
    if( ! SESSION::show_sidebar() ){

        // 右ペーンが空の時は常に最大化
        if( CONFIG::get_expand_sidebar() && is_all_admin_empty() ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE1 );

        // 通常
        else m_hpaned.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

    }

    // 開いていたら閉じる( 右ペーンが空の時は閉じない )
    else if( ! is_all_admin_empty() )  m_hpaned.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE2 );
}


//
// サイドバーの表示が切り替わったときに呼ばれる
//
void Core::slot_show_hide_leftpane( int mode )
{
    const bool present = false;

#ifdef _DEBUG
    std::cout << "slot_show_hide_leftpane mode = " << mode << std::endl;
#endif

    if( mode == SKELETON::PANE_NORMAL ) SESSION::set_show_sidebar( true );
    else SESSION::set_show_sidebar( false );

    // 表示されたらサイドバーをフォーカス
    if( SESSION::focused_admin() != SESSION::FOCUS_SIDEBAR && SESSION::show_sidebar() ) switch_sidebar( std::string(), present );

    // 非表示になったときは SESSION::focused_admin_sidebar() で指定されるadminにフォーカスを移す
    else{

#ifdef _DEBUG
        std::cout << "focused_admin = " << SESSION::focused_admin_sidebar() << std::endl;
#endif

        if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_BOARD ) switch_board( present );
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_ARTICLE ) switch_article( present );
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_IMAGE ) switch_image( present );
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_MESSAGE ) switch_message( present );
        else if( SESSION::focused_admin_sidebar() == SESSION::FOCUS_NO ){

            if( ! BOARD::get_admin()->empty() ) switch_board( present );
            else if( ! ARTICLE::get_admin()->empty() ) switch_article( present );
            else if( ! IMAGE::get_admin()->empty() ) switch_image( present );
        }
    }
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
    if( SESSION::toolbar_pos() == pos ) pos = SESSION::TOOLBAR_NO;
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

    SESSION::set_focused_admin( SESSION::FOCUS_NO );
    SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
    restore_focus( true, false );
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
// コマンドセット
//
// 他のadminクラスに委譲する場合はこの関数で、coreが実行するコマンドはexec_command()で処理
//
void Core::set_command( const COMMAND_ARGS& command )
{
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "Core::set_command : " << command.command << " " << command.url
              << " " << command.arg1 << " " << command.arg2 << " " << command.arg3 << " " << command.arg4 << std::endl;
#endif

    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );    
    
    ////////////////////////////
    // article系のコマンド

    // メインビュー
    if( command.command  == "open_article" ) {

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        ARTICLE::get_admin()->set_command( "open_list",
                                           std::string(),

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           command.arg1 // datファイルのURLを空白で区切って指定
            );

        return;
    }

    // キーワードで抽出( AND/OR )
    else if( command.command  == "open_article_keyword" ) { 

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        std::string mode_str = "KEYWORD";
        if( command.arg2 == "true" ) mode_str = "KEYWORD_OR";  // OR 抽出
        
        // 検索履歴更新
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_ARTICLE, command.arg1 );
       
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

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        // 画像ウィンドウが開いている時にメインウィンドウを前面に出す
        switch_article( true );

        return;
    }


    // 名前 で抽出
    else if( command.command  == "open_article_name" ) { 

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

    // 自分の書き込みレスを抽出
    else if( command.command  == "open_article_post" ) { 

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "POST" // 書き込み抽出モード
            );
        return;
    }

    // URL抽出
    else if( command.command  == "open_article_url" ) { 

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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
    else if( command.command  == "open_article_searchlog" ) { 

        if( CORE::get_search_manager()->is_searching() ){
            SKELETON::MsgDiag mdiag( NULL, "他の検索スレッドが実行中です" );
            mdiag.run();
            return;
        }

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        // 検索履歴更新
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_ARTICLE, command.arg1 );

        ARTICLE::get_admin()->set_command( "open_view",
                                           command.url, 

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "SEARCHLOG", // ログ検索

                                           command.arg1, // query
                                           "exec" // Viewを開いた直後に検索開始
            );

        return;
    }

    // 全ログ検索
    else if( command.command  == "open_article_searchalllog" ) { 

        if( CORE::get_search_manager()->is_searching() ){
            SKELETON::MsgDiag mdiag( NULL, "他の検索スレッドが実行中です" );
            mdiag.run();
            return;
        }

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );
        
        // 検索履歴更新
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_ARTICLE, command.arg1 );

        ARTICLE::get_admin()->set_command( "open_view",
                                           "allboard",

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "SEARCHALLLOG", // 全ログ検索

                                           command.arg1, // query
                                           "exec" // Viewを開いた直後に検索開始
            );

        return;
    }


    // スレタイ検索
    else if( command.command  == "open_article_searchtitle" ) { 

        if( CORE::get_search_manager()->is_searching() ){
            SKELETON::MsgDiag mdiag( NULL, "他の検索スレッドが実行中です" );
            mdiag.run();
            return;
        }

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        ARTICLE::get_admin()->set_command( "open_view",
                                           "title",

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "SEARCHTITLE", // モード

                                           command.arg1, // query
                                           "exec" // Viewを開いた直後に検索開始
            );

        return;
    }

    // 書き込みログ表示
    else if( command.command  == "open_article_postlog" ) { 

        if( ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        ARTICLE::get_admin()->set_command( "open_view",
                                           "postlog",

                                           // 以下 COMMAND_ARGS::arg1, arg2,....
                                           "left", // タブで開く
                                           "true", // url 開いてるかチェックしない
                                           "", // 開き方のモード ( Admin::open_view 参照 )

                                           "POSTLOG", // モード
                                           command.arg1 // ログ番号
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
    //
    // command.arg1 == "reopen" のときはキャッシュだけ再読み込み
    //
    else if( command.command == "delete_article" ){

        DBTREE::delete_article( command.url, ( command.arg1 == "reopen" ) );

        if( DBTREE::article_is_cached( command.url ) ) return;

        ARTICLE::get_admin()->set_command( "unlock_views", command.url );
        ARTICLE::get_admin()->set_command( "close_view", command.url,
                                           "closeall" // command.url を含む全てのビューを閉じる
            );

        // ポップアップも削除して対象となるarticlebaseのロックを解除 (注) ポップアップは遅延してdeleteされる
        // そうしないと articlebase::unlock_impl()が呼び出されないためnotetreebaseが削除されない
        ARTICLE::get_admin()->set_command( "delete_all_popups" );  

        // もう一度開く
        if( command.arg1 == "reopen" ) core_set_command( "open_article", command.url , "true", "",  command.arg2 );

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

    // タブアイコンのセット
    else if( command.command  == "toggle_article_icon" ){

        ARTICLE::get_admin()->set_command( "toggle_icon", command.url );
        return;
    }

    // ツールバー表示更新
    else if( command.command  == "redraw_article_toolbar" ){

        ARTICLE::get_admin()->set_command( "redraw_toolbar" );
        return;
    }

    // ツールバーボタン更新
    else if( command.command  == "update_article_toolbar_button" ){

        ARTICLE::get_admin()->set_command( "update_toolbar_button" );
        return;
    }

    // ポップアップメニュー再作成
    else if( command.command  == "reset_article_popupmenu" ){

        ARTICLE::get_admin()->set_command( "reset_popupmenu" );
        return;
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


    // 板を閉じる
    // その板に所属するスレも閉じる
    else if( command.command == "close_board" ){

        std::string datbase = DBTREE::url_datbase( command.url );

        ARTICLE::get_admin()->set_command_immediately( "close_view", datbase,
                                                       "closeall" // datbase を含む全てのビューを閉じる
            );

        BOARD::get_admin()->set_command_immediately( "close_view", command.url,
                                                     "closeall" // command.url を含む全てのビューを閉じる
            );
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
   
    // ツールバーボタン更新
    else if( command.command  == "update_board_toolbar_button" ){

        BOARD::get_admin()->set_command( "update_toolbar_button" );
        return;
    }

    // 列項目更新
    else if( command.command  == "update_board_columns" ){

        BOARD::get_admin()->set_command( "update_columns" );
        return;
    }

    // 全boardviewの再レイアウト
    else if( command.command == "relayout_all_board" ){
        BOARD::get_admin()->set_command( "relayout_all" );
    }


    ////////////////////////////
    // bbslist系のコマンド

    else if( command.command  == "update_bbslist" ){

        CORE::core_set_command( "set_status","" ,"" );        

        // フォーカスされていなかったらサイドバーにフォーカス切り替え
        if( SESSION::focused_admin() != SESSION::FOCUS_SIDEBAR
            || SESSION::get_bbslist_current_url() != URL_BBSLISTVIEW ) CORE::core_set_command( "switch_sidebar", URL_BBSLISTVIEW );

        BBSLIST::get_admin()->set_command( "update_view", URL_BBSLISTVIEW );
        return;
    }
    else if( command.command  == "update_bbslist_item" ){

        BBSLIST::get_admin()->set_command( "update_item", URL_BBSLISTVIEW );
        return;
    }
    else if( command.command  == "append_favorite" ){

        BBSLIST::get_admin()->set_command( "append_item", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "update_favorite_item" ){

        BBSLIST::get_admin()->set_command( "update_item", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "check_update_root" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
            mdiag.run();
            return;
        }

        BBSLIST::get_admin()->set_command( "check_update_root", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "check_update_open_root" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
            mdiag.run();
            return;
        }

        BBSLIST::get_admin()->set_command( "check_update_open_root", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "cancel_update" ){

        BBSLIST::get_admin()->set_command( "cancel_update", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "save_favorite" ){

        BBSLIST::get_admin()->set_command( "save_xml", URL_FAVORITEVIEW );
        return;
    }
    else if( command.command  == "toggle_favorite_icon" ){

        BBSLIST::get_admin()->set_command( "toggle_icon", command.url );
        return;
    }

    // ツールバーボタン更新
    else if( command.command  == "update_bbslist_toolbar_button" ){

        BBSLIST::get_admin()->set_command( "update_toolbar_button" );
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
        if( ! DBIMG::is_cached( command.url ) ){
            const bool nomosaic = false;
            DBIMG::download_img( command.url, std::string(), nomosaic );
        }

        IMAGE::get_admin()->set_command( "open_view", command.url );
        return;
    }
    else if( command.command == "delete_image" ){

        DBIMG::delete_cache( command.url );
        return;
    }

    // キャッシュに無い画像を閉じる
    else if( command.command == "close_nocached_image_views" ){

        IMAGE::get_admin()->set_command( "close_nocached_views" );
        return;
    }

    ////////////////////////////
    // message系

    else if( command.command == "open_message" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
            mdiag.run();
        }
        else{

            if( SESSION::get_embedded_mes() ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );
            MESSAGE::get_admin()->set_command( "open_view", command.url, command.arg1 );
        }
    }

    else if( command.command == "create_new_thread" ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
            mdiag.run();
        }
        else if( DBTREE::url_bbscgi_new( command.url ).empty() ){
            SKELETON::MsgDiag mdiag( NULL, "この板では新スレを立てることは出来ません" );
            mdiag.run();
        }
        else{
            if( SESSION::get_embedded_mes() ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_NORMAL );
            MESSAGE::get_admin()->set_command( "open_view", command.url, command.arg1, "new" );
        }
    }

    // ツールバーボタン更新
    else if( command.command  == "update_message_toolbar_button" ){

        MESSAGE::get_admin()->set_command( "update_toolbar_button" );
        return;
    }

    // messageviewの再レイアウト
    else if( command.command == "relayout_all_message" ){
        MESSAGE::get_admin()->set_command( "relayout_all" );
    }

    ////////////////////////////
    // ダイアログボックスが表示/非表示状態になった

    else if( command.command == "dialog_shown" ){ // 表示された

        // フォーカスが外れて画像ウィンドウの開け閉めをしないようにする
        IMAGE::get_admin()->set_command_immediately( "disable_fold_win" );
        MESSAGE::get_admin()->set_command_immediately( "disable_fold_win" );

        return;
    }
    else if( command.command == "dialog_hidden" ){ // 非表示になった

        IMAGE::get_admin()->set_command_immediately( "enable_fold_win" );
        MESSAGE::get_admin()->set_command_immediately( "enable_fold_win" );

        return;
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
        MESSAGE::get_admin()->set_command( "redraw_current_view" );

        return;
    }

    ///////////////////////////////

    // 移転があった
    else if( command.command == "update_url" ){

        ARTICLE::get_admin()->set_command( "update_url", command.url, command.arg1 );
        BOARD::get_admin()->set_command( "update_url", command.url, command.arg1 );
        BBSLIST::get_admin()->set_command( "update_url", command.url, command.arg1 );
        IMAGE::get_admin()->set_command( "update_url", command.url, command.arg1 );
        MESSAGE::get_admin()->set_command( "update_url", command.url, command.arg1 );

        return;
    }

    ///////////////////////////////

    // 板名変更
    else if( command.command == "update_boardname" ){

        ARTICLE::get_admin()->set_command( "update_boardname", command.url );
        BOARD::get_admin()->set_command( "update_boardname", command.url );
        BBSLIST::get_admin()->set_command( "update_boardname", command.url );
        IMAGE::get_admin()->set_command( "update_boardname", command.url );
        MESSAGE::get_admin()->set_command( "update_boardname", command.url );

        return;
    }

    ///////////////////////////////

    // フォーカス回復
    else if( command.command == "restore_focus" ){

        restore_focus( true, ( command.arg1 == "present" ) );
        return;
    }

    ///////////////////////////////

    // タイトル、URL、ステータスなどの表示
    else if( command.command  == "set_title" ){
        m_title = command.arg1;
        set_maintitle();
    }

    else if( command.command  == "set_url" ){
        m_toolbar->m_entry_url.set_text( command.url );
    }

    else if( command.command  == "set_status" ){
        m_win_main.set_status( command.arg1 );
    }

    else if( command.command  == "set_status_temporary" ){
        m_win_main.set_status_temporary( command.arg1 );
    }

    else if( command.command  == "restore_status" ){
        m_win_main.restore_status();
    }

    else if( command.command  == "set_info" ){
        m_win_main.set_mginfo( command.arg1 );
    }

    // マウスジェスチャ
    else if( command.command  == "set_mginfo" ){

        // 画像ウィンドウが表示されている場合
        if( ! SESSION::get_embedded_img() && SESSION::is_shown_win_img()
            && SESSION::is_focus_win_img() ) IMAGE::get_admin()->set_command( "set_mginfo", "", command.arg1 );

        else m_win_main.set_mginfo( command.arg1 );
    }

    ////////////////////////////
    // その他 Coreが自前で処理するコマンド( Core::exec_command() で処理 )

    m_list_command.push_back( command );
    dispatch(); // 一度メインループに戻った後にcallback_dispatch() が呼び戻される
}


//
// ディスパッチャのコールバック関数
//
void Core::callback_dispatch()
{
    while( m_list_command.size() ) exec_command();
}


// coreが自前でする処理
void Core::exec_command()
{
    const bool present = false;

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

        // 残りは CONFIG::get_restore_* で全てリストアするかロックされているタブだけリストアするか決定
        BOARD::get_admin()->set_command( "restore", "", ( CONFIG::get_restore_board() ? "" : "only_locked" ) );
        ARTICLE::get_admin()->set_command( "restore", "", ( CONFIG::get_restore_article() ? "" : "only_locked" ) );

        // ロックされている画像があるか調べる
        bool img_locked = false;
        if( ! CONFIG::get_restore_image() ){
            std::list< bool > list_locked = SESSION::get_image_locked();
            std::list< bool >::iterator it_locked = list_locked.begin();
            for( ; it_locked != list_locked.end(); ++it_locked ){
                if( ( *it_locked ) ){ img_locked = true; break; }
            }
        }

        if( SESSION::image_URLs().size() &&
            ( CONFIG::get_restore_image() || img_locked ) ){

            show_imagetab();

            IMAGE::get_admin()->set_command( "restore", "", ( CONFIG::get_restore_image() ? "" : "only_locked" ) );
        }
    }

    // 各ビューのタブ幅調整
    else if( command.command == "adjust_tabwidth" ){

        BOARD::get_admin()->set_command( "adjust_tabwidth" );
        ARTICLE::get_admin()->set_command( "adjust_tabwidth" );
    }

    // メインツールバーのボタン表示更新
    else if( command.command == "update_main_toolbar_button" ){
        m_toolbar->update_button();
    }

    // history 登録
    else if( command.command  == "set_history_article" ) set_history_article( command.url );
        
    else if( command.command  == "set_history_board" ) set_history_board( command.url );

    else if( command.command  == "set_history_close" ) set_history_close( command.url );

    else if( command.command  == "update_history" ){
        HISTORY::get_history_manager()->update_thread();
        HISTORY::get_history_manager()->update_board();
        HISTORY::get_history_manager()->update_close();
    }


    // 履歴のクリア
    else if( command.command  == "clear_board" ) slot_clear_board();

    else if( command.command  == "clear_thread" ) slot_clear_thread();

    else if( command.command  == "clear_closed_thread" ) slot_clear_close();

    else if( command.command  == "clear_search" ) slot_clear_search();

    else if( command.command  == "clear_name" ) slot_clear_name();

    else if( command.command  == "clear_mail" ) slot_clear_mail();

    // ビューの切替え
    else if( command.command  == "switch_article" ) switch_article( present );

    else if( command.command  == "switch_board" ) switch_board( present );

    else if( command.command  == "switch_sidebar" ) switch_sidebar( command.url, present );

    else if( command.command  == "switch_image" ) switch_image( present );

    else if( command.command  == "switch_message" ) switch_message( present );

    else if( command.command  == "toggle_article" ) toggle_article();

    else if( command.command  == "switch_leftview" ) switch_leftview();

    else if( command.command  == "switch_rightview" ) switch_rightview();

    // メニューバー表示/非表示
    else if( command.command  == "toggle_menubar" ) toggle_menubar();

    // サイドバー表示/非表示
    else if( command.command  == "toggle_sidebar" ) toggle_sidebar();

    // 2chへのログイン処理が完了した
    else if( command.command  == "login2ch_finished" ) set_maintitle();

    // あるadminのnotebookが空になった
    else if( command.command  == "empty_page" ) empty_page( command.url );

    // あるadminのnotebookがswitchした
    else if( command.command  == "page_switched" ){
        set_toggle_view_button();
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

    // グローバルあぼーん(ワード)
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
                                          command.arg3, // 選択文字
                                          atoi( command.arg4.c_str() )  // レス番号
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
        std::string num_str;
        const std::string url_dat = DBTREE::url_dat( command.url, num_from, num_to, num_str );
        const std::string url_subject = DBTREE::url_subject( command.url );
       
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

        // 画像の場合
        else if( DBIMG::get_type_ext( command.url ) != DBIMG::T_UNKNOWN ){

            // 画像ビューを使用
            if( CONFIG::get_use_image_view() ){

                if( ! SESSION::is_online() ){
                    SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
                    mdiag.run();
                }
                else{
                    // キャッシュに無かったらロード
                    if( ! DBIMG::is_cached( command.url ) ){
                        const bool nomosaic = false;
                        DBIMG::download_img( command.url, std::string(), nomosaic );
                    }

                    CORE::core_set_command( "open_image", command.url );
                    CORE::core_set_command( "switch_image" );
                }
            }

            // 外部ビュアー使用
            else open_by_browser( command.url );
        }

        // 掲示板のベースURLの場合
        else if( ! url_subject.empty() ){

#ifdef _DEBUG
            std::cout << "exec : open_board url = " << url_subject << std::endl;
#endif

            CORE::core_set_command( "open_board" , url_subject, "true" );
        }

        // その他
        else open_by_browser( command.url );
    }

    // ある admin クラスのコマンドが空になった
    else if( command.command  == "empty_command" ){}

    // ある jdwindow クラスのブートが終わった
    else if( command.command  == "window_boot_fin" ){}

    // 起動中
    if( SESSION::is_booting() && ! m_init ){

        // coreがコマンドを全て実行して、かつ全てのadminクラスがブートした
        if( m_list_command.size() == 0
            && ! BBSLIST::get_admin()->is_booting()
            && ! BOARD::get_admin()->is_booting()
            && ! ARTICLE::get_admin()->is_booting()
            && ! IMAGE::get_admin()->is_booting()
            && ! MESSAGE::get_admin()->is_booting()
            ){

            // 起動完了
            SESSION::set_booting( false );
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

    // サイドバー表示状態変更
    if( ! SESSION::show_sidebar() ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE2 );

    // フォーカス回復
    restore_focus( true, true );

    // タイマーセット
    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &Core::slot_timeout), 0 );
    sigc::connection conn = Glib::signal_timeout().connect( slot_timeout, TIMER_TIMEOUT );

    // 2chログイン
    if( SESSION::login2ch() ) slot_toggle_login2ch();

    // BEログイン
    if( SESSION::loginbe() ) slot_toggle_loginbe();

    // タイトル表示
    set_maintitle();

    // 画像ウィンドウが復元されると画面が表示されないので再レイアウト指定
    ARTICLE::get_admin()->set_command( "relayout_current_view" );

#ifdef _DEBUG
    std::cout << "\n\n----------- boot fin --------------\n\n";
#endif
}


//
// フォーカス回復
//
// force : true の時は強制的に回復(処理が重い)
// present : フォーカス回復後にメインウィンドウをpresentする
//
void Core::restore_focus( const bool force, const bool present )
{
    int admin = SESSION::focused_admin();

    // フォーカスするadminがemptyならリセット
    bool reset_focus = false;
    switch( admin )
    {
        case SESSION::FOCUS_SIDEBAR: if( BBSLIST::get_admin()->empty() ) reset_focus = true; break;
        case SESSION::FOCUS_BOARD: if( BOARD::get_admin()->empty() ) reset_focus = true; break;
        case SESSION::FOCUS_ARTICLE: if( ARTICLE::get_admin()->empty() ) reset_focus = true; break;
        case SESSION::FOCUS_IMAGE: if( IMAGE::get_admin()->empty() ) reset_focus = true; break;
        case SESSION::FOCUS_MESSAGE: if( MESSAGE::get_admin()->empty() ) reset_focus = true; break;
    }
    if( reset_focus ){
        SESSION::set_focused_admin( SESSION::FOCUS_NO );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
        admin = SESSION::FOCUS_NO;
    }

#ifdef _DEBUG
    std::cout << "Core::restore_focus admin = " << admin << std::endl;
#endif

    // ウィンドウが表示されているときはウィンドウのフォーカスを外す
    if( ! SESSION::get_embedded_img() ) IMAGE::get_admin()->set_command_immediately( "focus_out" );
    if( ! SESSION::get_embedded_mes() ) MESSAGE::get_admin()->set_command_immediately( "focus_out" );

    if( ! force ){ // 通常回復

        // フォーカス状態回復
        switch( admin )
        {
            case SESSION::FOCUS_SIDEBAR: BBSLIST::get_admin()->set_command_immediately( "restore_focus" ); break;
            case SESSION::FOCUS_BOARD: BOARD::get_admin()->set_command_immediately( "restore_focus" ); break;
            case SESSION::FOCUS_ARTICLE: ARTICLE::get_admin()->set_command_immediately( "restore_focus" ); break;
            case SESSION::FOCUS_IMAGE: IMAGE::get_admin()->set_command_immediately( "restore_focus" ); break;
            case SESSION::FOCUS_MESSAGE: MESSAGE::get_admin()->set_command_immediately( "restore_focus" ); break;
        }

    } else { // 強制的に回復

        int admin_sidebar = SESSION::focused_admin_sidebar();

        if( admin == SESSION::FOCUS_NO ){
            if( ! ARTICLE::get_admin()->empty() ) admin = admin_sidebar = SESSION::FOCUS_ARTICLE;
            else if( ! BOARD::get_admin()->empty() ) admin = admin_sidebar = SESSION::FOCUS_BOARD;
            else if( ! BBSLIST::get_admin()->empty() ) admin = admin_sidebar = SESSION::FOCUS_SIDEBAR;
        }

        SESSION::set_focused_admin( SESSION::FOCUS_NO );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );

        // adminの表示状態回復
        set_right_current_page( SESSION::notebook_main_page() );

        // フォーカス状態回復
        switch( admin ){
            case SESSION::FOCUS_SIDEBAR: switch_sidebar( std::string(), present ); break;
            case SESSION::FOCUS_BOARD: switch_board( present ); break;
            case SESSION::FOCUS_ARTICLE: switch_article( present ); break;
            case SESSION::FOCUS_IMAGE: switch_image( present ); break;
            case SESSION::FOCUS_MESSAGE: switch_message( present ); break;
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
    m_hpaned.get_ctrl().clock_in();
    m_vpaned_r.get_ctrl().clock_in();
    m_hpaned_r.get_ctrl().clock_in();
    m_vpaned_message.get_ctrl().clock_in();
   
    return true;
}


//
// 右ペーンのnotebookのタブの切替え
//
void Core::slot_switch_page( GtkNotebookPage*, guint )
{
    const bool present = false;
    int page = get_right_current_page();

#ifdef _DEBUG
    std::cout << "Core::slot_switch_page " << page << std::endl;
#endif

    switch( page ){

        case PAGE_ARTICLE: switch_article( present ); break;

        case PAGE_IMAGE: switch_image( present ); break;

        case PAGE_BOARD: switch_board( present ); break;
    }
}


// 右ペーンのnotebookのページ番号
int Core::get_right_current_page()
{
    int mode = SESSION::get_mode_pane();
    int page = m_notebook_right.get_current_page();

    if( mode == SESSION::MODE_2PANE ){

        // 2paneで画像ビューをウィンドウ表示している場合
        // 1 ページ目はIMAGEではなくてBOARDになる
        if( ! SESSION::get_embedded_img() && page == 1 ) page = PAGE_BOARD;
    }

    return page;
}


// 右ペーンのnotebookのページをセット
void Core::set_right_current_page( int page )
{
    // page が empty でないか調べる
    if( page == PAGE_ARTICLE && ARTICLE::get_admin()->empty() ){

        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE && ! BOARD::get_admin()->empty() ) page = PAGE_BOARD;
        else if( ! IMAGE::get_admin()->empty() ) page = PAGE_IMAGE;
        else return;
    }
    if( page == PAGE_BOARD && BOARD::get_admin()->empty() ){

        if( ! ARTICLE::get_admin()->empty() ) page = PAGE_ARTICLE;
        else if( ! IMAGE::get_admin()->empty() ) page = PAGE_IMAGE;
        else return;
    }
    if( page == PAGE_IMAGE && IMAGE::get_admin()->empty() ){

        if( ! ARTICLE::get_admin()->empty() ) page = PAGE_ARTICLE;
        else if( SESSION::get_mode_pane() == SESSION::MODE_2PANE && ! BOARD::get_admin()->empty() ) page = PAGE_BOARD;
        else return;
    }

    if( get_right_current_page() == page ) return;

    // 画像ビューをウィンドウ表示している場合
    if( ! SESSION::get_embedded_img() && page == PAGE_IMAGE ) return;

    if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

        // 2paneで画像ビューをウィンドウ表示している場合
        // 1 ページ目はIMAGEではなくてBOARDになる
        if( ! SESSION::get_embedded_img() && page == PAGE_BOARD ) page = 1;
    }
    else if( page == PAGE_BOARD ) return; // 2pane以外ではboardはnotebookに含まれない

    m_notebook_right.set_current_page( page );
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

    if( SESSION::is_dialog_shown() ) return true;

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

    restore_focus( false, false );

    return true;
}


//
// URL entryでenterを押した
//
void Core::slot_active_url()
{
    std::string url = m_toolbar->m_entry_url.get_text();
    if( !url.empty() ){

        if( url == "about:config" ) slot_aboutconfig();
        else CORE::core_set_command( "open_url", url );
    }
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

    const bool present = false;

    bool emp_img = ! ( SESSION::get_embedded_img() && ! IMAGE::get_admin()->empty() );
    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );

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
    if( url == URL_IMAGEADMIN && SESSION::get_embedded_img() ){

        hide_imagetab();

        // 空でないadminを前に出す
        if( SESSION::get_mode_pane() == SESSION::MODE_2PANE ){

            if( get_right_current_page() == PAGE_IMAGE ){
                if( ! ARTICLE::get_admin()->empty() ) switch_article( present );
                else if( ! BOARD::get_admin()->empty() ) switch_board( present );
            }
        }
        else if( ! ARTICLE::get_admin()->empty() ) switch_article( present );

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

            if( get_right_current_page() == PAGE_ARTICLE && emp_mes ) {

                if( BOARD::get_admin()->empty() && ! emp_img ) switch_image( present );
                else if( ! BOARD::get_admin()->empty() ) switch_board( present );
            }
        }
        else if( ! emp_img ) switch_image( present );

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){

            if( ! emp_mes ) focused_admin = SESSION::FOCUS_MESSAGE;
            else if( ! BOARD::get_admin()->empty() ) focused_admin = SESSION::FOCUS_BOARD;
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
                if( ! ARTICLE::get_admin()->empty() ) switch_article( present );
                else if( ! emp_img ) switch_image( present );
            }
        }

        // フォーカス切り替え
        if( focused_admin == SESSION::FOCUS_NO ){
            focused_admin = SESSION::FOCUS_SIDEBAR;
            SESSION::set_focused_admin_sidebar( SESSION::FOCUS_NO );
        }
    }

    // 埋め込みmessageビューが空になった
    else if( url == URL_MESSAGEADMIN && SESSION::get_embedded_mes() ){

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

        case SESSION::FOCUS_SIDEBAR: switch_sidebar( std::string(), present ); break;
        case SESSION::FOCUS_BOARD: switch_board( present ); break;
        case SESSION::FOCUS_ARTICLE: switch_article( present ); break;
        case SESSION::FOCUS_IMAGE: switch_image( present ); break;
        case SESSION::FOCUS_MESSAGE: switch_message( present ); break;
    }
}


//
// ビューのトグルボタンを上げ下げする
//
void Core::set_toggle_view_button()
{
    m_enable_menuslot = false;

    switch( SESSION::focused_admin() ){

        case SESSION::FOCUS_SIDEBAR:

            if( SESSION::get_bbslist_current_page() == 0 ){
                m_toolbar->m_button_bbslist.set_active( true );
                m_toolbar->m_button_favorite.set_active( false );
            }
            else{
                m_toolbar->m_button_bbslist.set_active( false );
                m_toolbar->m_button_favorite.set_active( true );
            }
            m_toolbar->m_button_board.set_active( false );
            m_toolbar->m_button_thread.set_active( false );
            m_toolbar->m_button_image.set_active( false );
            break;
            
        case SESSION::FOCUS_BOARD:

            if( ! BOARD::get_admin()->empty() ){
                m_toolbar->m_button_bbslist.set_active( false );
                m_toolbar->m_button_favorite.set_active( false );
                m_toolbar->m_button_board.set_active( true );
                m_toolbar->m_button_thread.set_active( false );
                m_toolbar->m_button_image.set_active( false );
            }
            else m_toolbar->m_button_board.set_active( false );

            break;

        case SESSION::FOCUS_ARTICLE:

            if( ! ARTICLE::get_admin()->empty() ){
                m_toolbar->m_button_bbslist.set_active( false );
                m_toolbar->m_button_favorite.set_active( false );
                m_toolbar->m_button_board.set_active( false );
                m_toolbar->m_button_thread.set_active( true );
                m_toolbar->m_button_image.set_active( false );
            } 
            else m_toolbar->m_button_thread.set_active( false );

            break;

        case SESSION::FOCUS_IMAGE:

            if( ! IMAGE::get_admin()->empty() ){
                m_toolbar->m_button_bbslist.set_active( false );
                m_toolbar->m_button_favorite.set_active( false );
                m_toolbar->m_button_board.set_active( false );
                m_toolbar->m_button_thread.set_active( false );
                m_toolbar->m_button_image.set_active( true );
            }
            else m_toolbar->m_button_image.set_active( false );

            break;

        case SESSION::FOCUS_MESSAGE:
                m_toolbar->m_button_bbslist.set_active( false );
                m_toolbar->m_button_favorite.set_active( false );
                m_toolbar->m_button_board.set_active( false );
                if( SESSION::get_embedded_mes() ) m_toolbar->m_button_thread.set_active( true );
                else m_toolbar->m_button_thread.set_active( false );
                m_toolbar->m_button_image.set_active( false );

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

    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );

    // スレ一覧ボタンの切り替え
    if( BOARD::get_admin()->empty() ) m_toolbar->m_button_board.set_sensitive( false );
    else m_toolbar->m_button_board.set_sensitive( true );

    // スレビューボタンの切り替え
    if( ! ARTICLE::get_admin()->empty() || ! emp_mes ) m_toolbar->m_button_thread.set_sensitive( true );
    else m_toolbar->m_button_thread.set_sensitive( false );

    // 画像ビューボタンの切り替え
    if( IMAGE::get_admin()->empty() ) m_toolbar->m_button_image.set_sensitive( false );
    else m_toolbar->m_button_image.set_sensitive( true );

    m_enable_menuslot = true;
}


//
// 右ペーンの最大化表示切り替え
//
void Core::toggle_maximize_rightpane()
{
    bool emp_board = BOARD::get_admin()->empty();
    bool emp_article = ARTICLE::get_admin()->empty();
    bool emp_img = ! ( SESSION::get_embedded_img() && ! IMAGE::get_admin()->empty() );
    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );

    // 埋め込みmessage
    if( SESSION::get_embedded_mes() ){
        if( ! emp_article && emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE1 );
        else if( emp_article && ! emp_mes ) m_vpaned_message.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE2 );
    }

    if( is_3pane() && CONFIG::get_expand_rpane() ){

        // スレ一覧を最大化
        if( ! emp_board && emp_article && emp_img && emp_mes ) get_rpctrl()->set_mode( SKELETON::PANE_MAX_PAGE1 );

        // スレビューを最大化
        else if( emp_board && ( ! emp_article || ! emp_img || ! emp_mes ) ) get_rpctrl()->set_mode( SKELETON::PANE_MAX_PAGE2 );

        // 戻す
        else if( ! emp_board && ( ! emp_article || ! emp_img || ! emp_mes ) ) get_rpctrl()->set_mode( SKELETON::PANE_NORMAL );
    }
}


//
// 各viewにスイッチ
//
void Core::switch_article( bool present )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_article present = " << present << std::endl;
#endif

    bool emp_mes = ! ( SESSION::get_embedded_mes() && ! MESSAGE::get_admin()->empty() );

    if( ! ARTICLE::get_admin()->empty() ){

        if( m_hpaned.get_ctrl().get_mode() == SKELETON::PANE_MAX_PAGE1 ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        if( SESSION::focused_admin() != SESSION::FOCUS_ARTICLE ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
            set_right_current_page( PAGE_ARTICLE );
        }

        ARTICLE::get_admin()->set_command_immediately( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_ARTICLE );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_ARTICLE );

        if( SESSION::get_embedded_img() ) SESSION::set_shown_win_img( false );
    }


    // articleは空だが、埋め込みmessageが表示されている場合
    else if( ! emp_mes ){
        switch_message( present );
        return;
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( present ) m_win_main.present();
}


void Core::switch_board( bool present )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_board\n";
#endif

    if( ! BOARD::get_admin()->empty() ){

        if( m_hpaned.get_ctrl().get_mode() == SKELETON::PANE_MAX_PAGE1 ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

        if( SESSION::focused_admin() != SESSION::FOCUS_BOARD ){

            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
            set_right_current_page( PAGE_BOARD );
        }

        BOARD::get_admin()->set_command_immediately( "focus_current_view" );
        SESSION::set_focused_admin( SESSION::FOCUS_BOARD );
        SESSION::set_focused_admin_sidebar( SESSION::FOCUS_BOARD );

        // 3paneの時はboardに切り替えても(フォーカスアウトしても)
        // 画像は表示されたままの時があることに注意
        if( SESSION::get_embedded_img() && SESSION::get_mode_pane() == SESSION::MODE_2PANE ) SESSION::set_shown_win_img( false );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( present ) m_win_main.present();
}


//
// url は表示するページ( URL_BBSLISTVIEW or URL_FAVORITEVIEW )
// urlが空の時はフォーカスを移すだけ
// present が true の時はメインウィンドウを前面に出す
//
void Core::switch_sidebar( const std::string& url, bool present )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_sidebar url = " << url
              << " current = " << SESSION::get_bbslist_current_url() << std::endl;
#endif

    if( ! BBSLIST::get_admin()->empty() ){

        if( SESSION::focused_admin() != SESSION::FOCUS_SIDEBAR ){
            FOCUS_OUT_ALL();
            ARTICLE::get_admin()->set_command( "delete_popup" );
        }

        // urlがフォーカスされていて、かつ他のadminがemptyで無いときは閉じる
        else if( SESSION::get_bbslist_current_url() == url && ! is_all_admin_empty() ){
            toggle_sidebar();
            return;
        }

        // 閉じていたら開く
        if( ! SESSION::show_sidebar() ) toggle_sidebar();

        // 右ペーンがemptyなら最大化
        else if( CONFIG::get_expand_sidebar() && is_all_admin_empty() ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_MAX_PAGE1 );

        if( ! url.empty() ) BBSLIST::get_admin()->set_command( "switch_view", url );

        BBSLIST::get_admin()->set_command_immediately( "focus_current_view" ); 
        SESSION::set_focused_admin( SESSION::FOCUS_SIDEBAR );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( present ) m_win_main.present();
}


void Core::switch_image( bool present )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_image\n";
#endif

    if( ! IMAGE::get_admin()->empty() ){

        if( SESSION::get_embedded_img() ){ // 埋め込み画像ビュー

            if( m_hpaned.get_ctrl().get_mode() == SKELETON::PANE_MAX_PAGE1 ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

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

        IMAGE::get_admin()->set_command_immediately( "focus_current_view" );
        if( SESSION::get_embedded_img() ) SESSION::set_shown_win_img( true );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( present && SESSION::get_embedded_img() ) m_win_main.present();
}


void Core::switch_message( bool present )
{
    if( SESSION::is_booting() ) return;
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "Core::switch_message\n";
#endif

    // 埋め込み書き込みビュー使用
    bool emb_mes = SESSION::get_embedded_mes();

    if( ! MESSAGE::get_admin()->empty() ){

        if( emb_mes ){ 

            if( m_hpaned.get_ctrl().get_mode() == SKELETON::PANE_MAX_PAGE1 ) m_hpaned.get_ctrl().set_mode( SKELETON::PANE_NORMAL );

            if( SESSION::focused_admin() != SESSION::FOCUS_MESSAGE ){

                FOCUS_OUT_ALL();
                ARTICLE::get_admin()->set_command( "delete_popup" );
                set_right_current_page( PAGE_ARTICLE );
            }

            SESSION::set_focused_admin( SESSION::FOCUS_MESSAGE );
            SESSION::set_focused_admin_sidebar( SESSION::FOCUS_MESSAGE );
        }

        MESSAGE::get_admin()->set_command_immediately( "focus_current_view" );
    }

    set_sensitive_view_button();
    set_toggle_view_button();
    toggle_maximize_rightpane();
    if( present && emb_mes ) m_win_main.present();
}


// 2paneの時にboard <-> article 切替え
void Core::toggle_article()
{
    const bool present = true;

    // 画像ウィンドウが表示されている場合
    if( ! SESSION::get_embedded_img() && SESSION::is_shown_win_img() ){
        if( SESSION::focused_admin() == SESSION::FOCUS_ARTICLE ) switch_article( present );
        else switch_board( present );
    }
    else if( SESSION::focused_admin() == SESSION::FOCUS_ARTICLE ) switch_board( present );
    else switch_article( present );
}


// 左移動
void Core::switch_leftview()
{
    const bool present = true;
    int next_admin = SESSION::focused_admin();

    // 画像ウィンドウが表示されている
    if( ! SESSION::get_embedded_img() && SESSION::is_shown_win_img() ) next_admin = SESSION::FOCUS_IMAGE;

    for(;;){

        if( next_admin == SESSION::FOCUS_IMAGE ) next_admin = SESSION::FOCUS_ARTICLE;
        else if( next_admin == SESSION::FOCUS_ARTICLE ) next_admin = SESSION::FOCUS_BOARD;
        else if( next_admin == SESSION::FOCUS_BOARD ) next_admin = SESSION::FOCUS_SIDEBAR;
        else break;

        if( next_admin == SESSION::FOCUS_SIDEBAR && ! BBSLIST::get_admin()->empty() ){
            switch_sidebar( std::string(), present );
            break;
        }
        else if( next_admin == SESSION::FOCUS_BOARD && ! BOARD::get_admin()->empty() ){
            switch_board( present );
            break;
        }
        else if( next_admin == SESSION::FOCUS_ARTICLE && ! ARTICLE::get_admin()->empty() ){
            switch_article( present );
            break;
        }
    }
}


// 右移動
void Core::switch_rightview()
{
    const bool present = true;
    int next_admin = SESSION::focused_admin();
    
    for(;;){

        if( next_admin == SESSION::FOCUS_SIDEBAR ) next_admin = SESSION::FOCUS_BOARD;
        else if( next_admin == SESSION::FOCUS_BOARD ) next_admin = SESSION::FOCUS_ARTICLE;
        else if( next_admin == SESSION::FOCUS_ARTICLE ) next_admin = SESSION::FOCUS_IMAGE;
        else break;

        if( next_admin == SESSION::FOCUS_BOARD && ! BOARD::get_admin()->empty() ){
            switch_board( present );
            break;
        }
        else if( next_admin == SESSION::FOCUS_ARTICLE && ! ARTICLE::get_admin()->empty() ){
            switch_article( present );
            break;
        }
        else if( next_admin == SESSION::FOCUS_IMAGE && ! IMAGE::get_admin()->empty() ){
            switch_image( present );
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
        size_t tmp_url_length = tmp_url.length();

        // urlの先頭と最後のの " を取る
        while( *tmp_url.c_str() == '\"' ) tmp_url = tmp_url.substr( 1 );
        while( tmp_url.c_str()[ tmp_url_length - 1 ] == '\"' )
        {
            tmp_url = tmp_url.substr( 0, tmp_url_length - 1 );
            --tmp_url_length;
        }

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
    HISTORY::get_history_manager()->append_thread( url, DBTREE::article_subject( url ), TYPE_THREAD );
}


void Core::set_history_board( const std::string& url )
{
    HISTORY::get_history_manager()->append_board( url, DBTREE::board_name( url ), TYPE_BOARD );
}

void Core::set_history_close( const std::string& url )
{
    HISTORY::get_history_manager()->append_close( url, DBTREE::article_subject( url ), TYPE_THREAD );
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
