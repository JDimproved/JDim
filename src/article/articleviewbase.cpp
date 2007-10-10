// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewbase.h"
#include "articleview.h"
#include "drawareamain.h"
#include "toolbar.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscx.h"

#include "dbtree/articlebase.h"
#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "skeleton/popupwin.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "command.h"
#include "session.h"
#include "viewfactory.h"
#include "sharedbuffer.h"
#include "prefdiagfactory.h"
#include "usrcmdmanager.h"
#include "compmanager.h"
#include "controlutil.h"
#include "controlid.h"

#include <sstream>

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif

#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif

using namespace ARTICLE;

#define PROTO_URL4REPORT "url4report://"


ArticleViewBase::ArticleViewBase( const std::string& url )
    : SKELETON::View( url ),
      m_url_article( url ),
      m_toolbar( 0 ),
      m_popup_win( NULL ),
      m_popup_shown( false ),
      m_enable_menuslot( true ),
      m_current_bm( 0 ),
      m_show_url4report( false )
{
#ifdef _DEBUG    
    std::cout << "ArticleViewBase::ArticleViewBase : " << get_url() << std::endl;
#endif

    // マウスジェスチャ可能
    set_enable_mg( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_ARTICLE );
}



ArticleViewBase::~ArticleViewBase()
{
#ifdef _DEBUG    
    std::cout << "ArticleViewBase::~ArticleViewBase : " << get_url() << std::endl;
#endif

    hide_popup( true );
    delete_popup();
}



//
// コピー用URL( readcgi型 )
//
// メインウィンドウのURLバーなどに表示する)
//
const std::string ArticleViewBase::url_for_copy()
{
    return DBTREE::url_readcgi( m_url_article, 0, 0 );
}


//
// タブのロック
//
void ArticleViewBase::lock()
{
    View::lock();
    if( m_toolbar ) m_toolbar->lock();
}


//
// タブのアンロック
//
void ArticleViewBase::unlock()
{
    View::unlock();
    if( m_toolbar ) m_toolbar->unlock();
}


JDLIB::RefPtr_Lock< DBTREE::ArticleBase >& ArticleViewBase::get_article()
{
    assert( m_article );
    return  m_article;
}


DrawAreaBase* ArticleViewBase::drawarea()
{
    assert( m_drawarea );
    return m_drawarea;
}


DrawAreaBase* ArticleViewBase::create_drawarea()
{
    return Gtk::manage( new ARTICLE::DrawAreaMain( m_url_article ) );
}


//
// セットアップ
//
// 各派生ビューで初期設定が済んだ後に呼ばれる
// 
void ArticleViewBase::setup_view()
{
#ifdef _DEBUG    
    std::cout << "ArticleViewBase::setup_view " << get_url() << " url_article = " << m_url_article << std::endl;
#endif
    
    m_article = DBTREE::get_article( m_url_article );
    m_drawarea = create_drawarea();
    assert( m_article );
    assert( m_drawarea );

    m_drawarea->sig_button_press().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_button_press ));
    m_drawarea->sig_button_release().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_button_release ));
    m_drawarea->sig_motion_notify().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_motion_notify ) );
    m_drawarea->sig_key_press().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_key_press ) );
    m_drawarea->sig_key_release().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_key_release ) );    
    m_drawarea->sig_scroll_event().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_scroll_event ));
    m_drawarea->sig_leave_notify().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_leave_notify ) );

    m_drawarea->sig_on_url().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_on_url ) );
    m_drawarea->sig_leave_url().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_leave_url ) );

    pack_widget();
    setup_action();

    show_all_children();    
}


//
// ツールバーやスクロールバーのパッキング
//
void ArticleViewBase::pack_widget()
{
    // ツールバーの設定
    m_toolbar = Gtk::manage( new ArticleToolBar( SESSION::get_show_article_toolbar() ) );
    m_toolbar->m_button_close.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::close_view ) );
    m_toolbar->m_button_reload.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_reload ) );
    m_toolbar->m_button_write.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_write ) );
    m_toolbar->m_button_delete.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_delete ) );        
    m_toolbar->m_button_board.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_open_board ) );
    m_toolbar->m_button_favorite.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_favorite ) );
    m_toolbar->m_button_stop.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::stop ) );
    m_toolbar->m_button_open_search.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_open_search ) );

    // 検索バー
    m_toolbar->m_entry_search.signal_activate().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_active_search ) );
    m_toolbar->m_entry_search.signal_operate().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_entry_operate ) );
    m_toolbar->m_button_close_search.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_close_search ) );
    m_toolbar->m_button_up_search.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_up_search ) );
    m_toolbar->m_button_down_search.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_down_search ) );
    m_toolbar->m_button_drawout_and.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_drawout_and ) );
    m_toolbar->m_button_drawout_or.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_drawout_or ) );
    m_toolbar->m_button_clear_hl.signal_clicked().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_push_claar_hl ) );

    pack_start( *m_toolbar, Gtk::PACK_SHRINK, 2 );
    pack_start( *m_drawarea, Gtk::PACK_EXPAND_WIDGET, 2 );
}




//
// アクション初期化
//
void ArticleViewBase::setup_action()
{
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "BookMark", "ブックマーク設定/解除"), sigc::mem_fun( *this, &ArticleViewBase::slot_bookmark ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く"), sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_current_url ) );
    action_group()->add( Gtk::Action::create( "CopyNAME", "名前コピー"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_name ) );
    action_group()->add( Gtk::Action::create( "CopyID", "IDコピー"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_id ) );
    action_group()->add( Gtk::Action::create( "Copy", "Copy"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_selection_str ) );
    action_group()->add( Gtk::Action::create( "WriteRes", "レスする" ),sigc::mem_fun( *this, &ArticleViewBase::slot_write_res ) );
    action_group()->add( Gtk::Action::create( "QuoteRes", "引用してレスする"),sigc::mem_fun( *this, &ArticleViewBase::slot_quote_res ) );
    action_group()->add( Gtk::Action::create( "QuoteSelectionRes", "引用してレスする"),sigc::mem_fun( *this, &ArticleViewBase::slot_quote_selection_res ) );
    action_group()->add( Gtk::Action::create( "CopyRes", "レスをコピー"),
                         sigc::bind< bool >( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), false ) );
    action_group()->add( Gtk::Action::create( "CopyResRef", "引用コピー"),
                         sigc::bind< bool >( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), true ) );
    action_group()->add( Gtk::Action::create( "Delete", "削除"), sigc::mem_fun( *this, &ArticleViewBase::delete_view ) );
    action_group()->add( Gtk::Action::create( "DeleteOpen", "スレ情報を消さずにスレ再取得"), sigc::mem_fun( *this, &ArticleViewBase::delete_open_view ) );
    action_group()->add( Gtk::Action::create( "Favorite", "お気に入りに登録する"), sigc::mem_fun( *this, &ArticleViewBase::slot_favorite ) );
    action_group()->add( Gtk::Action::create( "Preference", "スレのプロパティ..."), sigc::mem_fun( *this, &ArticleViewBase::show_preference ) );
    action_group()->add( Gtk::Action::create( "PreferenceImage", "画像のプロパティ..."), sigc::mem_fun( *this, &ArticleViewBase::slot_preferences_image ) );
    action_group()->add( Gtk::Action::create( "SaveDat", "datファイルを保存..."), sigc::mem_fun( *this, &ArticleViewBase::slot_save_dat ) );

    // 検索
    action_group()->add( Gtk::Action::create( "Search_Menu", "検索" ) );
    action_group()->add( Gtk::Action::create( "SearchCache_Menu", "キャッシュ内ログ検索" ) );
    action_group()->add( Gtk::Action::create( "SearchCacheLocal", "この板のログのみを検索"), sigc::mem_fun( *this, &ArticleViewBase::slot_search_cachelocal ) );
    action_group()->add( Gtk::Action::create( "SearchCacheAll", "キャッシュ内の全ログを検索") );
    action_group()->add( Gtk::Action::create( "ExecSearchCacheAll", "検索する"), sigc::mem_fun( *this, &ArticleViewBase::slot_search_cacheall ) );
    action_group()->add( Gtk::Action::create( "SearchTitle", "スレタイ検索"), sigc::mem_fun( *this, &ArticleViewBase::slot_search_title ) );

    // 抽出系
    action_group()->add( Gtk::Action::create( "Drawout_Menu", "抽出" ) );
    action_group()->add( Gtk::Action::create( "DrawoutWord", "キーワード抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_selection_str ) );
    action_group()->add( Gtk::Action::create( "DrawoutRes", "レス抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_res ) );
    action_group()->add( Gtk::Action::create( "DrawoutNAME", "名前抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_name ) );
    action_group()->add( Gtk::Action::create( "DrawoutID", "ID抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_id ) );
    action_group()->add( Gtk::Action::create( "DrawoutBM", "ブックマーク抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_bm ) );
    action_group()->add( Gtk::Action::create( "DrawoutURL", "URL抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_url ) );
    action_group()->add( Gtk::Action::create( "DrawoutRefer", "参照抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_refer ) );
    action_group()->add( Gtk::Action::create( "DrawoutAround", "周辺抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_around ) );
    action_group()->add( Gtk::Action::create( "DrawoutTmp", "テンプレート抽出"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_tmp ) );

    // あぼーん系
    action_group()->add( Gtk::Action::create( "AboneWord_Menu", "NG ワード" ) );
    action_group()->add( Gtk::Action::create( "AboneRes", "レスをあぼ〜んする"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_res ) );
    action_group()->add( Gtk::Action::create( "AboneID", "NG IDに追加"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_id ) );
    action_group()->add( Gtk::Action::create( "AboneName", "NG 名前に追加 (対象: ローカル)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name ) );
    action_group()->add( Gtk::Action::create( "AboneWord", "NG ワードに追加 (対象: ローカル)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_word ) );

    action_group()->add( Gtk::Action::create( "AboneNameBoard", "NG 名前に追加 (対象: 板)" ) );
    action_group()->add( Gtk::Action::create( "SetAboneNameBoard", "追加する"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name_board ) );
    action_group()->add( Gtk::Action::create( "AboneWordBoard", "NG ワードに追加 (対象: 板)" ) );
    action_group()->add( Gtk::Action::create( "SetAboneWordBoard", "追加する"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_word_board ) );

    action_group()->add( Gtk::Action::create( "GlobalAboneName", "NG 名前に追加 (対象: 全体)" ) );
    action_group()->add( Gtk::Action::create( "SetGlobalAboneName", "追加する"), sigc::mem_fun( *this, &ArticleViewBase::slot_global_abone_name ) );
    action_group()->add( Gtk::Action::create( "GlobalAboneWord", "NG ワードに追加 (対象: 全体)" ) );
    action_group()->add( Gtk::Action::create( "SetGlobalAboneWord", "追加する"), sigc::mem_fun( *this, &ArticleViewBase::slot_global_abone_word ) );

    action_group()->add( Gtk::ToggleAction::create( "TranspAbone", "透明あぼ〜ん", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp ) );
    action_group()->add( Gtk::ToggleAction::create( "TranspChainAbone", "透明/連鎖あぼ〜ん", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp_chain ) );


    // 移動系
    action_group()->add( Gtk::Action::create( "Move_Menu", "移動" ) );
    action_group()->add( Gtk::Action::create( "Home", "Home"), sigc::mem_fun( *this, &ArticleViewBase::goto_top ) );
    action_group()->add( Gtk::Action::create( "GotoNew", "GotoNew"), sigc::mem_fun( *this, &ArticleViewBase::goto_new ) );
    action_group()->add( Gtk::Action::create( "End", "End"), sigc::mem_fun( *this, &ArticleViewBase::goto_bottom ) );
    action_group()->add( Gtk::Action::create( "PreBookMark", "PreBookMark"), sigc::mem_fun( *this, &ArticleViewBase::slot_pre_bm ) );
    action_group()->add( Gtk::Action::create( "NextBookMark", "NextBookMark"), sigc::mem_fun( *this, &ArticleViewBase::slot_next_bm ) );
    action_group()->add( Gtk::Action::create( "Jump", "ジャンプ"), sigc::mem_fun( *this, &ArticleViewBase::slot_jump ) );

    // 画像系
    action_group()->add( Gtk::Action::create( "Cancel_Mosaic", "モザイク解除"), sigc::mem_fun( *this, &ArticleViewBase::slot_cancel_mosaic ) );
    action_group()->add( Gtk::Action::create( "ShowLargeImg", "サイズが大きい画像を表示"),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_show_large_img ) );
    action_group()->add( Gtk::ToggleAction::create( "ProtectImage", "キャッシュを保護する", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_protectimage ) );
    action_group()->add( Gtk::Action::create( "DeleteImage_Menu", "削除" ) );    
    action_group()->add( Gtk::Action::create( "DeleteImage", "削除する"), sigc::mem_fun( *this, &ArticleViewBase::slot_deleteimage ) );
    action_group()->add( Gtk::Action::create( "SaveImage", "名前を付けて保存..."), sigc::mem_fun( *this, &ArticleViewBase::slot_saveimage ) );
    action_group()->add( Gtk::ToggleAction::create( "AboneImage", "画像をあぼ〜んする", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_abone_img ) );

    // ユーザコマンド
    const int usrcmd_size = CORE::get_usrcmd_manager()->get_size();
    for( int i = 0; i < usrcmd_size; ++i ){
        std::string cmdname = "usrcmd" + MISC::itostr( i );
        std::string cmdlabel = CORE::get_usrcmd_manager()->get_label( i );
        Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( cmdname, cmdlabel );
        action_group()->add( action, sigc::bind< int >( sigc::mem_fun( *this, &ArticleViewBase::slot_usrcmd ), i ) );
    }
    bool submenu_usrcmd = CONFIG::get_max_show_usrcmd() <= usrcmd_size;
    if( submenu_usrcmd ) action_group()->add( Gtk::Action::create( "Usrcmd_Menu", "ユーザコマンド" ) );

    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

    // レイアウト
    Glib::ustring str_ui =
    "<ui>"

    // 削除ボタン押したときのポップアップ
    "<popup name='popup_menu_delete'>"
    "<menuitem action='Delete'/>"
    "<separator/>"
    "<menuitem action='DeleteOpen'/>"
    "</popup>"

    // 壊れていますをクリックしたときのポップアップ
    "<popup name='popup_menu_broken'>"
    "<menuitem action='DeleteOpen'/>"
    "</popup>"

    // レス番号をクリックしたときのメニュー
    "<popup name='popup_menu_res'>"
    "<menuitem action='BookMark'/>"
    "<separator/>"

    "<menu action='Drawout_Menu'>"
    "<menuitem action='DrawoutRefer'/>"
    "<menuitem action='DrawoutAround'/>"
    "<menuitem action='DrawoutRes'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='WriteRes'/>"
    "<menuitem action='QuoteRes'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyRes'/>"
    "<menuitem action='CopyResRef'/>"
    "<separator/>"

    "<menuitem action='AboneRes'/>"
    "</popup>"

    // レスアンカーをクリックしたときのメニュー
    "<popup name='popup_menu_anc'>"
    "<menuitem action='Jump'/>"
    "<menuitem action='DrawoutAround'/>"
    "<menuitem action='DrawoutRes'/>"
    "</popup>"

    // IDをクリックしたときのメニュー
    "<popup name='popup_menu_id'>"
    "<menuitem action='DrawoutID'/>"
    "<menuitem action='CopyID'/>"
    "<separator/>"
    "<menuitem action='AboneID'/>"
    "</popup>"

    // 名前をクリックしたときのメニュー
    "<popup name='popup_menu_name'>"
    "<menuitem action='DrawoutNAME'/>"
    "<menuitem action='CopyNAME'/>"
    "<separator/>"

    "<menuitem action='AboneName'/>"

    "<menu action='AboneNameBoard'>"
    "<menuitem action='SetAboneNameBoard'/>"
    "</menu>"

    "<menu action='GlobalAboneName'>"
    "<menuitem action='SetGlobalAboneName'/>"
    "</menu>"

    "</popup>"

    // あぼーんをクリックしたときのメニュー
    "<popup name='popup_menu_abone'>"
    "<menuitem action='TranspAbone'/>"
    "<menuitem action='TranspChainAbone'/>"
    "</popup>"

    // 通常の右クリックメニュー
    "<popup name='popup_menu'>"

    "<menu action='Drawout_Menu'>"
    "<menuitem action='DrawoutWord'/>"
    "<menuitem action='DrawoutBM'/>"
    "<menuitem action='DrawoutURL'/>"
    "<menuitem action='DrawoutTmp'/>"
    "</menu>"

    "<menu action='Move_Menu'>"
    "<menuitem action='Home'/>"
    "<menuitem action='End'/>"
    "<menuitem action='GotoNew'/>"
    "<menuitem action='PreBookMark'/>"
    "<menuitem action='NextBookMark'/>"
    "</menu>"

    "<menu action='AboneWord_Menu'>"

    "<menuitem action='AboneWord'/>"

    "<menu action='AboneWordBoard'>"
    "<menuitem action='SetAboneWordBoard'/>"
    "</menu>"

    "<menu action='GlobalAboneWord'>"
    "<menuitem action='SetGlobalAboneWord'/>"
    "</menu>"

    "</menu>"

    "<menu action='Search_Menu'>"

    "<menu action='SearchCache_Menu'>"
    "<menuitem action='SearchCacheLocal'/>"
    "<menu action='SearchCacheAll'>"
    "<menuitem action='ExecSearchCacheAll'/>"
    "</menu>"
    "</menu>"

    "<menuitem action='SearchTitle' />"

    "</menu>"

    "<separator/>"
    "<menuitem action='QuoteSelectionRes' />"

    "<separator/>"
    "<menuitem action='OpenBrowser'/>";

    // ユーザコマンド
    if( submenu_usrcmd ) str_ui += "<menu action='Usrcmd_Menu'>";
    for( int i = 0; i < usrcmd_size; ++i ) str_ui += "<menuitem action='usrcmd" + MISC::itostr( i ) + std::string( "'/>" );
    if( submenu_usrcmd ) str_ui += "</menu>";

    Glib::ustring str_ui2 = 

    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='Copy'/>"

    "<separator/>"
    "<menuitem action='SaveDat'/>"

    "<separator/>"

    "<menuitem action='Preference'/>"

    "</popup>"

    // 画像メニュー
    "<popup name='popup_menu_img'>"
    "<menuitem action='Cancel_Mosaic'/>"
    "<menuitem action='ShowLargeImg'/>"
    "<separator/>"
    "<menuitem action='OpenBrowser'/>";

    // ユーザコマンド
    if( submenu_usrcmd ) str_ui2 += "<menu action='Usrcmd_Menu'>";
    for( int i = 0; i < usrcmd_size; ++i ) str_ui2 += "<menuitem action='usrcmd" + MISC::itostr( i ) + std::string( "'/>" );
    if( submenu_usrcmd ) str_ui2 += "</menu>";

    Glib::ustring str_ui3 = 
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<separator/>"
    "<menuitem action='SaveImage'/>"
    "<separator/>"
    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteImage_Menu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='AboneImage'/>"
    "<separator/>"
    "<menuitem action='PreferenceImage'/>"
    "</popup>"

    "</ui>";

    str_ui += str_ui2 + str_ui3;

    ui_manager()->add_ui_from_string( str_ui );

    // ポップアップメニューにショートカットキーやマウスジェスチャを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );
}



//
// クライアント領域幅
//
const int ArticleViewBase::width_client()
{
#ifdef _DEBUG
    if( m_drawarea ) std::cout << "ArticleViewBase::width_client : " << m_drawarea->width_client() << std::endl;
#endif

    if( m_drawarea ) return m_drawarea->width_client();

    return SKELETON::View::width_client();
}


//
// クライアント領高さ
//
const int ArticleViewBase::height_client()
{
#ifdef _DEBUG
    if( m_drawarea ) std::cout << "ArticleViewBase::height_client : " << m_drawarea->height_client() << std::endl;
#endif

    if( m_drawarea ) return m_drawarea->height_client();

    return SKELETON::View::height_client();
}


// アイコンのID取得
const int ArticleViewBase::get_icon( const std::string& iconname )
{
    int id = ICON::NONE;

    if( iconname == "default" ) id = ICON::THREAD;
    if( iconname == "loading" ) id = ICON::LOADING;
    if( iconname == "loading_stop" ) id = ICON::LOADING_STOP;
    if( iconname == "old" ) id = ICON::THREAD_OLD;
    if( iconname == "update" ) id = ICON::THREAD_UPDATE;  // 更新チェックしで更新があった場合
    if( iconname == "updated" ) id = ICON::THREAD_UPDATED;

    return id;
}


//
// コマンド
//
bool ArticleViewBase::set_command( const std::string& command, const std::string& arg )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::set_command " << get_url() << std::endl
              << "command = " << command << std::endl;
#endif

    if( command == "append_dat" ) append_dat( arg, -1 );
    else if( command == "append_html" ) append_html( arg );
    else if( command == "clear_screen" )
    {
    	if( m_drawarea ) m_drawarea->clear_screen();
    }
    else if( command == "goto_num" ) goto_num( atoi( arg.c_str() ) );
    else if( command == "delete_popup" ) delete_popup();

    return true;
}


//
// クロック入力
//
// クロックタイマーの本体はコアが持っていて、定期的にadminがアクティブなviewにクロック入力を渡す
//
void ArticleViewBase::clock_in()
{
    assert( m_drawarea );

    View::clock_in();

    // ポップアップが出てたらそっちにクロックを回す
    if( is_popup_shown() && m_popup_win->view() ){
        m_popup_win->view()->clock_in();
        return;
    }

    m_drawarea->clock_in();

    return;
}



//
// 再読み込み
//
void ArticleViewBase::reload()
{
    // オフライン
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    CORE::core_set_command( "open_article", m_url_article , "left", "" );
}


//
// ロード停止
//
void ArticleViewBase::stop()
{
    assert( m_article );
    m_article->stop_load();
}


//
// 再描画
//
void ArticleViewBase::redraw_view()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::redraw_view\n";
#endif

    assert( m_drawarea );
    m_drawarea->redraw_view();

    // ポップアップが表示されていたらポップアップも再描画
    if( is_popup_shown() ){
        ArticleViewBase* popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
        if( popup_article ) return popup_article->redraw_view();
    }
}


//
// フォーカスイン
//
void ArticleViewBase::focus_view()
{
    assert( m_drawarea );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::focus_view\n";
#endif

    m_drawarea->focus_view();
    redraw_view();
}


//
// フォーカスアウト
//
void ArticleViewBase::focus_out()
{
    SKELETON::View::focus_out();

#ifdef _DEBUG
    std::cout << "ArticleViewBase::focus_out " << get_url() << std::endl;
#endif

    m_drawarea->focus_out();

    // フォーカスアウトした瞬間に、子ポップアップが表示されていて、かつ
    // ポインタがその上だったらポップアップは消さない
    if( is_mouse_on_popup() ) return;

    hide_popup();
}


//
// 閉じる
//
void ArticleViewBase::close_view()
{
    if( m_article->is_loading() ){
        SKELETON::MsgDiag mdiag( NULL, "読み込み中です" );
        mdiag.run();
        return;
    }

    ARTICLE::get_admin()->set_command( "close_currentview" );
}


//
// 記事削除
//
void ArticleViewBase::delete_view()
{
    CORE::core_set_command( "delete_article", m_url_article );
    ARTICLE::get_admin()->set_command( "switch_admin" );
}


//
// スレ再取得
//
void ArticleViewBase::delete_open_view()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    if( DBTREE::article_status( m_url_article ) & STATUS_OLD ){
        SKELETON::MsgDiag mdiag( NULL, "DAT落ちしています。\n\nログが消える恐れがあります。実行しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_NO );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;
    }

    CORE::core_set_command( "delete_article", m_url_article, "reopen", MISC::itostr( drawarea()->get_seen_current() ) );
}


//
// マウスポインタをポップアップの上に移動する
// 
void ArticleViewBase::warp_pointer_to_popup()
{
    if( is_popup_shown() ){

        const int mrg = 32;

        int x, y;
        get_pointer( x, y );

        m_popup_win->get_pointer( x, y );

        if( x < mrg ) x = mrg;
        else if( x > m_popup_win->get_width() - mrg ) x = m_popup_win->get_width() - mrg;

        if( y < 0 ) y = mrg;
        else y = m_popup_win->get_height() - mrg;

        MISC::WarpPointer( get_window(), m_popup_win->get_window(), x, y );
    }
}


//
// viewの操作
//
void ArticleViewBase::operate_view( const int& control )
{
    assert( m_drawarea );

    if( control == CONTROL::None ) return;

    // スクロール系操作
    if( m_drawarea->set_scroll( control ) ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::operate_view control = " << control << std::endl;
#endif    

    // その他の処理
    switch( control ){
            
        // リロード
        case CONTROL::Reload:
            slot_push_reload();
            break;

            // コピー
        case CONTROL::Copy:
            slot_copy_selection_str();
            break;

            // 全て選択
        case CONTROL::SelectAll:
            slot_select_all();
            break;
        
            // 検索
        case CONTROL::Search:
            open_searchbar( false );
            break;

        case CONTROL::SearchInvert:
            open_searchbar( true );
            break;

        case CONTROL::SearchNext:
            slot_push_down_search();
            break;

        case CONTROL::SearchPrev:
            slot_push_up_search();
            break;

            // 閉じる
        case CONTROL::Quit:
            ARTICLE::get_admin()->set_command( "close_currentview" );
            break;

            // 書き込み
        case CONTROL::WriteMessage:
            slot_push_write();
            break;

            // 削除
        case CONTROL::Delete:
        {
            SKELETON::MsgDiag mdiag( NULL, "ログを削除しますか？\n\n「スレ再取得」を押すとあぼ〜んなどのスレ情報を削除せずにスレを再取得します。",
                                     false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
            mdiag.add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );
            mdiag.add_button( Gtk::Stock::YES, Gtk::RESPONSE_YES );
            mdiag.add_button( "スレ再取得", Gtk::RESPONSE_YES + 100 );
            mdiag.set_default_response( Gtk::RESPONSE_YES );
            int ret = mdiag.run();
            if( ret == Gtk::RESPONSE_YES ) delete_view();
            else if( ret == Gtk::RESPONSE_YES + 100 ) delete_open_view();
            break;
        }

            // Board に切り替え
        case CONTROL::Left:
            CORE::core_set_command( "switch_leftview" );
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

            // image に切り替え
        case CONTROL::Right:
            CORE::core_set_command( "switch_rightview" );
            break;

            // 右、左のタブに切り替え
        case CONTROL::TabLeft:
            ARTICLE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            ARTICLE::get_admin()->set_command( "tab_right" );
            break;

        // ポップアップメニュー表示
        case CONTROL::ShowPopupMenu:
            show_popupmenu( "", true );
            break;

            // ブックマーク移動
        case CONTROL::PreBookMark:
            slot_pre_bm();
            break;

        case CONTROL::NextBookMark:
            slot_next_bm();
            break;

            // 親にhideを依頼する and ローディング停止
        case CONTROL::StopLoading:
            stop();
            sig_hide_popup().emit();
            break;

            // サイドバー表示/非表示
        case CONTROL::ShowSideBar:
            CORE::core_set_command( "toggle_sidebar" );
            break;

            // メニューバー表示/非表示
        case CONTROL::ShowMenuBar:
            CORE::core_set_command( "toggle_menubar" );
            break;
    }
}


//
// 一番上へ
//
void ArticleViewBase::goto_top()
{
    assert( m_drawarea );
    m_drawarea->goto_top();
}



//
// 一番下へ
//
void ArticleViewBase::goto_bottom()
{
    assert( m_drawarea );
    m_drawarea->goto_bottom();
}



//
// num番へジャンプ
//
void ArticleViewBase::goto_num( int num )
{
    assert( m_drawarea );
    m_drawarea->set_jump_history();
    m_drawarea->goto_num( num );
}


//
// ツールバー表示切り替え
//
void ArticleViewBase::toggle_toolbar()
{
    if( m_toolbar ){

        if( SESSION::get_show_article_toolbar() ) m_toolbar->show_toolbar();
        else m_toolbar->hide_toolbar();
    }
}


//
// 新着に移動
//
void ArticleViewBase::goto_new()
{
    assert( m_drawarea );
    m_drawarea->goto_new();
}



//
// 検索バーを開く
//
void ArticleViewBase::open_searchbar( bool invert )
{
    if( m_toolbar ){ 
        m_toolbar->show_searchbar(); 
        m_search_invert = invert;
        m_toolbar->m_entry_search.grab_focus(); 
    }
}



//
// 検索バーを開くボタンを押した
//
void ArticleViewBase::slot_push_open_search()
{
    if( ! m_toolbar ) return;

    if( ! m_toolbar->m_searchbar_shown ) open_searchbar( false );
    else slot_push_close_search();
}



//
// 検索バーを隠すボタンを押した
//
void ArticleViewBase::slot_push_close_search()
{
    if( m_toolbar ){
        m_toolbar->hide_searchbar();
        m_drawarea->focus_view();
    }
}


//
// 前を検索
//
void ArticleViewBase::slot_push_up_search()
{
    m_search_invert = true;
    slot_active_search();
    redraw_view();
}



//
// 次を検索
//
void ArticleViewBase::slot_push_down_search()
{
    m_search_invert = false;
    slot_active_search();
    redraw_view();
}



//
// 別のタブを開いてキーワード抽出 (AND)
//
void ArticleViewBase::slot_push_drawout_and()
{
    if( ! m_toolbar ) return;
    if( m_toolbar->m_entry_search.completion() ) return;

    std::string query = m_toolbar->m_entry_search.get_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
}


//
// 別のタブを開いてキーワード抽出 (OR)
//
void ArticleViewBase::slot_push_drawout_or()
{
    if( ! m_toolbar ) return;
    if( m_toolbar->m_entry_search.completion() ) return;

    std::string query = m_toolbar->m_entry_search.get_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "true" );
}


//
// ハイライト解除
//
void ArticleViewBase::slot_push_claar_hl()
{
    assert( m_drawarea );
    if( m_query.empty() ) return;

    m_query = std::string();
    m_drawarea->clear_highlight();
}


//
// リロードボタン
//
void ArticleViewBase::slot_push_reload()
{
    if( CONFIG::get_reload_allthreads() ) ARTICLE::get_admin()->set_command( "reload_all_tabs" );
    else reload();
}


//
// メッセージ書き込みボタン
//
void ArticleViewBase::slot_push_write()
{
    CORE::core_set_command( "open_message" ,m_url_article, std::string() );
}



//
// 削除ボタン
//
void ArticleViewBase::slot_push_delete()
{
    show_popupmenu( "popup_menu_delete", false );
}


//
// 板を開くボタン
//
void ArticleViewBase::slot_push_open_board()
{
    CORE::core_set_command( "open_board", DBTREE::url_subject( m_url_article ), "true",
                            "auto" // オートモードで開く
        );
}





//
// プロパティ表示
//
void ArticleViewBase::show_preference()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_preference\n";
#endif

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLE, m_url_article );
    pref->run();
    delete pref;
}



//
// 画像プロパティ表示
//
void ArticleViewBase::slot_preferences_image()
{
    if( m_url_tmp.empty() ) return;
    std::string url = m_url_tmp;

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_IMAGE, url );
    pref->run();
    delete pref;
}




//
// 前のブックマークに移動
//
void ArticleViewBase::slot_pre_bm()
{
    assert( m_article );

    if( m_current_bm == 0 ) m_current_bm = m_drawarea->get_seen_current();

    for( int i = m_current_bm -1 ; i >= 1 ; --i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i );
            m_current_bm = i;
            return;
        }
    }

    for( int i = m_article->get_number_load() ; i > m_current_bm ; --i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i );
            m_current_bm = i;
            return;
        }
    }
}



//
// 後ろのブックマークに移動
//
void ArticleViewBase::slot_next_bm()
{
    assert( m_article );

    if( m_current_bm == 0 ) m_current_bm = m_drawarea->get_seen_current();

    for( int i = m_current_bm + 1; i <= m_article->get_number_load() ; ++i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i );
            m_current_bm = i;
            return;
        }
    }

    for( int i = 1; i <= m_current_bm ; ++i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i );
            m_current_bm = i;
            return;
        }
    }
}



//
// ジャンプ
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_jump()
{
    CORE::core_set_command( "open_article", m_url_article , "true", "auto", m_str_num );
}


//
// dat 保存
//
void ArticleViewBase::slot_save_dat()
{
    m_article->save_dat( std::string() );
}


//
// 荒らし報告用のURLリストをHTML形式で取得
//
std::string ArticleViewBase::get_html_url4report( std::list< int >& list_resnum )
{
    std::string html;

    std::list < int >::iterator it = list_resnum.begin();
    for( ; it != list_resnum.end(); ++it ){

        int num = (*it);
        std::string time_str = m_article->get_time( num );
        std::string id_str = m_article->get_id_name( num );

        html += url_for_copy() + MISC::itostr( num );
        if( ! time_str.empty() ) html += " " + MISC::remove_str_regex( time_str, "\\([^\\)]+\\)" ); // 曜日を取り除く
        if( ! id_str.empty() ) html += " ID:" + id_str.substr( strlen( PROTO_ID ) );
        html += "<br>";
    }

    return html;
}


//
// レスを抽出して表示
//
// num は "from-to"　の形式。"a+b"の時はaとbを連結する(bのヘッダ行を取り除く)
//
// (例) 3から10を表示したいなら "3-10"
//      3と4を連結して表示したい時は "3+4"
//
// show_title == trueの時は 板名、スレ名を表示
// 
void ArticleViewBase::show_res( const std::string& num, bool show_title )
{
    assert( m_article );

    // 板名、スレ名
    if( show_title ){

        std::string html;
        std::string tmpstr = DBTREE::board_name( m_url_article );
        if( ! tmpstr.empty() ) html += "[ " + tmpstr + " ] ";

        tmpstr = DBTREE::article_subject( m_url_article );
        if( ! tmpstr.empty() ) html += tmpstr;

        if( ! html.empty() ) append_html( html );
    }

    std::list< bool > list_joint;
    std::list< int > list_resnum = m_article->get_res_str_num( num, list_joint );

    if( !list_resnum.empty() ) append_res( list_resnum, list_joint );
    else if( !show_title ) append_html( "未取得レス" );
}


//
// 名前 で抽出して表示
// 
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::show_name( const std::string& name, bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_name " << name << std::endl;
#endif
    
    std::list< int > list_resnum = m_article->get_res_name( name );       

    std::ostringstream comment;
    comment << "名前：" << name << "  " << list_resnum.size() << " 件";

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << " <a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy() << MISC::intlisttostr( list_resnum ) << "<br>";
    }

    append_html( comment.str() );
    if( ! list_resnum.empty() ) append_res( list_resnum );
}


//
// ID で抽出して表示
// 
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::show_id( const std::string& id_name, bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_id " << id_name << std::endl;
#endif
    
    std::list< int > list_resnum = m_article->get_res_id_name( id_name );       

    std::ostringstream comment;
    comment << "ID:" << id_name.substr( strlen( PROTO_ID ) ) << "  " << list_resnum.size() << " 件";

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << " <a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy() << MISC::intlisttostr( list_resnum ) << "<br>";
    }
      
    append_html( comment.str() );
    if( ! list_resnum.empty() ) append_res( list_resnum );
}



//
// ブックマークを抽出して表示
//
void ArticleViewBase::show_bm()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_bm " << std::endl;
#endif
    
    std::list< int > list_resnum = m_article->get_res_bm();

    if( ! list_resnum.empty() ) append_res( list_resnum );
    else append_html( "ブックマークはセットされていません" );
}




//
// URLを含むレスを抽出して表示
// 
void ArticleViewBase::show_res_with_url()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_res_with_url\n";
#endif

    std::list< int > list_resnum = m_article->get_res_with_url();

    if( ! list_resnum.empty() ) append_res( list_resnum );
    else append_html( "リンクを含むスレはありません" );
}



//
// num 番のレスを参照してるレスを抽出して表示
// 
void ArticleViewBase::show_refer( int num )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_refer " << num << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_reference( num );

    // num 番は先頭に必ず表示
    list_resnum.push_front( num );

    append_res( list_resnum );
}




//
// キーワードで抽出して表示
// 
// mode_or = true の時は or 検索
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::drawout_keywords( const std::string& query, bool mode_or, bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::drawout_keywords " << query << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_query( query, mode_or );         

    std::ostringstream comment;
    comment << query << "  " << list_resnum.size() << " 件";

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << " <a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy() << MISC::intlisttostr( list_resnum ) << "<br>";
    }

    append_html( comment.str() );

    if( ! list_resnum.empty() ){

        append_res( list_resnum );

        // ハイライト表示
        std::list< std::string > list_query = MISC::split_line( query );
        m_drawarea->search( list_query, false );
    }
}



//
// html をappend
//
void ArticleViewBase::append_html( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::append_html html = " << html << std::endl;
#endif

    assert( m_drawarea );
    m_drawarea->append_html( html );
    redraw_view();
}



//
// dat をレス番号 num 番として append
//
// num < 0 の時は現在の最大スレ番号の後に追加
//
void ArticleViewBase::append_dat( const std::string& dat, int num )
{
    assert( m_drawarea );
    if( num < 0 ) num = get_article()->get_number_load() +1;
    m_drawarea->append_dat( dat, num );
    redraw_view();
}


//
// リストで指定したレスの表示
//
void ArticleViewBase::append_res( std::list< int >& list_resnum )
{
    assert( m_drawarea );
    m_drawarea->append_res( list_resnum );
    redraw_view();
}


//
// リストで指定したレスを表示(連結情報付き)
//
// list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
//
void ArticleViewBase::append_res( std::list< int >& list_resnum, std::list< bool >& list_joint )
{
    assert( m_drawarea );
    m_drawarea->append_res( list_resnum, list_joint );
    redraw_view();
}



//
// drawareaから出た
//
bool ArticleViewBase::slot_leave_notify( GdkEventCrossing* event )
{
    // クリックしたときやホイールを回すと event->mode に　GDK_CROSSING_GRAB
    // か GDK_CROSSING_UNGRAB がセットされてイベントが発生する場合がある
    if( event->mode == GDK_CROSSING_GRAB ) return false;
    if( event->mode == GDK_CROSSING_UNGRAB ) return false;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_leave_drawarea\n";
#endif

    focus_out();

    return false;
}



//
// drawarea のクリックイベント
//
bool ArticleViewBase::slot_button_press( std::string url, int res_number, GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_button_press url = " << get_url() << std::endl;
#endif

    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    ARTICLE::get_admin()->set_command( "switch_admin" );

    return true;
}



//
// drawarea でのマウスボタンのリリースイベント
//
bool ArticleViewBase::slot_button_release( std::string url, int res_number, GdkEventButton* event )
{
    /// マウスジェスチャ
    int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない 
    if( get_control().MG_wheel_end( event ) ) return true;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_button_release mg = " << mg << " url = " << get_url()
              << " url = " << url << " res = " << res_number
              << std::endl;
#endif

    if( event->type == GDK_BUTTON_RELEASE ){

        if( ! is_mouse_on_popup() ){

            // マウスジェスチャ
            if( mg != CONTROL::None && enable_mg() ) operate_view( mg );
                
            else if( ! click_url( url, res_number, event ) ){

                if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){
                    show_popupmenu( url, false );
                }
                else operate_view( get_control().button_press( event ) );
            }
        }
    }

    return true;
}



//
// drawarea でマウスが動いた
//
bool ArticleViewBase::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

    return true;
}



//
// drawareaのキープレスイベント
//
bool ArticleViewBase::slot_key_press( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_key_press\n";
#endif

    // ポップアップはキーフォーカスを取れないので親からキー入力を送ってやる
    ArticleViewBase* popup_article = NULL;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article ) return popup_article->slot_key_press( event );

    int key = get_control().key_press( event );

    if( key != CONTROL::None ) operate_view( key );
    else release_keyjump_key( event->keyval );

    return true;
}



//
// drawareaのキーリリースイベント
//
bool ArticleViewBase::slot_key_release( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_key_release\n";
#endif

    // ポップアップはキーフォーカスを取れないのでキー入力を送ってやる
    ArticleViewBase* popup_article = NULL;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article ) return popup_article->slot_key_release( event );
   
    return true;
}



//
// drawareaのマウスホイールイベント
//
bool ArticleViewBase::slot_scroll_event( GdkEventScroll* event )
{
    // ポップアップしているときはそちらにイベントを送ってやる
    ArticleViewBase* popup_article = NULL;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article ) return popup_article->slot_scroll_event( event );

    // ホイールマウスジェスチャ
    int control = get_control().MG_wheel_scroll( event );
    if( enable_mg() && control != CONTROL::None ){
        operate_view( control );
        return true;
    }

    m_drawarea->wheelscroll( event );
    return true;
}



//
// リンクの上にポインタが来た
//
// drawareaのsig_on_url()シグナルとつなぐ
//
void ArticleViewBase::slot_on_url( std::string url, int res_number )
{

#ifdef _DEBUG    
    std::cout << "ArticleViewBase::slot_on_url " << url << std::endl;
#endif

    CORE::VIEWFACTORY_ARGS args;
    SKELETON::View* view_popup = NULL;
    int margin_popup = CONFIG::get_margin_popup();

    // 画像ポップアップ
    if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ){

        // あぼーん
        if( DBIMG::get_abone( url ) ){
            args.arg1 = "あぼ〜んされています";
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPHTML, m_url_article, args );
        }

        else if ( DBIMG::is_loading( url ) || DBIMG::get_code( url ) != HTTP_INIT ) {

#ifdef _DEBUG
            std::cout << "image " << DBIMG::get_code( url ) << " " << DBIMG::is_loading( url ) << "\n";
#endif

            view_popup = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP,  url );
            margin_popup = CONFIG::get_margin_imgpopup();
        }
    }

    // レスポップアップ
    else if( url.find( PROTO_ANCHORE ) == 0 ){
      
        args.arg1 = url.substr( strlen( PROTO_ANCHORE) );
        args.arg2 = "false"; // 板名、スレ名非表示
        args.arg3 = "false"; // あぼーんしたレスの内容は非表示(あぼーんと表示)

#ifdef _DEBUG
        std::cout << "anchore = " << args.arg1 << std::endl;
#endif

        view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, m_url_article, args );
    }

    // レス番号
    else if( url.find( PROTO_RES ) == 0 ){

        args.arg1 = url.substr( strlen( PROTO_RES ) );
        int tmp_num = atoi( args.arg1.c_str() );

#ifdef _DEBUG
        std::cout << "res = " << args.arg1 << std::endl;
#endif

        // あぼーんされたレスの内容をポップアップ表示
        if( m_article->get_abone( tmp_num ) ){

            args.arg2 = "false"; // 板名、スレ名非表示
            args.arg3 = "true"; // あぼーんレスの内容を表示


            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, m_url_article, args );
        }

        // 参照ポップアップ
        else if( CONFIG::get_refpopup_by_mo() && m_article->get_res_reference( tmp_num ).size() ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPREFER, m_url_article, args );
        }
    }

    // 抽出(or)
    else if( url.find( PROTO_OR ) == 0 ){

        std::string url_tmp = url.substr( strlen( PROTO_OR ) );

        int i = url_tmp.find( KEYWORD_SIGN );

        std::string url_dat = DBTREE::url_dat( url_tmp.substr( 0, i ) );
        args.arg1 = url_tmp.substr( i + strlen( KEYWORD_SIGN ) );
        args.arg2 = "OR";

        if( ! url_dat.empty() ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPDRAWOUT, url_dat, args );
        }
    }

    // 名前ポップアップ
    else if( CONFIG::get_namepopup_by_mo() && url.find( PROTO_NAME ) == 0 ){

        int num_name = m_article->get_num_name( res_number );
        args.arg1 = m_article->get_name( res_number );

        if( num_name >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPNAME, m_url_article, args );
        }
    }

    // IDポップアップ
    else if( CONFIG::get_idpopup_by_mo() && url.find( PROTO_ID ) == 0 ){

        args.arg1 = m_article->get_id_name( res_number );
        int num_id = m_article->get_num_id_name( res_number );

        if( num_id >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
        }
    }

    // ID:〜の範囲選択の上にポインタがあるときIDポップアップ
    else if( url.find( "ID:" ) == 0 ){

        args.arg1 = PROTO_ID + url.substr( 3 );
        int num_id = m_article->get_num_id_name( args.arg1 );

#ifdef _DEBUG
        std::cout << "num_id = " << num_id << std::endl;
#endif

        if( num_id >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
        }
    }

    // その他のリンク
    if( !view_popup ){

        // dat 又は板の場合
        int num_from, num_to;
        std::string url_dat = DBTREE::url_dat( url, num_from, num_to );
        std::string url_subject = DBTREE::url_subject( url );

#ifdef _DEBUG
        std::cout << "url_dat = " << url_dat << std::endl;
        std::cout << "url_subject = " << url_subject << std::endl;
#endif

        // 他スレ
        if( ! url_dat.empty() ){
            if( num_from == 0 ) args.arg1 = "1"; // 最低でも1レス目は表示
            else args.arg1 = MISC::get_filename( url );

            args.arg2 = "true"; // 板名、スレ名表示
            args.arg3 = "false"; // あぼーんしたレスの内容は非表示(あぼーんと表示)

            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, url_dat, args );
        }

        // 板
        else if( ! url_subject.empty() ){

            std::string tmpstr = DBTREE::board_name( url );
            args.arg1 = "[ " + tmpstr + " ] ";

            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPHTML, m_url_article, args );
        }
    }

    if( view_popup ) show_popup( view_popup, margin_popup );
}



//
// リンクからマウスが出た
//
void ArticleViewBase::slot_leave_url()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_leave_url\n";
#endif

    hide_popup();
}



//
// リンクをクリック
//
bool ArticleViewBase::click_url( std::string url, int res_number, GdkEventButton* event )
{
    assert( m_article );
    if( url.empty() ) return false;

    CONTROL::Control& control = get_control();
  
#ifdef _DEBUG    
    std::cout << "ArticleViewBase::click_url " << url << std::endl;
#endif

    // プレビュー画面など、レスが存在しないときはいくつかの機能を無効にする
    bool res_exist = ( ! m_article->empty() && m_article->res_header( res_number ) );

    /////////////////////////////////////////////////////////////////
    // ID クリック
    if( url.find( PROTO_ID ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            int num_id = m_article->get_num_id_name( res_number );
            m_id_name = m_article->get_id_name( res_number );

            // ID ポップアップ
            if( num_id >= 1 && control.button_alloted( event, CONTROL::PopupIDButton ) ){
                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_id_name;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
                const int margin_popup = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup );
            }
            else if( control.button_alloted( event, CONTROL::DrawoutIDButton ) ) slot_drawout_id();
            else if( control.button_alloted( event, CONTROL::PopupmenuIDButton ) ){
                show_popupmenu( url, false );
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // 名前クリック
    else if( url.find( PROTO_NAME ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            int num_name = m_article->get_num_name( res_number );
            m_name = m_article->get_name( res_number );

            // 名前ポップアップ
            if( num_name >= 1 && control.button_alloted( event, CONTROL::PopupIDButton ) ){
                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_name;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPNAME, m_url_article, args );
                const int margin_popup = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup );
            }
            else if( control.button_alloted( event, CONTROL::DrawoutIDButton ) ) slot_drawout_name();
            else if( control.button_alloted( event, CONTROL::PopupmenuIDButton ) ){
                show_popupmenu( url, false );
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // あぼーんクリック
    else if( url.find( PROTO_ABONE ) == 0 ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ){
            show_popupmenu( url, false );
        }
    }

    /////////////////////////////////////////////////////////////////
    // 壊れていますクリック
    else if( url.find( PROTO_BROKEN ) == 0 ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ){
            show_popupmenu( url, false );
        }
    }

    /////////////////////////////////////////////////////////////////
    // 荒らし報告用URL表示クリック
    else if( url.find( PROTO_URL4REPORT ) == 0 ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ){

            m_show_url4report = true;
            relayout();
        }
    }

    /////////////////////////////////////////////////////////////////
    // BE クリック
    else if( url.find( PROTO_BE ) == 0 ){

        if( ! res_exist ) return true;

        hide_popup();

        std::stringstream ssurl;
        ssurl << "http://be.2ch.net/test/p.php?i="
              << url.substr( strlen( PROTO_BE ) )
              << "&u=d:"
              << DBTREE::url_readcgi( m_url_article, res_number, 0 );
#ifdef _DEBUG    
        std::cout << "open  " << ssurl.str() << std::endl;
#endif
        if( control.button_alloted( event, CONTROL::OpenBeButton ) ) CORE::core_set_command( "open_url_browser", ssurl.str() );
        else if( control.button_alloted( event, CONTROL::PopupmenuBeButton ) ){
            show_popupmenu( ssurl.str(), false );
        }
    }

    /////////////////////////////////////////////////////////////////
    // アンカーをクリック
    else if( url.find( PROTO_ANCHORE ) == 0 ){

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

        // ジャンプ先セット
        m_str_num = url.substr( strlen( PROTO_ANCHORE ) );

#ifdef _DEBUG
        std::cout << "anchor num = " << m_str_num << std::endl;
#endif

            hide_popup();

            if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ) show_popupmenu( url, false );
            else if( control.button_alloted( event, CONTROL::DrawoutAncButton ) ) slot_drawout_around();
        }
    }

    /////////////////////////////////////////////////////////////////
    // レス番号クリック
    else if( url.find( PROTO_RES ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            m_str_num = MISC::itostr( res_number );
            m_url_tmp = DBTREE::url_readcgi( m_url_article, res_number, 0 );

            if( control.button_alloted( event, CONTROL::PopupmenuResButton ) ) show_popupmenu( url, false );

            // ブックマークセット
            else if( control.button_alloted( event, CONTROL::BmResButton ) ) slot_bookmark();

            // 参照ポップアップ表示
            else if( control.button_alloted( event, CONTROL::ReferResButton )
                     && m_article->get_res_reference( res_number ).size() ){

                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_str_num;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPREFER, m_url_article, args );
                const int margin_popup = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup );
            }
        }
    }


    /////////////////////////////////////////////////////////////////
    // OR抽出
    else if( url.find( PROTO_OR ) == 0 ){

        std::string url_tmp = url.substr( strlen( PROTO_OR ) );
        int i = url_tmp.find( KEYWORD_SIGN );
        std::string url_dat = DBTREE::url_dat( url_tmp.substr( 0, i ) );

        if( ! url_dat.empty() ){
            std::string query = url_tmp.substr( i + strlen( KEYWORD_SIGN ) );
            CORE::core_set_command( "open_article_keyword" ,url_dat, query, "true" );
        }
    }
  
    /////////////////////////////////////////////////////////////////
    // 画像クリック
    else if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN && ( CONFIG::get_use_image_view() || CONFIG::get_use_inline_image() ) ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuImageButton ) ){
            show_popupmenu( url, false );
        }

        else if( ! DBIMG::is_cached( url ) && ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
            mdiag.run();
        }

        else if( DBIMG::get_abone( url )){
            SKELETON::MsgDiag mdiag( NULL, "あぼ〜んされています" );
            mdiag.run();
        }

        else if( DBIMG::get_type_real( url ) == DBIMG::T_LARGE ){
            SKELETON::MsgDiag mdiag( NULL, "画像サイズが大きすぎます。\n\n表示するにはリンクの上でコンテキストメニューを開いて\n「サイズが大きい画像を表示」をクリックしてください。" );
            mdiag.run();
        }

        else{

            bool top = true;
            bool load = false;

            // バックで開く
            if( control.button_alloted( event, CONTROL::OpenBackImageButton ) ) top = false;
        
            // キャッシュに無かったらロード
            if( ! DBIMG::is_cached( url ) ){
                DBIMG::download_img( url, DBTREE::url_readcgi( m_url_article, res_number, 0 ) );
                hide_popup();
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP,  url );
                const int margin_popup = CONFIG::get_margin_imgpopup();
                show_popup( view_popup, margin_popup );
                top = false;
                load = true;
            }

            if( CONFIG::get_use_image_view() ){
                CORE::core_set_command( "open_image", url );
                if( top ) CORE::core_set_command( "switch_image" );
            }
            else if( ! load && control.button_alloted( event, CONTROL::ClickButton ) ) CORE::core_set_command( "open_url", url );

            redraw_view();
        }
    }

    /////////////////////////////////////////////////////////////////
    // ブラウザで開く
    else if( control.button_alloted( event, CONTROL::ClickButton ) ){

        std::string tmp_url = url;

        // 相対パス
        if( url.find( "./" ) == 0 ) tmp_url = DBTREE::url_boardbase( m_url_article ) + url.substr( 1 );

        // 絶対パス
        else if( url.find( "/" ) == 0 ) tmp_url = DBTREE::url_root( m_url_article ) + url.substr( 1 );

        hide_popup();

        CORE::core_set_command( "open_url", tmp_url );
    }

    /////////////////////////////////////////////////////////////////
    // 失敗
    else return false;

    return true;
}



//
// ポップアップが表示されているか
//
const bool ArticleViewBase::is_popup_shown() const
{
    return ( m_popup_win && m_popup_shown );
}


//
// ポップアップが表示されていてかつマウスがその上にあるか
//
const bool ArticleViewBase::is_mouse_on_popup()
{
    if( ! is_popup_shown() ) return false;
    return m_popup_win->view()->is_mouse_on_view();
}



//
// ポップアップ表示
//
// view にあらかじめ内容をセットしてから呼ぶこと
// viewは SKELETON::PopupWin のデストラクタで削除される
//
void ArticleViewBase::show_popup( SKELETON::View* view, int margin )
{
    hide_popup();
    if( !view ) return;

    delete_popup();

    m_popup_win = new SKELETON::PopupWin( this, view, margin );
    m_popup_win->signal_leave_notify_event().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_popup_leave_notify_event ) );
    m_popup_win->sig_hide_popup().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_hide_popup ) );
    m_popup_shown = true;
}



//
// 子 popup windowの外にポインタが出た
//
bool ArticleViewBase::slot_popup_leave_notify_event( GdkEventCrossing* event )
{
    // クリックしたときやホイールを回すと event->mode に　GDK_CROSSING_GRAB
    // か GDK_CROSSING_UNGRAB がセットされてイベントが発生する場合がある
    if( event->mode == GDK_CROSSING_GRAB ) return false;
    if( event->mode == GDK_CROSSING_UNGRAB ) return false;

    slot_hide_popup();
    return true;
}


//
// 子 popup windowからhide依頼が来た
//
void ArticleViewBase::slot_hide_popup()
{
    hide_popup();

    // ポインタがwidgetの外にあったら親に知らせて自分も閉じてもらう
    if( ! is_mouse_on_view() ) sig_hide_popup().emit();
}



//
// popup のhide
//
// force = true ならチェック無しで強制 hide
//
void ArticleViewBase::hide_popup( bool force )
{
    if( ! is_popup_shown() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::hide_popup force = " << force << " " << get_url() << std::endl;
#endif

    if( ! force ){
        
        // ArticleView をポップアップ表示している場合
        ArticleViewBase* popup_article = NULL;
        popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );

        if( popup_article ){

            // 孫のpopupが表示されてたらhideしない
            if( popup_article->is_popup_shown() ) return;

            // ポップアップメニューが表示されてたらhideしない
            // ( ポップアップメニューがhideしたときにhideする )
            if( SESSION::is_popupmenu_shown() ) return;

#ifdef _DEBUG
        std::cout << "target = " << popup_article->get_url() << std::endl;
#endif
        }
    }

    m_popup_win->hide();
    m_popup_shown = false;
}



//
// ポップアップの削除
//
void ArticleViewBase::delete_popup()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::delete_popup " << get_url() << std::endl;
#endif

    if( m_popup_win ) delete m_popup_win;
    m_popup_win = NULL;
    m_popup_shown = false;
}


//
// ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
//
// SKELETON::View::show_popupmenu() を参照すること
//
void ArticleViewBase::activate_act_before_popupmenu( const std::string& url )
{
    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // 子ポップアップが表示されていて、かつポインタがその上だったら表示しない
    ArticleViewBase* popup_article = NULL;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article && popup_article->is_mouse_on_view() ) return;
    hide_popup();
    Glib::RefPtr< Gtk::Action > act, act2;
    act = action_group()->get_action( "CopyURL" );
    act2 = action_group()->get_action( "OpenBrowser" );

    // url がセットされてない
    if( url.empty() ) {
        if( act ) act->set_sensitive( false );
        if( act2 ) act2->set_sensitive( false );
        m_url_tmp = std::string();
    }

    // url がセットされている
    else {

        if( act ) act->set_sensitive( true );
        if( act2 ) act2->set_sensitive( true );

        // レス番号クリックの場合
        if( url.find( PROTO_RES ) == 0 ){
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_RES ) ).c_str() ), 0 );
        }

        // アンカークリックの場合
        else if( url.find( PROTO_ANCHORE ) == 0 ){
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_ANCHORE ) ).c_str() ), 0 );
        }
        
        else m_url_tmp = url;
    }


    // 範囲選択されてない
    const unsigned int max_selection_str = 1024;
    const unsigned int max_selection_str_quote = 8192;

    std::string str_select = m_drawarea->str_selection();
    act = action_group()->get_action( "QuoteSelectionRes" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str_quote ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "Copy" );
    if( act ){
        if( str_select.empty() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "DrawoutWord" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "AboneWord_Menu" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "Search_Menu" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // ユーザコマンド
    // 選択不可かどうか判断して visible か sensitive にする
    const int usrcmd_size = CORE::get_usrcmd_manager()->get_size();
    for( int i = 0; i < usrcmd_size; ++i ){
        std::string str_cmd = "usrcmd" + MISC::itostr( i );
        act = action_group()->get_action( str_cmd );
        if( act ){

#if GTKMMVER >= 260
            if( CONFIG::get_hide_usrcmd() ){
                if( CORE::get_usrcmd_manager()->is_sensitive( i, url, str_select ) ) act->set_visible( true );
                else act->set_visible( false );
            }
            else{
                if( CORE::get_usrcmd_manager()->is_sensitive( i, url, str_select ) ) act->set_sensitive( true );
                else act->set_sensitive( false );
            }
#else
            if( CORE::get_usrcmd_manager()->is_sensitive( i, url, str_select ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
#endif            
        }
    }


    // ブックマークがセットされていない
    act = action_group()->get_action( "DrawoutBM" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }
    act = action_group()->get_action( "PreBookMark" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }
    act = action_group()->get_action( "NextBookMark" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 新着移動
    act = action_group()->get_action( "GotoNew" );
    if( act ){
        if( m_article->get_number_new() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 透明あぼーん
    act = action_group()->get_action( "TranspAbone" );
    if( act ){

        Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
        if( m_article->get_abone_transparent() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 透明/連鎖あぼーん
    act = action_group()->get_action( "TranspChainAbone" );
    if( act ){

        Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
        if( m_article->get_abone_transparent() && m_article->get_abone_chain() ) tact->set_active( true );
        else tact->set_active( false );
    }


    // 画像
    if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ){ 

        // モザイク
        act = action_group()->get_action( "Cancel_Mosaic" );
        if( act ){
            if( DBIMG::is_cached( url ) && DBIMG::get_mosaic( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // サイズの大きい画像を表示
        act = action_group()->get_action( "ShowLargeImg" );
        if( act ){
            if( DBIMG::get_type_real( url ) == DBIMG::T_LARGE ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // 保護のトグル切替え
        act = action_group()->get_action( "ProtectImage" );
        if( act ){

            if( DBIMG::is_cached( url ) ){

                act->set_sensitive( true );

                Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
                if( DBIMG::is_protected( url ) ) tact->set_active( true );
                else tact->set_active( false );
            }
            else act->set_sensitive( false );
        }

        // 削除
        act = action_group()->get_action( "DeleteImage_Menu" );
        if( act ){

            if( ( DBIMG::is_cached( url ) || DBIMG::get_type_real( url ) == DBIMG::T_LARGE )
                && ! DBIMG::is_protected( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // 保存
        act = action_group()->get_action( "SaveImage" );
        if( act ){

            if(  DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // プロパティ
        act = action_group()->get_action( "PreferenceImage" );
        if( act ){

            if(  DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // あぼーん
        act = action_group()->get_action( "AboneImage" );
        if( act ){

            if( DBIMG::is_protected( url ) ) act->set_sensitive( false );
            else{

                act->set_sensitive( true );

                Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
                if( DBIMG::get_abone( url ) ) tact->set_active( true );
                else tact->set_active( false );
            }
        }
    }

    m_enable_menuslot = true;
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* ArticleViewBase::get_popupmenu( const std::string& url )    
{
    // 表示
    Gtk::Menu* popupmenu = NULL;

    // 削除サブメニュー
    if( url == "popup_menu_delete" ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_delete" ) );
    }

    // レス番号ポップアップメニュー
    else if( url.find( PROTO_RES ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_res" ) );
    }

    //　アンカーポップアップメニュー
    else if( url.find( PROTO_ANCHORE ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_anc" ) );
    }

    // IDポップアップメニュー
    else if( url.find( PROTO_ID ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_id" ) );
    }

    // 名前ポップアップメニュー
    else if( url.find( PROTO_NAME ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_name" ) );
    }

    // あぼーんポップアップメニュー
    else if( url.find( PROTO_ABONE ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_abone" ) );
    }

    // 壊れていますポップアップメニュー
    else if( url.find( PROTO_BROKEN ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_broken" ) );
    }

    // 画像ポップアップメニュー
    else if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ){ 
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_img" ) );
    }

    // 通常メニュー
    else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );

    return popupmenu;
}


//
// クリップボードに選択文字コピーのメニュー
//
void ArticleViewBase::slot_copy_selection_str()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_copy_selection_str " << get_url() << std::endl;
#endif    

    if( m_drawarea->str_selection().empty() ) return;
    COPYCLIP( m_drawarea->str_selection() );
}


//
// 全選択
//
void ArticleViewBase::slot_select_all()
{
    if( m_drawarea ) m_drawarea->select_all();
}


//
// 選択して抽出
//
void ArticleViewBase::slot_drawout_selection_str()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

    CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH, query );
    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
}


//
// 全ログ検索実行
//
void ArticleViewBase::slot_search_cacheall()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_cacheall "<< query << std::endl;
#endif
    
    CORE::core_set_command( "open_article_searchalllog", "" , query );
}


//
// ログ検索実行
//
void ArticleViewBase::slot_search_cachelocal()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    std::string url = DBTREE::url_subject( m_url_article );

    if( query.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_cachelocal " << url << std::endl
              << query << std::endl;
#endif
    
    CORE::core_set_command( "open_article_searchlog", url , query );
}


//
// スレタイ検索実行
//
void ArticleViewBase::slot_search_title()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_title query = " << query << std::endl;
#endif
    
    CORE::core_set_command( "open_article_searchtitle", "" , query );
}



//
// ユーザコマンド実行
//
void ArticleViewBase::slot_usrcmd( int num )
{
    CORE::core_set_command( "exec_usr_cmd" ,m_url_article, MISC::itostr( num ), m_url_tmp, m_drawarea->str_selection() );
}



//
// ブックマーク設定、解除
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_bookmark()
{
    if( m_str_num.empty() ) return;

    int number = atoi( m_str_num.c_str() );
    bool bookmark = ! DBTREE::is_bookmarked( m_url_article, number );
    DBTREE::set_bookmark( m_url_article, number, bookmark );
    if( bookmark ) m_current_bm = number;
    redraw_view();
    ARTICLE::get_admin()->set_command( "redraw_views", m_url_article );
}



//
// ポップアップメニューでブラウザで開くを選択
//
void ArticleViewBase::slot_open_browser()
{
    if( m_url_tmp.empty() ) return;
    std::string url = m_url_tmp;

    // 画像、かつキャッシュにある場合
    if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN && DBIMG::is_cached( url ) ) url = "file://" + DBIMG::get_cache_path( url );

    CORE::core_set_command( "open_url_browser", url );
}


//
// レスをする
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_write_res()
{
    if( m_str_num.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_write_res number = " << m_str_num << std::endl;
#endif    

    CORE::core_set_command( "open_message" ,m_url_article, ">>" + m_str_num + "\n" );
}


//
// 参照レスをする
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_quote_res()
{
    assert( m_article );
    if( m_str_num.empty() ) return;
    
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_quote_res number = " << m_str_num << std::endl;
#endif    

    CORE::core_set_command( "open_message" ,m_url_article,
                            ">>" + m_str_num + "\n" + m_article->get_res_str( atoi( m_str_num.c_str() ), true ) );
}


//
// 選択部分を引用してレスする
//
void ArticleViewBase::slot_quote_selection_res()
{
    assert( m_article );

    int num_from = m_drawarea->get_selection_resnum_from();
    if( ! num_from ) return;

    int num_to = m_drawarea->get_selection_resnum_to();

    std::string str_num = MISC::itostr( num_from );
    if( num_from < num_to ) str_num += "-" + MISC::itostr( num_to );

    std::string str_res;
    str_res = CONFIG::get_ref_prefix();

    std::string query = m_drawarea->str_selection();
    if( query.empty() ) return;

    query = MISC::replace_str( query, "\n", "\n" + str_res );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_quote_selection_res number = " << str_num << std::endl;
#endif    

    CORE::core_set_command( "open_message", m_url_article, ">>" + str_num + "\n" + str_res + query + "\n" );
}



//
// リンクのURLをコピーのメニュー
//
void ArticleViewBase::slot_copy_current_url()
{
    if( m_url_tmp.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_copy_current_url url = " << m_url_tmp << std::endl;
#endif    

    COPYCLIP( m_url_tmp );
}


//
// 名前をコピー
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_copy_name()
{
    std::string name = m_name;
    COPYCLIP( name );
}


//
// IDをコピー
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_copy_id()
{
    std::string id = "ID:" + m_id_name.substr( strlen( PROTO_ID ) );
    COPYCLIP( id );
}



//
// レス番号クリック時のレスのコピーのメニュー
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_copy_res( bool ref )
{
    assert( m_article );
    if( m_str_num.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::copy_res number = " << m_str_num << std::endl;
#endif    

    std::string tmpstr;
    if( ref ) tmpstr = CONFIG::get_ref_prefix();
    std::string board_name = DBTREE::board_name( m_url_article );
    if( ! board_name.empty() ) tmpstr += "[ " + board_name + " ] ";
    tmpstr += DBTREE::article_subject( m_url_article ) + "\n\n";
    tmpstr += m_article->get_res_str( atoi( m_str_num.c_str() ), ref );

    COPYCLIP( tmpstr );
}




//
// お気に入り登録
//
void ArticleViewBase::slot_favorite()
{
    CORE::DATA_INFO info;
    info.type = TYPE_THREAD;
    info.url = m_url_article;;
    info.name = DBTREE::article_subject( m_url_article );

    CORE::SBUF_clear_info();
    CORE::SBUF_append( info );

    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}



//
// 別のタブを開いてレス抽出
//
// 呼び出す前に m_str_num に抽出するレスをセットしておくこと
//
void ArticleViewBase::slot_drawout_res()
{
    CORE::core_set_command( "open_article_res" ,m_url_article, m_str_num );
}



//
// 別のタブを開いて周辺のレスを抽出
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_drawout_around()
{
    const int range = 10;
    int center = atoi( m_str_num.c_str() );
    int from = MAX( 0, center - range );
    int to = center + range;
    std::stringstream ss;
    ss << from << "-" << to;
    CORE::core_set_command( "open_article_res" ,m_url_article, ss.str(), m_str_num );
}


//
// 別のタブを開いてテンプレート表示
//
void ArticleViewBase::slot_drawout_tmp()
{
    const int to = 20;
    std::stringstream ss;
    ss << "1-" << to;
    CORE::core_set_command( "open_article_res" ,m_url_article, ss.str() );
}


//
// 別のタブを開いて名前抽出
//
// 呼び出す前に m_name にIDをセットしておくこと
//
void ArticleViewBase::slot_drawout_name()
{
    CORE::core_set_command( "open_article_name" ,m_url_article, m_name );
}



//
// 別のタブを開いてID抽出
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_drawout_id()
{
    CORE::core_set_command( "open_article_id" ,m_url_article, m_id_name );
}



//
// 別のタブを開いてブックマーク抽出
//
void ArticleViewBase::slot_drawout_bm()
{
    CORE::core_set_command( "open_article_bm" ,m_url_article );
}



//
// 別のタブを開いて参照抽出
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_drawout_refer()
{
    CORE::core_set_command( "open_article_refer" ,m_url_article, m_str_num );
}



//
// 別のタブを開いてURL抽出
//
void ArticleViewBase::slot_drawout_url()
{
    CORE::core_set_command( "open_article_url" ,m_url_article );
}



//
// レス番号であぼ〜ん
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_abone_res()
{
    int number = atoi( m_str_num.c_str() );

    DBTREE::set_abone_res( m_url_article, number, true );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// IDであぼ〜ん(ローカル、板(一時的) )
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_abone_id()
{
    DBTREE::add_abone_id( m_url_article, m_id_name );
    DBTREE::add_abone_id_board( m_url_article, m_id_name );
}



//
// 名前であぼ〜ん
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_abone_name()
{
    DBTREE::add_abone_name( m_url_article, m_name );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 範囲選択した文字列であぼ〜ん
//
void ArticleViewBase::slot_abone_word()
{
    DBTREE::add_abone_word( m_url_article, m_drawarea->str_selection() );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 名前であぼ〜ん(板レベル)
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_abone_name_board()
{
    DBTREE::add_abone_name_board( m_url_article, m_name );
}


//
// 範囲選択した文字列であぼ〜ん(板レベル)
//
void ArticleViewBase::slot_abone_word_board()
{
    DBTREE::add_abone_word_board( m_url_article, m_drawarea->str_selection() );
}


//
// 名前であぼ〜ん(全体)
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_global_abone_name()
{
    CORE::core_set_command( "set_global_abone_name", "", m_name );
}


//
// 範囲選択した文字列であぼ〜ん(全体)
//
void ArticleViewBase::slot_global_abone_word()
{
    CORE::core_set_command( "set_global_abone_word", "", m_drawarea->str_selection() );
}



//
// 透明あぼーん
//
void ArticleViewBase::slot_toggle_abone_transp()
{
    if( ! m_enable_menuslot ) return;

    assert( m_article );

    bool setval = true;

    if( m_article->get_abone_transparent() ) setval = false;
    m_article->set_abone_transparent( setval );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 透明/連鎖あぼーん
//
void ArticleViewBase::slot_toggle_abone_transp_chain()
{
    if( ! m_enable_menuslot ) return;

    assert( m_article );

    bool setval = true;

    if( m_article->get_abone_transparent() && m_article->get_abone_chain() ) setval = false;
    m_article->set_abone_transparent( setval );
    m_article->set_abone_chain( setval );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}



//
// 画像のモザイク解除
//
void ArticleViewBase::slot_cancel_mosaic()
{
    if( ! DBIMG::is_cached( m_url_tmp ) ) return;

    if( DBIMG::is_fake( m_url_tmp ) ){

        SKELETON::MsgDiag mdiag( NULL,
                                 "拡張子が偽装されています。モザイクを解除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

        mdiag.set_default_response( Gtk::RESPONSE_NO );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;
    }

    DBIMG::set_mosaic( m_url_tmp, false );
    CORE::core_set_command( "redraw", m_url_tmp );
}


//
// 大きい画像を表示
//
void ArticleViewBase::slot_show_large_img()
{
    DBIMG::show_large_img( m_url_tmp );
}



//
// 画像削除
//
void ArticleViewBase::slot_deleteimage()
{
    if( ! m_url_tmp.empty() ) CORE::core_set_command( "delete_image", m_url_tmp );
}


//
// 画像保存
//
void ArticleViewBase::slot_saveimage()
{
    DBIMG::save( m_url_tmp, NULL, std::string() );
}


//
// 画像保護
//
void ArticleViewBase::slot_toggle_protectimage()
{
    if( ! m_enable_menuslot ) return;

    DBIMG::set_protect( m_url_tmp , ! DBIMG::is_protected( m_url_tmp ) );
}


//
// 画像あぼ〜ん
//
void ArticleViewBase::slot_abone_img()
{
    if( ! m_enable_menuslot ) return;
    if( m_url_tmp.empty() ) return;

    DBIMG::set_abone( m_url_tmp , ! DBIMG::get_abone( m_url_tmp ) );
    if( DBIMG::get_abone( m_url_tmp ) ) CORE::core_set_command( "delete_image", m_url_tmp );
}


//
// 検索entryでenterを押した
//
void ArticleViewBase::slot_active_search()
{
    if( ! m_toolbar ) return;
    if( m_toolbar->m_entry_search.completion() ) return;

    std::string query = m_toolbar->m_entry_search.get_text();
    if( query.empty() ){
        slot_push_claar_hl();
        focus_view();
        CORE::core_set_command( "set_info", "", "" );
        return;
    }

    std::list< std::string > list_query;
    list_query = MISC::split_line( query );

    if( m_query != query ){
        m_query = query;
        m_drawarea->set_jump_history();
        m_drawarea->search( list_query, m_search_invert );
    }

    int hit = m_drawarea->search_move( m_search_invert );

    if( ! hit ){
        slot_push_claar_hl();
        CORE::core_set_command( "set_info", "", "検索結果： ヒット無し" );
    }
    else{
        focus_view();
        CORE::core_set_command( "set_info", "", "検索結果： " + MISC::itostr( hit ) + "件" );
    }
}



//
// 検索entryの操作
//
void ArticleViewBase::slot_entry_operate( int controlid )
{
    if( controlid == CONTROL::Cancel ) focus_view();
    else if( controlid == CONTROL::DrawOutAnd ) slot_push_drawout_and();
}
