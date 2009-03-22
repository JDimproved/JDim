// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "admin.h"
#include "window.h"
#include "view.h"
#include "dragnote.h"
#include "msgdiag.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "history/historymanager.h"
#include "history/viewhistoryitem.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "command.h"
#include "session.h"
#include "global.h"
#include "updatemanager.h"


enum
{
    MAX_TABS = 50
};


// open_view で使うモード
enum
{
    OPEN_MODE_AUTO = 1,
    OPEN_MODE_NOSWITCH = 2,
    OPEN_MODE_LOCK = 4,
    OPEN_MODE_OFFLINE = 8
};


using namespace SKELETON;


Admin::Admin( const std::string& url )
    : m_url( url ),
      m_win( NULL ),
      m_notebook( NULL ),
      m_focus( false ),
      m_use_viewhistory( false )
{
    m_notebook = new DragableNoteBook();

    m_notebook->signal_switch_page().connect( sigc::mem_fun( *this, &Admin::slot_switch_page ) );
    m_notebook->set_scrollable( true );

    m_notebook->sig_tab_click().connect( sigc::mem_fun( *this, &Admin::slot_tab_click ) );
    m_notebook->sig_tab_close().connect( sigc::mem_fun( *this, &Admin::slot_tab_close ) );
    m_notebook->sig_tab_reload().connect( sigc::mem_fun( *this, &Admin::slot_tab_reload ) );
    m_notebook->sig_tab_menu().connect( sigc::mem_fun( *this, &Admin::slot_tab_menu ) );

    // D&D
    m_notebook->sig_drag_data_get().connect( sigc::mem_fun(*this, &Admin::slot_drag_data_get ) );

    m_list_command.clear();
}


Admin::~Admin()
{
#ifdef _DEBUG
    std::cout << "Admin::~Admin " << m_url << std::endl;
#endif

    // デストラクタの中からdispatchを呼ぶと落ちるので dispatch不可にする
    set_dispatchable( false );

    int pages = m_notebook->get_n_pages();

#ifdef _DEBUG
    std::cout << "pages = " << pages << std::endl;
#endif
    if( pages ){

        for( int i = 0; i < pages; ++i ){

            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( 0 ) );

            m_notebook->remove_page( 0 );

            if( view ) delete view;
        }
    }
    m_list_command.clear();

    close_window();
    delete_jdwin();

    if( m_notebook ) delete m_notebook;
}


//
// メニューのセットアップ
//
void Admin::setup_menu( const bool enable_checkupdate )
{
    // 右クリックメニュー
    m_action_group = Gtk::ActionGroup::create();
    m_action_group->add( Gtk::Action::create( "Quit", "Quit" ), sigc::mem_fun( *this, &Admin::slot_close_tab ) );

    m_action_group->add( Gtk::ToggleAction::create( "LockTab", "タブをロックする(_K)", std::string(), false ),
                         sigc::mem_fun( *this, &Admin::slot_lock ) );

    m_action_group->add( Gtk::Action::create( "Close_Tab_Menu", "複数のタブを閉じる(_L)" ) );
    m_action_group->add( Gtk::Action::create( "CloseOther", "他のタブ(_O)" ), sigc::mem_fun( *this, &Admin::slot_close_other_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseLeft", "左←のタブ(_L)" ), sigc::mem_fun( *this, &Admin::slot_close_left_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseRight", "右→のタブ(_R)" ), sigc::mem_fun( *this, &Admin::slot_close_right_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseAll", "全てのタブ(_A)" ), sigc::mem_fun( *this, &Admin::slot_close_all_tabs ) );

    m_action_group->add( Gtk::Action::create( "Reload_Tab_Menu", "全てのタブの再読み込み(_A)" ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateAll", "更新チェックのみ(_U)" ), sigc::mem_fun( *this, &Admin::slot_check_update_all_tabs ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateReloadAll", "更新されたタブを再読み込み(_A)" ),
                         sigc::mem_fun( *this, &Admin::slot_check_update_reload_all_tabs ) );
    m_action_group->add( Gtk::Action::create( "ReloadAll", "再読み込み(_R)" ), sigc::mem_fun( *this, &Admin::slot_reload_all_tabs ) );
    m_action_group->add( Gtk::Action::create( "CancelReloadAll", "キャンセル(_C)" ), sigc::mem_fun( *this, &Admin::slot_cancel_reload_all_tabs ) );

    m_action_group->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く(_W)" ), sigc::mem_fun( *this, &Admin::slot_open_by_browser ) );
    m_action_group->add( Gtk::Action::create( "CopyURL", "URLをコピー(_U)" ), sigc::mem_fun( *this, &Admin::slot_copy_url ) );
    m_action_group->add( Gtk::Action::create( "CopyTitle", "タイトルとURLをコピー(_T)" ), sigc::mem_fun( *this, &Admin::slot_copy_title_url ) );

    m_action_group->add( Gtk::Action::create( "Preference", "プロパティ(_P)..."), sigc::mem_fun( *this, &Admin::show_preference ) );

    // 戻る、進む
    m_action_group->add( Gtk::Action::create( "PrevView", "PrevView"),
                         sigc::bind< int >( sigc::mem_fun( *this, &Admin::back_clicked_viewhistory ), 1 ) );
    m_action_group->add( Gtk::Action::create( "NextView", "NextView"),
                         sigc::bind< int >( sigc::mem_fun( *this, &Admin::forward_clicked_viewhistory ), 1 ) );

    m_ui_manager = Gtk::UIManager::create();    
    m_ui_manager->insert_action_group( m_action_group );

    // ポップアップメニューのレイアウト
    Glib::ustring str_ui = 

    "<ui>"

    // 通常
    "<popup name='popup_menu'>"

    "<menuitem action='LockTab'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='Close_Tab_Menu'>"
    "<menuitem action='CloseAll'/>"
    "<menuitem action='CloseOther'/>"
    "<menuitem action='CloseLeft'/>"
    "<menuitem action='CloseRight'/>"
    "</menu>"
    "<separator/>"

    "<menu action='Reload_Tab_Menu'>"
    "<menuitem action='ReloadAll'/>";

    if( enable_checkupdate ){

        str_ui += 
        "<menuitem action='CheckUpdateAll'/>"
        "<menuitem action='CheckUpdateReloadAll'/>";
    }

    str_ui +=

    "<separator/>"
    "<menuitem action='CancelReloadAll'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitle'/>"

    "<separator/>"
    "<menuitem action='Preference'/>"

    "</popup>"


    "</ui>";

    m_ui_manager->add_ui_from_string( str_ui );    

    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu" ) );
    Gtk::MenuItem* item;

    // 移動サブメニュー作成と登録
    for( int i = 0; i < MAX_TABS; ++i ){
        item = Gtk::manage( new Gtk::MenuItem( "dummy" ) );
        item->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *this, &Admin::set_current_page ), i ) );

        m_vec_movemenu_items.push_back( item );
        m_vec_movemenu_append.push_back( false );
    }

    m_move_menu = Gtk::manage( new Gtk::Menu() );

    // 進む、戻る
    Glib::RefPtr< Gtk::Action > act;
    act = m_action_group->get_action( "PrevView" );
    act->set_accel_group( m_ui_manager->get_accel_group() );
    item = Gtk::manage( act->create_menu_item() );
    m_move_menu->append( *item );

    act = m_action_group->get_action( "NextView" );
    act->set_accel_group( m_ui_manager->get_accel_group() );
    item = Gtk::manage( act->create_menu_item() );
    m_move_menu->append( *item );

    m_move_menu->append( *Gtk::manage( new Gtk::SeparatorMenuItem() ) );

    // 先頭、最後に移動
    item = Gtk::manage( new Gtk::MenuItem( "先頭のタブに移動(_H)", true ) );
    m_move_menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &Admin::tab_head ) );

    item = Gtk::manage( new Gtk::MenuItem( "最後のタブに移動(_T)", true ) );
    m_move_menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &Admin::tab_tail ) );

    m_move_menu->append( *Gtk::manage( new Gtk::SeparatorMenuItem() ) );

    item  = Gtk::manage( new Gtk::MenuItem( "移動" ) );
    item->set_submenu( *m_move_menu );
    m_vec_movemenu_items.push_back( item );

    popupmenu->insert( *item, 0 );
    item->show_all();

    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    popupmenu->insert( *item, 1 );
    item->show_all();

    // ポップアップメニューにアクセレータを表示
    CONTROL::set_menu_motion( popupmenu );
}


Gtk::Widget* Admin::get_widget()
{
    return dynamic_cast< Gtk::Widget*>( m_notebook );
}


Gtk::Window* Admin::get_win()
{
    return dynamic_cast< Gtk::Window*>( m_win );
}


void Admin::delete_jdwin()
{
    if( m_win ){
        delete m_win;
        m_win = NULL;
    }
}


const bool Admin::is_booting()
{
    if( get_jdwin() && get_jdwin()->is_booting() ) return true;

    return ( has_commands() );
}


// SIGHUPを受け取った
void Admin::shutdown()
{
    int pages = m_notebook->get_n_pages();

#ifdef _DEBUG
    std::cout << "pages = " << pages << std::endl;
#endif
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view ) view->shutdown();
        }
    }
}



//
// ページが含まれていないか
//
bool Admin::empty()
{
    return ( m_notebook->get_n_pages() == 0 );
}


// タブの数
int Admin::get_tab_nums()
{
    if( m_notebook ) return m_notebook->get_n_pages();

    return 0;
}


//
// 含まれているページのURLのリスト取得
//
const std::list<std::string> Admin::get_URLs()
{
    std::list<std::string> urls;
    
    const int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< SKELETON::View* >( m_notebook->get_nth_page( i ) );
            if( view ) urls.push_back( view->get_url() );
        }
    }

    return urls;
}


//
// クロック入力
//
void Admin::clock_in()
{
    // アクティブなビューにクロックを送る
    SKELETON::View* view = get_current_view();
    if( view ) view->clock_in();

    // 全てのビューにクロックを送る
    // clock_in_always()には軽い処理だけを含めること
    int pages = m_notebook->get_n_pages();
    if( pages ){
        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< SKELETON::View* >( m_notebook->get_nth_page( i ) );
            if( view ) view->clock_in_always();
        }
    }

    m_notebook->clock_in();

    if( m_win ) m_win->clock_in();
}



//
// コマンド受付(通常)
//
void Admin::set_command( const std::string& command, const std::string& url,
                         const std::string& arg1, const std::string& arg2,
                         const std::string& arg3, const std::string& arg4,
                         const std::string& arg5, const std::string& arg6 )
{
    set_command_impl( false, command, url, arg1, arg2, arg3, arg4, arg5, arg6 );
}


//
// コマンド受付(即実行)
//
void Admin::set_command_immediately( const std::string& command, const std::string& url,
                         const std::string& arg1, const std::string& arg2,
                         const std::string& arg3, const std::string& arg4,
                         const std::string& arg5, const std::string& arg6 )
{
    set_command_impl( true, command, url, arg1, arg2, arg3, arg4, arg5, arg6 );
}



//
// コマンド受付
//
// immediately = false の場合はすぐにコマンドを実行しないで一旦Dispatcherで
// メインスレッドにコマンドを渡してからメインスレッドで実行する。通常は
// immediately = false で呼び出して、緊急にコマンドを実行させたい場合は
// immediately = true とすること。
//
void Admin::set_command_impl( const bool immediately,
                         const std::string& command, const std::string& url,
                         const std::string& arg1, const std::string& arg2,
                         const std::string& arg3, const std::string& arg4,
                         const std::string& arg5, const std::string& arg6 )
{
#ifdef _DEBUG
    std::cout << "Admin::set_command : immediately = " << immediately <<  " command = " << command << " url = " << url << std::endl
              << arg1 << " " << arg2 << std::endl
              << arg3 << " " << arg4 << std::endl
              << arg5 << " " << arg6 << std::endl;
#endif
    
    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;
    command_arg.arg2 = arg2;
    command_arg.arg3 = arg3;
    command_arg.arg4 = arg4;
    command_arg.arg5 = arg5;
    command_arg.arg6 = arg6;

    if( immediately ){

        m_list_command.push_front( command_arg );
        exec_command();
    }
    else{

        m_list_command.push_back( command_arg );
        dispatch(); // 一度メインループに戻った後にcallback_dispatch() が呼び戻される
    }
}


//
// ディスパッチャのコールバック関数
//
void Admin::callback_dispatch()
{
    while( m_list_command.size() ) exec_command();
}


//
// コマンド実行
//
void Admin::exec_command()
{
    if( m_list_command.size() == 0 ) return;
    
    COMMAND_ARGS command = m_list_command.front();
    m_list_command.pop_front();

    // コマンドリストが空になったことをcoreに知らせる
    if( m_list_command.size() == 0 ) CORE::core_set_command( "empty_command", m_url );

#ifdef _DEBUG
    std::cout << "Admin::exec_command " << m_url << " : " << command.command << " " << command.url << " " << std::endl
              << command.arg1 << " " << command.arg2 << std::endl
              << command.arg3 << " " << command.arg4 << std::endl
              << command.arg5 << " " << command.arg6 << std::endl;
#endif

    // 前回終了時の状態を回復
    if( command.command == "restore" ){
        restore( ( command.arg1 == "only_locked" ) );
    }

    // 移転などでホストの更新
    else if( command.command == "update_url" ){
        update_url( command.url, command.arg1 );
    }

    // 板名更新
    else if( command.command == "update_boardname" ){
        update_boardname( command.url );
    }

    // viewを開く
    else if( command.command == "open_view" ){
        open_view( command );
    }
    // リストで開く
    // arg1 にはdatファイルを空白で区切って指定する
    //
    else if( command.command == "open_list" ){
        open_list( command );
    }
    else if( command.command == "switch_view" ){
        switch_view( command.url );
    }
    else if( command.command == "tab_left" ){
        tab_left();
    }
    else if( command.command == "tab_right" ){
        tab_right();
    }
    else if( command.command == "tab_head" ){
        tab_head();
    }
    else if( command.command == "tab_tail" ){
        tab_tail();
    }
    else if( command.command == "redraw" ){
        redraw_view( command.url );
    }
    else if( command.command == "redraw_current_view" ){
        redraw_current_view();
    }
    else if( command.command == "relayout_current_view" ){
        relayout_current_view();
    }
    // command.url を含むview全てを検索して表示中なら再描画
    else if( command.command == "redraw_views" ){
        redraw_views( command.url );
    }
    else if( command.command == "update_view" ){ // ビュー全体を更新
        update_view( command.url );
    }
    else if( command.command == "update_item" ){ // ビューの一部を更新
        update_item( command.url, command.arg1 );
    }
    else if( command.command == "update_finish" ){
        update_finish( command.url );
    }

    // 全ロック解除
    else if( command.command == "unlock_views" ){
        unlock_all_view( command.url );
    }

    else if( command.command == "close_view" ){
        if( command.arg1 == "closeall" ) close_all_view( command.url );
        else close_view( command.url );
    }
    else if( command.command == "close_currentview" ){
        close_current_view();
    }
    else if( command.command == "set_page" ){
        set_current_page( atoi( command.arg1.c_str() ) );
    }

    // フォーカスイン、アウト
    else if( command.command == "focus_current_view" ){
        m_focus = true;
        focus_current_view();
    }
    else if( command.command == "focus_out" ){
        m_focus = false;
        focus_out();
    }
    else if( command.command == "restore_focus" ){
        m_focus = true;
        restore_focus();
    }

    // adminクラスを前面に出す
    else if( command.command == "switch_admin" ){
        switch_admin();
    }

    // タブに文字をセット、タブ幅調整
    else if( command.command  == "set_tablabel" ){
        set_tablabel( command.url, command.arg1 );
    }
    else if( command.command  == "adjust_tabwidth" ){
        m_notebook->adjust_tabwidth();
    }

    // 全てのビューを再描画
    else if( command.command == "relayout_all" ) relayout_all();

    // タイトル表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_title" ){
        set_title( command.url, command.arg1, false );
    }

    // ステータス表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_status" ){

        bool force = ( command.arg2 == "force" );
        set_status( command.url, command.arg1, force );
    }

    // マウスジェスチャ
    else if( command.command  == "set_mginfo" ){
        if( m_win ) m_win->set_mginfo( command.arg1 );
    }

    // 全タブオートリロード
    else if( command.command == "reload_all_tabs" ){
        reload_all_tabs();
    }

    // オートリロードのキャンセル
    else if( command.command == "cancel_reload" ){
        slot_cancel_reload_all_tabs();
    }

    // タブを隠す
    else if( command.command == "hide_tabs" ) m_notebook->set_show_tabs( false );

    // window 開け閉じ
    else if( command.command == "open_window" ){
        open_window();
        return;
    }
    else if( command.command == "close_window" ){
        close_window();
        return;
    }

    // ツールバーの検索ボックスをフォーカス
    else if( command.command == "focus_toolbar_search" ){
        focus_toolbar_search();
    }

    // ツールバー表示切り替え
    else if( command.command == "toggle_toolbar" ){
        toggle_toolbar();
    }

    // ツールバーのアドレスを更新
    else if( command.command == "update_toolbar_url" ){
        m_notebook->update_toolbar_url( command.url, command.arg1 );
    }

    // ツールバーのラベルを更新
    else if( command.command == "redraw_toolbar" ){
        redraw_toolbar();
    }

    // ツールバーボタン表示更新
    else if( command.command == "update_toolbar_button" ){
        update_toolbar_button();
    }

    // 検索バー表示
    else if( command.command == "open_searchbar" ){
        open_searchbar();
    }

    // 検索バー非表示
    else if( command.command == "close_searchbar" ){
        close_searchbar();
    }

    // タブ表示切り替え
    else if( command.command == "toggle_tab" ){
        toggle_tab();
    }

    // アイコン表示切り替え
    else if( command.command == "toggle_icon" ){
        toggle_icon( command.url );
    }

    // window 開け閉じ可能/不可
    else if( command.command == "enable_fold_win" ){
        if( get_jdwin() ) get_jdwin()->set_enable_fold( true );
    }

    else if( command.command == "disable_fold_win" ){
        if( get_jdwin() ) get_jdwin()->set_enable_fold( false );
    }

    // プロパティ表示
    else if( command.command == "show_current_preferences" ){
        SKELETON::View* view = get_current_view();
        if( view ) view->show_preference();
    }

    // View履歴:戻る
    else if( command.command == "back_viewhistory" ){
        back_viewhistory( command.url, atoi( command.arg1.c_str() ) );
    }

    // View履歴:進む
    else if( command.command == "forward_viewhistory" ){
        forward_viewhistory( command.url, atoi( command.arg1.c_str() ) );
    }

    // View履歴削除
    else if( command.command == "clear_viewhistory" ){
        clear_viewhistory();
    }

    // オートリロード開始
    else if( command.command == "start_autoreload" ){

        int mode = AUTORELOAD_ON;
        if( command.arg1 == "once" ) mode = AUTORELOAD_ONCE;
        int sec = atoi( command.arg2.c_str() );

        set_autoreload_mode( command.url, mode, sec );
    }

    // オートリロード停止
    else if( command.command == "stop_autoreload" ){

        set_autoreload_mode( command.url, AUTORELOAD_NOT, 0 );
    }

    else{

        // ツールバー関係
        SKELETON::View* view = get_view( command.url );

        if( view && command.command == "toolbar_exec_search" ) view->exec_search();
        else if( view && command.command == "toolbar_operate_search" ) view->operate_search( command.arg1 );
        else if( view && command.command == "toolbar_set_search_query" ) view->set_search_query( command.arg1 );
        if( view && command.command == "toolbar_up_search" ) view->up_search();
        if( view && command.command == "toolbar_down_search" ) view->down_search();

        else if( view && command.command == "toolbar_write" ) view->write();
        else if( view && command.command == "toolbar_reload" ) view->reload();
        else if( view && command.command == "toolbar_stop" ) view->stop();
        else if( view && command.command == "toolbar_close_view" ) view->close_view();
        else if( view && command.command == "toolbar_delete_view" ) view->delete_view();
        else if( view && command.command == "toolbar_set_favorite" ) view->set_favorite();

        // ロック/アンロック
        else if( view && command.command == "toolbar_lock_view" ){
            if( view->is_locked() ) unlock( get_current_page() );
            else lock( get_current_page() );
        }

        // 子クラス別のコマンド処理
        else command_local( command );
    }
}


// リストで与えられたページをタブで連続して開く
//
// 連続してリロードかけるとサーバに負担をかけるので、オフラインで開いて
// タイミングをずらしながらリロードする
//
void Admin::open_list( const COMMAND_ARGS& command_list )
{
#ifdef _DEBUG
    std::cout << "Admin::open_list " << m_url << std::endl;
#endif

    const std::string& str_list = command_list.arg1;

    std::list< std::string > list_url = MISC::split_line( str_list );
    if( list_url.empty() ) return;

    int waittime = 0;
    const bool online = SESSION::is_online();

    std::list< std::string >::iterator it = list_url.begin();
    for( ; it != list_url.end(); ++it, waittime += AUTORELOAD_MINSEC ){

        // 各admin別の引数をセット
        COMMAND_ARGS command_arg = get_open_list_args( ( *it ), command_list );

        // 共通の引数をセット
        command_arg.command = "open_view";
        command_arg.url = (*it);
        command_arg.arg1 = "true";   // タブで開く
        command_arg.arg2 = "false";  // 既に開いているかチェック
        command_arg.arg3 = "noswitch";  // タブを切り替えない

#ifdef _DEBUG
        std::cout << "url = " << command_arg.url << std::endl;
#endif

        open_view( command_arg );

        // 一番最初のページは普通にオンラインで開く
        // 二番目からは ウェイトを入れてリロード
        if( online ){
            if( !waittime ) SESSION::set_online( false );
            else set_autoreload_mode( command_arg.url, AUTORELOAD_ONCE, waittime );
        }
    }

    SESSION::set_online( online );

    switch_admin();
    switch_view( *( list_url.begin() ) );
}


//
// 移転などでviewのurlを更新
//
void Admin::update_url( const std::string& url_old, const std::string& url_new )
{
#ifdef _DEBUG
    std::cout << "Admin::update_url " << url_old << " -> " << url_new << std::endl;
#endif

    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url().find( url_old ) == 0 ) view->update_url( url_old, url_new );
        }
    }
}


//
// urlを含むviewの板名を更新
//
void Admin::update_boardname( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::update_boardname url = " << url << std::endl;
#endif

    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url().find( url ) == 0 ) view->update_boardname();
        }

        redraw_toolbar();
    }
}


//
// URLやステータスを更新
//
void Admin::update_status( View* view, const bool force )
{
    if( view ){
        set_title( view->get_url(), view->get_title(), force );
        set_url( view->get_url(), view->url_for_copy(), force );
        set_status( view->get_url(), view->get_status(), force );
    }
}



//
// ビューを開く
//
// command.arg1: 開く位置
//
// "false" ならアクティブなタブを置き換える
// "true" なら一番右側に新しいタブを開く
// "right" ならアクティブなタブの右に開く
// "left" ならアクティブな左に開く
// "page数字" なら指定した位置にタブで開く
//
// command.arg2: "true" なら既に command.url を開いているかチェックしない
//
// command.arg3: モード
//
// "auto"なら表示されていればリロードせずに切替える
// されていなければarg1で指定した場所に新しいタブで開いてロード
// スレ番号ジャンプなどで使用する
//
// "noswitch"ならタブを切り替えない(連続して開くときに使用)
//
// "lock" なら開いてからロックする
//
// "offline" なら オフラインで開く
//
//
// その他のargは各ビュー別の設定
//
void Admin::open_view( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "Admin::open_view : " << command.url << std::endl
              << "arg1 = " << command.arg1 << std::endl
              << "arg2 = " << command.arg2 << std::endl 
              << "arg3 = " << command.arg3 << std::endl;   
#endif

    SKELETON::View* view;
    SKELETON::View* current_view = get_current_view();

    const bool online = SESSION::is_online();
    const bool nocheck_opened = ( command.arg2 == "true" );
    int mode = 0;
    if( ! command.arg3.empty() ){

        if( command.arg3.find( "auto" ) != std::string::npos ) mode |= OPEN_MODE_AUTO;
        if( command.arg3.find( "noswitch" ) != std::string::npos ) mode |= OPEN_MODE_NOSWITCH;
        if( command.arg3.find( "lock" ) != std::string::npos ) mode |= OPEN_MODE_LOCK;
        if( command.arg3.find( "offline" ) != std::string::npos ) mode |= OPEN_MODE_OFFLINE;
    }

    if( current_view ) current_view->focus_out();

    // urlを既に開いていたら表示してリロード
    if( ! nocheck_opened ){

        view = get_view( command.url );
        if( view ){

            // タブの切り替え
            if( ! ( mode & OPEN_MODE_NOSWITCH ) ){
            
                int page = m_notebook->page_num( *view );
#ifdef _DEBUG
                std::cout << "page = " << page << std::endl;
#endif        
                set_current_page( page );
                switch_admin();
            }

            // オートモードは切り替えのみ
            if( mode & OPEN_MODE_AUTO ) return;

            // ロック
            if( mode & OPEN_MODE_LOCK ) lock( m_notebook->page_num( *view ) );

            // オフラインで開く
            if( mode & OPEN_MODE_OFFLINE ) SESSION::set_online( false );

            view->show_view();
            SESSION::set_online( online );

            return;
        }
    }

    view = create_view( command );
    if( !view ) return;

    const int page = m_notebook->get_current_page();
    const bool open_tab = (  page == -1
                             || ( ! command.arg1.empty() && command.arg1 != "false" )
                             || ( mode & OPEN_MODE_AUTO ) // オートモードの時もタブで開く
                             || is_locked( page )
        );

    // ツールバー表示
    if( page == -1 ) show_toolbar();

    // タブで表示
    if( open_tab ){

        int openpage = -1;

        // 現在のページの右に表示
        if( command.arg1 == "right" ) openpage = page +1;

        // 現在のページの左に表示
        else if( command.arg1 == "left" ) openpage = page;

        // 指定した位置に表示
        else if( command.arg1.find( "page" ) == 0 ) openpage = atoi( command.arg1.c_str() + 4 );

        // ロックされていたら右に表示
        else if( command.arg1 != "true" && page != -1 && is_locked( page ) ) openpage = page +1;

#ifdef _DEBUG
        std::cout << "append page = " << openpage << std::endl;
#endif


        if( page != -1 ) m_notebook->insert_page( command.url, *view, openpage );

        // 最後に表示
        else m_notebook->append_page( command.url, *view );

        if( m_use_viewhistory ){

            if( m_last_closed_url.empty() ) HISTORY::get_history_manager()->create_viewhistory( view->get_url() );

            // 直前に閉じたViewの履歴を次に開いたViewに引き継ぐ
            else{
                HISTORY::get_history_manager()->append_viewhistory( m_last_closed_url, view->get_url() );    
                m_last_closed_url = std::string();
            }
        }
    }

    // 開いてるviewを消してその場所に表示
    else{

#ifdef _DEBUG
        std::cout << "replace page\n";
#endif
        // タブ入れ替え
        m_notebook->insert_page( command.url, *view, page );

        m_notebook->remove_page( page + 1 );

        if( current_view ){

            std::string url_current = current_view->get_url();
            delete current_view;

            if( m_use_viewhistory ) HISTORY::get_history_manager()->append_viewhistory( url_current, view->get_url() );    
        }
    }

    m_notebook->show_all();
    view->show();

    // オフラインで開く
    if( mode & OPEN_MODE_OFFLINE ) SESSION::set_online( false );

    view->show_view();
    SESSION::set_online( online );

    // ツールバーの情報更新
    m_notebook->set_current_toolbar( view->get_id_toolbar(), view );

    // タブの切り替え
    if( ! ( mode & OPEN_MODE_NOSWITCH ) ){
        switch_admin();
        set_current_page( m_notebook->page_num( *view ) );
    }

    // ロック
    if( mode & OPEN_MODE_LOCK ) lock( m_notebook->page_num( *view ) );
}


//
// ツールバーの検索ボックスをフォーカス
//
void Admin::focus_toolbar_search()
{
    m_notebook->focus_toolbar_search();
}


//
// ツールバー表示更新
//
void Admin::redraw_toolbar()
{
    SKELETON::View* view = get_current_view();
    if( view ) m_notebook->set_current_toolbar( view->get_id_toolbar(), view );
}


//
// ツールバーのボタン表示更新
//
void Admin::update_toolbar_button()
{
    m_notebook->update_toolbar_button();
    redraw_toolbar();
}


//
// ビュー切り替え
//
// 指定したURLのビューに切り替える
//
void Admin::switch_view( const std::string& url )
{
    SKELETON::View* view = get_view( url );
    if( view ){
            
        int page = m_notebook->page_num( *view );
        set_current_page( page );
    }
}



//
// タブ左移動
//
void Admin::tab_left()
{
    int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    int page = m_notebook->get_current_page();
    if( page == -1 ) return;

    if( page == 0 ) page = pages;

    set_current_page( --page );
}



//
// タブ右移動
//
void Admin::tab_right()
{
    int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    int page = m_notebook->get_current_page();
    if( page == -1 ) return;

    if( page == pages -1 ) page = -1;

    set_current_page( ++page );
}



//
// タブ先頭移動
//
void Admin::tab_head()
{
    int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    set_current_page( 0 );
}



//
// タブ最後に移動
//
void Admin::tab_tail()
{
    int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    set_current_page( pages-1 );
}




//
// ビューを再描画
//
void Admin::redraw_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::redraw_view : " << m_url << " : url = " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ) view->redraw_view();
}



//
// 現在のビューを再描画
//
void Admin::redraw_current_view()
{
#ifdef _DEBUG
    std::cout << "Admin::redraw_current_view : " << m_url << std::endl;
#endif

    SKELETON::View* view = get_current_view();
    if( view ) view->redraw_view();
}


//
// 現在のビューを再レイアウト
//
void Admin::relayout_current_view()
{
#ifdef _DEBUG
    std::cout << "Admin::relayout_current_view : " << m_url << std::endl;
#endif

    SKELETON::View* view = get_current_view();
    if( view ) view->relayout();
}


//
// urlを含むビューを検索してそれがカレントならば再描画
//
void Admin::redraw_views( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::redraw_view : " << m_url << " : url = " << url << std::endl;
#endif

    SKELETON::View* current_view = get_current_view();
    std::list< SKELETON::View* > list_view = get_list_view( url );

    std::list< SKELETON::View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){
        if( ( *it ) == current_view ) ( *it )->redraw_view();
    }
}




//
// ビューを閉じる
//
void Admin::close_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::close_view : " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    close_view( view );
}


void Admin::close_view( SKELETON::View* view )
{
    if( !view ) return;

    int page = m_notebook->page_num( *view );
    int current_page = m_notebook->get_current_page();

    if( is_locked( page ) ) return;

    // もし現在表示中のビューを消すときは予めひとつ右のビューにスイッチしておく
    // そうしないと左のビューを一度表示してしまうので遅くなる
    if( page == current_page ){
        SKELETON::View* newview = dynamic_cast< View* >( m_notebook->get_nth_page( page + 1 ) );
        if( newview ) switch_view( newview->get_url() );
    }

    m_notebook->remove_page( page );

    if( m_use_viewhistory ){

        // 直前に閉じたViewの履歴を次に開いたViewに引き継ぐ
        if( ! m_last_closed_url.empty() ) HISTORY::get_history_manager()->delete_viewhistory( m_last_closed_url );
        m_last_closed_url = view->get_url();
    }

    delete view;

#ifdef _DEBUG
    std::cout << "Admin::close_view : delete page = " << page << std::endl;
#endif

    // 全てのビューが無くなったらコアに知らせる
    if( empty() ){
#ifdef _DEBUG
        std::cout << "empty\n";
#endif

        m_notebook->hide_toolbar();

        CORE::core_set_command( "empty_page", m_url );
    }
}



//
// url を含むビューを全てアンロック
//
void Admin::unlock_all_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::unlock_all_view : " << url << std::endl;
#endif

    std::list< View* > list_view = get_list_view( url );

    std::list< View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){

        SKELETON::View* view = ( *it );
        if( view && view->is_locked() ){
            view->unlock();
            redraw_toolbar();
        }
    }
}


//
// url を含むビューを全て閉じる
//
void Admin::close_all_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::close_all_view : " << url << std::endl;
#endif

    std::list< View* > list_view = get_list_view( url );

    std::list< View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){

        SKELETON::View* view = ( *it );
        close_view( view );
    }
}



//
// 現在のビューを閉じる
//
void Admin::close_current_view()
{
#ifdef _DEBUG
    std::cout << "Admin::close_current_view : " << m_url << std::endl;
#endif

    SKELETON::View* view = get_current_view();
    if( view ) close_view( view->get_url() );
}


//
// ビューを更新する
//
void Admin::update_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::update_view : " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ) view->update_view();
}


//
// ビューの一部(例えばBoardViewなら行など)を更新
//
void Admin::update_item( const std::string& url,  const std::string& id ) 
{
#ifdef _DEBUG
    std::cout << "Admin::update_item : " << url << " " << id << std::endl;
#endif

    std::list< SKELETON::View* > list_view = get_list_view();
    std::list< SKELETON::View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){
        SKELETON::View* view = ( *it );
        if( view ) view->update_item( url, id );
    }

}


//
// ビューに更新終了を知らせる
//
// update_view はデータロード時などの更新途中でも呼び出されるが
// update_finish は最後に一度だけ呼び出される
//
void Admin::update_finish( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::update_finish : " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ) view->update_finish();
}


//
// タイトル表示
//
void Admin::set_title( const std::string& url, const std::string& title, const bool force )
{
    if( m_win ) m_win->set_title( "JD - " + title );
    else{

        SKELETON::View* view = get_current_view();
        if( view ){

            // アクティブなviewからコマンドが来たら表示する
            if( force || ( m_focus && view->get_url() == url ) ) CORE::core_set_command( "set_title", url, title );
        }
    }
}


//
// URLバーにアドレス表示
//
void Admin::set_url( const std::string& url, const std::string& url_show, const bool force )
{
    if( m_win ){}
    else{

        SKELETON::View* view = get_current_view();
        if( view ){

            // アクティブなviewからコマンドが来たら表示する
            if( force || ( m_focus && view->get_url() == url ) ) CORE::core_set_command( "set_url", url_show );
        }
    }
}


//
// ステータス表示
//
// virtual
void Admin::set_status( const std::string& url, const std::string& stat, const bool force )
{
    if( m_win ) m_win->set_status( stat );
    else{
        
        SKELETON::View* view = get_current_view();
        if( view ){
            
            // アクティブなviewからコマンドが来たら表示する
            if( force || ( m_focus && view->get_url() == url ) ){
                CORE::core_set_command( "set_status", url, stat );
                CORE::core_set_command( "set_mginfo", "", "" );
            }
        }
    }
}


//
// フォーカスする
//
// タイトルやURLバーやステータス表示も更新する
//
void Admin::focus_view( int page )
{
#ifdef _DEBUG
    std::cout << "Admin::focus_view : " << m_url << " page = " << page << std::endl;
#endif

    if( m_win ) m_win->focus_in();
    
    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) {
        view->focus_view();
        update_status( view, false );
    }
}



//
// 現在のviewをフォーカスする
//
void Admin::focus_current_view()
{
    if( ! m_focus ) return;

#ifdef _DEBUG
    std::cout << "Admin::focus_current_view : " << m_url << std::endl;
#endif

    int page = m_notebook->get_current_page();
    focus_view( page );
}



//
// フォーカスアウトしたあとにフォーカス状態を回復する
//
// focus_current_view()と違ってURLバーやステータスを再描画しない
//
void Admin::restore_focus()
{
#ifdef _DEBUG
    std::cout << "Admin::restore_focus : " << m_url << std::endl;
#endif

    int page = m_notebook->get_current_page();
    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) view->focus_view();
}



//
// 現在のviewをフォーカスアウトする
// メインウィンドウがフォーカスアウトしたときなどに呼ばれる
//
void Admin::focus_out()
{
#ifdef _DEBUG
    std::cout << "Admin::focus_out : " << m_url << std::endl;
#endif

    // ウィンドウ表示の時、ビューが隠れないようにフォーカスアウトする前に transient 指定をしておく
    if( get_jdwin() ) get_jdwin()->set_transient( true );

    SKELETON::View* view = get_current_view();
    if( view ) view->focus_out();
    m_notebook->focus_out();
}



//
// タブラベル更新
//
void Admin::set_tablabel( const std::string& url, const std::string& str_label )
{
#ifdef _DEBUG
    std::cout << "Admin::set_tablabel : " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ){

        m_notebook->set_tab_fulltext( str_label, m_notebook->page_num( *view ) );

        // View履歴のタイトルも更新
        if( m_use_viewhistory ){
            HISTORY::get_history_manager()->replace_current_title_viewhistory( view->get_url(), str_label );
        }
    }
}



//
// 再レイアウト実行
//
void Admin::relayout_all()
{
    std::list< SKELETON::View* > list_view = get_list_view();
    std::list< SKELETON::View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){
        SKELETON::View* view = ( *it );
        if( view ) view->relayout();
    }
}



//
// タブ表示切り替え
//
void Admin::toggle_tab()
{
    if( m_notebook->get_show_tabs() ) m_notebook->set_show_tabs( false );
    else m_notebook->set_show_tabs( true );
}


//
// アイコン表示切り替え
//
void Admin::toggle_icon( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "Admin::toggle_icon url = " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ){

        std::string iconname = "default";

        // まだロード中
        if( view->is_loading() ) iconname = "loading";

        // オートリロードモードでロード待ち
        else if( view->get_autoreload_mode() != AUTORELOAD_NOT ) iconname = "loading_stop";

        // 更新可能
        else if( view->is_check_update() ) iconname = "update";

        // 更新済み
        else if( view->is_updated() ){

            // タブがアクティブの時は通常アイコンを表示
            if( get_notebook()->page_num( *view ) == get_notebook()->get_current_page() ) iconname = "default";
            else iconname = "updated";
        }

        // 古い
        else if( view->is_old() ) iconname = "old";

#ifdef _DEBUG
        std::cout << "name = " << iconname << std::endl;
#endif
        int id = view->get_icon( iconname );
        get_notebook()->set_tabicon( iconname, get_notebook()->page_num( *view ), id );
    }
}


//
// オートリロードのモード設定
//
// 成功したらtrueを返す
//
// mode : モード (global.h　参照)
// sec :  リロードまでの秒数
//
bool Admin::set_autoreload_mode( const std::string& url, int mode, int sec )
{
    SKELETON::View* view = get_view( url );
    if( view ){

        if( mode == AUTORELOAD_NOT && view->get_autoreload_mode() == AUTORELOAD_NOT ) return false;

        view->set_autoreload_mode( mode, sec );

        // モード設定が成功したらアイコン変更
        if( view->get_autoreload_mode() == mode ){

            toggle_icon( view->get_url() );
            return true;
        }
    }

    return false;
}



//
// ビュークラス取得
//
View* Admin::get_view( const std::string& url )
{
    SKELETON::View* view = get_current_view();
    if( view && view->get_url() == url ) return view;

    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url() == url ) return view;
        }
    }
    
    return NULL;
}



//
// ビュークラスのリスト取得
//
// url を含むビューのリストを返す
//
std::list< View* > Admin::get_list_view( const std::string& url )
{
    std::list< View* > list_view;
    
    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url().find ( url ) != std::string::npos ) list_view.push_back( view );
        }
    }
    
    return list_view;
}



//
// 全てのビュークラスのリスト取得
//
//
std::list< View* > Admin::get_list_view()
{
    std::list< View* > list_view;
    
    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view ) list_view.push_back( view );
        }
    }
    
    return list_view;
}




//
// 現在表示されているビュークラス取得
//
View* Admin::get_current_view()
{
    int page = m_notebook->get_current_page();
    if( page == -1 ) return NULL;
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );

    return view;
}




//
// 指定したページに表示切替え
//
void Admin::set_current_page( int page )
{
    m_notebook->set_current_page( page );
}



//
// 現在表示されているページ番号
//
int Admin::get_current_page()
{
    return m_notebook->get_current_page();
}


//
// 現在表示されているページのURL
//
std::string Admin::get_current_url()
{
    SKELETON::View* view = get_current_view();
    if( ! view ) return std::string();
    return view->get_url();
}


//
// notebookのタブのページが切り替わったら呼ばれるslot
//
void Admin::slot_switch_page( GtkNotebookPage*, guint page )
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "Admin::slot_switch_page : " << m_url << " page = " << page << std::endl;
#endif

    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ){
#ifdef _DEBUG
        std::cout << "url = " << view->get_url() << std::endl;
#endif

        // ツールバー表示切り替え
        m_notebook->set_current_toolbar( view->get_id_toolbar(), view );

        // タブのアイコンを通常に戻して再描画
        toggle_icon( view->get_url() );
        view->redraw_view();
        if( m_focus ){
            update_status( view, false );
            focus_current_view();
        }

        CORE::core_set_command( "page_switched", m_url, view->get_url() );
    }
}


//
// タブをクリック
//
void Admin::slot_tab_click( int page )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_click " << page << std::endl;
#endif

    set_current_page( page );
}


//
// タブを閉じる
//
void Admin::slot_tab_close( int page )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_close " << page << std::endl;
#endif

    // 閉じる
    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) close_view( view->get_url() );
}


//
// タブ再読み込み
//
void Admin::slot_tab_reload( int page )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_reload " << page << std::endl;
#endif

    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) view->reload();
}


//
// タブメニュー表示
//
void Admin::slot_tab_menu( int page, int x, int y )
{
    if( ! m_ui_manager ) return;

#ifdef _DEBUG
    std::cout << "Admin::slot_tab_menu " << page << std::endl;
#endif

    Glib::RefPtr< Gtk::Action > act;
    m_clicked_page = -1; // メニューのactive状態を変えたときにslot関数が呼び出されるのをキャンセル

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );

    // ロック
    act = m_action_group->get_action( "LockTab" );
    if( page >= 0 && act ){

        if( ! is_lockable( page ) ) act->set_sensitive( false );
        else{
            act->set_sensitive( true );

            Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
            if( is_locked( page ) ) tact->set_active( true );
            else tact->set_active( false );
        }
    }

    // 閉じる
    act = m_action_group->get_action( "Quit" );
    if( act ){
        if( is_locked( page ) ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // 進む、戻る
    if( view ){
        act = m_action_group->get_action( "PrevView" );
        if( act ){
            if( HISTORY::get_history_manager()->can_back_viewhistory( view->get_url(), 1 ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }
        act = m_action_group->get_action( "NextView" );
        if( act ){
            if( HISTORY::get_history_manager()->can_forward_viewhistory( view->get_url(), 1 ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }
    }

    m_clicked_page = page;

    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu" ) );
    if( popupmenu ){

        // menu item をサブメニューから取り除く
        for( int i = 0; i < MAX_TABS ; ++i ){
            if( m_vec_movemenu_append[ i ] )m_move_menu->remove( *m_vec_movemenu_items[ i ] );
            m_vec_movemenu_append[ i ] = false;
        }

        // 移動サブメニューにタブ名をセットする
        int pages = m_notebook->get_n_pages();
        for( int i = 0; i < MIN( pages, MAX_TABS ); ++ i ){

            Gtk::Label* label = dynamic_cast< Gtk::Label* >( m_vec_movemenu_items[ i ]->get_child() );
            if( label ){

                std::string name = m_notebook->get_tab_fulltext( i );
                if( name.empty() ) name = "???";
                const unsigned int maxsize = 50;
                label->set_text( MISC::cut_str( name, maxsize ) );
            }
            m_move_menu->append( *m_vec_movemenu_items[ i ] );
            m_vec_movemenu_append[ i ] = true;
        }

        // コメント更新
        Gtk::Label* label = dynamic_cast< Gtk::Label* >( m_vec_movemenu_items[ MAX_TABS ]->get_child() );
        if( label ) label->set_text_with_mnemonic( "移動 [ タブ数 " + MISC::itostr( pages ) +" ](_M)" );

        m_move_menu->show_all();

        popupmenu->popup( 0, gtk_get_current_event_time() );
    }
}


//
// 右クリックメニューの閉じる
//
void Admin::slot_close_tab()
{
    if( m_clicked_page < 0 ) return;

#ifdef _DEBUG
    std::cout << "Admin::slot_close_tab " << m_clicked_page << std::endl;
#endif

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) close_view( view->get_url() );
}


//
// ロック
//
void Admin::slot_lock()
{
    if( m_clicked_page < 0 ) return;

    if( is_locked( m_clicked_page ) ) unlock( m_clicked_page );
    else lock( m_clicked_page );
}


//
// 右クリックメニューの他を閉じる
//
void Admin::slot_close_other_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_other_tabs " << m_clicked_page << std::endl;
#endif

    std::string url;
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) url = view->get_url();

    int pages = m_notebook->get_n_pages();
    for( int i = 0; i < pages; ++i ){
        view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view && view->get_url() != url ) set_command( "close_view", view->get_url() );
    }
}



//
// 右クリックメニューの左を閉じる
//
void Admin::slot_close_left_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_left_tabs " << m_clicked_page << std::endl;
#endif

    for( int i = 0; i < m_clicked_page; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }
}


//
// 右クリックメニューの右を閉じる
//
void Admin::slot_close_right_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_right_tabs " << m_clicked_page << std::endl;
#endif

    int pages = m_notebook->get_n_pages();
    for( int i = m_clicked_page +1; i < pages; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }
}




//
// 右クリックメニューの全てを閉じる
//
void Admin::slot_close_all_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_all_tabs " << m_clicked_page << std::endl;
#endif

    int pages = m_notebook->get_n_pages();
    for( int i = 0; i < pages; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }
}


//
// 右クリックメニューの全てのタブの更新チェック
//
void Admin::slot_check_update_all_tabs()
{
    check_update_all_tabs( m_clicked_page );

    CORE::get_checkupdate_manager()->run( false );
}


//
// 右クリックメニューの全てのタブの更新チェックと再読み込み
//
void Admin::slot_check_update_reload_all_tabs()
{
    check_update_all_tabs( m_clicked_page );

    CORE::get_checkupdate_manager()->run( true );
}


//
// from_page から全てのタブを更新チェック
//
// この後で CORE::get_checkupdate_manager()->run() すること
//
void Admin::check_update_all_tabs( const int from_page )
{
#ifdef _DEBUG
    std::cout << "Admin::check_update_all_tabs from = " << from_page << std::endl;
#endif

    if( ! SESSION::is_online() ) return;

    int pages = m_notebook->get_n_pages();

    // クリックしたタブから右側
    for( int i = from_page ; i < pages; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view && view->get_enable_autoreload() ) CORE::get_checkupdate_manager()->push_back( view->get_url() );
    }

    // クリックしたタブから左側
    for( int i = 0 ; i < from_page; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view && view->get_enable_autoreload() ) CORE::get_checkupdate_manager()->push_back( view->get_url() );
    }
}


//
// 右クリックメニューの全ての再読み込み
//
void Admin::slot_reload_all_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_reload_all_tabs " << m_clicked_page << std::endl;
#endif

    reload_all_tabs( m_clicked_page );
}


//
// 開いているタブから全てのタブを再読み込み
//
void Admin::reload_all_tabs()
{
    reload_all_tabs( m_notebook->get_current_page() );
}


//
// from_page から全てのタブを再読み込み
//
void Admin::reload_all_tabs( const int from_page )
{
#ifdef _DEBUG
    std::cout << "Admin::reload_all_tabs from = " << from_page << std::endl;
#endif

    if( ! SESSION::is_online() ) return;

    int waittime = 0;
    int pages = m_notebook->get_n_pages();

    // クリックしたタブから右側
    for( int i = from_page ; i < pages; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ){
            if( set_autoreload_mode( view->get_url(), AUTORELOAD_ONCE, waittime ) ) waittime += AUTORELOAD_MINSEC;
        }
    }

    // クリックしたタブから左側
    for( int i = 0 ; i < from_page; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ){
            if( set_autoreload_mode( view->get_url(), AUTORELOAD_ONCE, waittime ) ) waittime += AUTORELOAD_MINSEC;
        }
    }
}


//
// 右クリックメニューの更新キャンセル
//
void Admin::slot_cancel_reload_all_tabs()
{
    for( int i = 0 ; i < m_notebook->get_n_pages(); ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ) set_autoreload_mode( view->get_url(), AUTORELOAD_NOT, 0 );
    }

    CORE::get_checkupdate_manager()->stop();
}


//
// 右クリックメニューのブラウザで開く
//
void Admin::slot_open_by_browser()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_open_by_browser " << m_clicked_page << std::endl;
#endif

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) CORE::core_set_command( "open_url_browser", view->url_for_copy() );
}


//
// 右クリックメニューのURLコピー
//
void Admin::slot_copy_url()
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) MISC::CopyClipboard( view->url_for_copy() );
}



//
// 右クリックメニューのタイトルとURLコピー
//
void Admin::slot_copy_title_url()
{
    std::string str = m_notebook->get_tab_fulltext( m_clicked_page );
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) str += "\n" + view->url_for_copy();
    
    MISC::CopyClipboard( str );
}


// ページがロックされているかリストで取得
std::list< bool > Admin::get_locked()
{
    std::list< bool > locked;
    
    int pages = m_notebook->get_n_pages();
    if( pages ){
        for( int i = 0; i < pages; ++i ) locked.push_back( is_locked( i ) );
    }

    return locked;
}

// タブのロック/アンロック
const bool Admin::is_lockable( const int page )
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) return view->is_lockable();

    return false;
}

const bool Admin::is_locked( const int page )
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) return view->is_locked();

    return false;
}

const bool Admin::is_locked( const std::string& url )
{
    SKELETON::View* view = get_view( url );
    if( view ) return view->is_locked();

    return false;
}

void Admin::lock( const int page )
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ){
        view->lock();
        redraw_toolbar();
    }
}

void Admin::unlock( const int page )
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ){
        view->unlock();
        redraw_toolbar();
    }
}

// プロパティ表示
void Admin::show_preference()
{
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) view->show_preference();
}

//
// View履歴:戻る
//
bool Admin::back_viewhistory( const std::string& url, const int count )
{
    return back_forward_viewhistory( url, true, count );
}


void Admin::back_clicked_viewhistory( const int count )
{
    if( m_clicked_page < 0 ) return;

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( ! view ) return;
    
    m_notebook->set_current_page( m_clicked_page );
    back_forward_viewhistory( view->get_url(), true, count );
}


//
// View履歴進む
//
bool Admin::forward_viewhistory( const std::string& url, const int count )
{
    return back_forward_viewhistory( url, false, count );
}


void Admin::forward_clicked_viewhistory( const int count )
{
    if( m_clicked_page < 0 ) return;

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( ! view ) return;

    m_notebook->set_current_page( m_clicked_page );
    back_forward_viewhistory( view->get_url(), false, count );
}


//
// 戻る、進む
//
bool Admin::back_forward_viewhistory( const std::string& url, const bool back, const int count )
{
    if( ! m_use_viewhistory ) return false;

#ifdef _DEBUG
    std::cout << "Admin::back_forward_viewhistory back = " << back
              << "count = " << count << " tab = " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ){

        if( view->is_locked() ) return false;

        const HISTORY::ViewHistoryItem* historyitem;

        if( back ) historyitem = HISTORY::get_history_manager()->back_viewhistory( url, count, false );
        else historyitem = HISTORY::get_history_manager()->forward_viewhistory( url, count, false );

        if( historyitem && ! historyitem->url.empty() ){
#ifdef _DEBUG
            std::cout << "open : " << historyitem->url << std::endl;
#endif

            // 既にタブで開いている場合
            if( get_view( historyitem->url) ){

                // 次のviewを開けるかチェック
                bool enable_next = false;
                if( back ) enable_next = HISTORY::get_history_manager()->can_back_viewhistory( url, count +1 );
                else enable_next = HISTORY::get_history_manager()->can_forward_viewhistory( url, count +1 );

                SKELETON::MsgDiag mdiag( get_win(), historyitem->title + "\n\nは既にタブで開いています",
                                         false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
                mdiag.add_button( "タブを開く(_T)", Gtk::RESPONSE_YES );
                if( enable_next ) mdiag.add_button( "次を開く(_N)", Gtk::RESPONSE_NO );
                mdiag.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );

                int ret = mdiag.run();
                mdiag.hide();

                switch( ret ){

                    case Gtk::RESPONSE_YES:
                        switch_view( historyitem->url );
                        return false;

                    case Gtk::RESPONSE_NO:
                        return back_forward_viewhistory( url, back, count + 1 );

                    default:
                        break;
                }

                return false;
            }

            // 検索ビューなど、back/forwardしたときに view のurlが変わることがあるので
            // view履歴内のURLを置き換える必要がある。そこで Admin::open_view() 中の
            // append_viewhistory() を実行しないでここで View履歴の現在位置を変更する
            // Admin::Open_view()も参照すること
            const bool use_history = get_use_viewhistory();
            set_use_viewhistory( false );

            if( use_history ){
                if( back ) HISTORY::get_history_manager()->back_viewhistory( url, count, true );
                else HISTORY::get_history_manager()->forward_viewhistory( url, count, true );
            }

            COMMAND_ARGS command_arg = url_to_openarg( historyitem->url, false, false );
            open_view( command_arg );

            // URLが変わっていたらview履歴内のURLも更新
            if( use_history ){

                SKELETON::View* current_view = get_current_view();
                if( current_view && current_view->get_url() != historyitem->url ){
                    HISTORY::get_history_manager()->replace_current_url_viewhistory( historyitem->url, current_view->get_url() );
                    redraw_toolbar();
                }

                set_use_viewhistory( true );
            }

            return true;
        }
    }

    return false;
}


//
// View履歴削除
//
void Admin::clear_viewhistory()
{
    if( ! m_use_viewhistory ) return;

    std::list< SKELETON::View* > list_view = get_list_view();
    std::list< SKELETON::View* >::iterator it = list_view.begin();
    for( ; it != list_view.end(); ++it ){
        SKELETON::View* view = ( *it );
        if( view ){
            HISTORY::get_history_manager()->delete_viewhistory( (*it)->get_url() );
            HISTORY::get_history_manager()->create_viewhistory( (*it)->get_url() );

            int page = m_notebook->page_num( *(*it) );
            std::string str_label = m_notebook->get_tab_fulltext( page );
            HISTORY::get_history_manager()->replace_current_title_viewhistory( (*it)->get_url(), str_label );
        }
    }

    if( ! m_last_closed_url.empty() ) HISTORY::get_history_manager()->delete_viewhistory( m_last_closed_url );
    m_last_closed_url = std::string();

    update_toolbar_button();
}
