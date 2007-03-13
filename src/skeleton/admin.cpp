// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "admin.h"
#include "view.h"
#include "dragnote.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "command.h"
#include "session.h"
#include "global.h"
#include "controlutil.h"
#include "controlid.h"


#define MOVETAB_MENU { 5, 10, 15, 20, 25, 30 }
        
#define MAX_TABS 50

using namespace SKELETON;


Admin::Admin( const std::string& url )
    : m_url( url )
    , m_notebook( 0 )
    , m_focus( false )
{
    m_notebook = new DragableNoteBook();

    m_notebook->signal_switch_page().connect( sigc::mem_fun( *this, &Admin::slot_switch_page ) );
    m_notebook->set_scrollable( true );

    m_notebook->sig_tab_close().connect( sigc::mem_fun( *this, &Admin::slot_tab_close ) );
    m_notebook->sig_tab_reload().connect( sigc::mem_fun( *this, &Admin::slot_tab_reload ) );
    m_notebook->sig_tab_menu().connect( sigc::mem_fun( *this, &Admin::slot_tab_menu ) );

    // D&D
    m_notebook->sig_drag_begin().connect( sigc::mem_fun(*this, &Admin::slot_drag_begin ) );
    m_notebook->sig_drag_end().connect( sigc::mem_fun(*this, &Admin::slot_drag_end ) );

    m_list_command.clear();

    // 右クリックメニュー
    m_action_group = Gtk::ActionGroup::create();
    m_action_group->add( Gtk::Action::create( "Quit", "Quit" ), sigc::mem_fun( *this, &Admin::slot_close_tab ) );

    m_action_group->add( Gtk::Action::create( "CloseOther_Menu", "他のタブを閉じる" ) );
    m_action_group->add( Gtk::Action::create( "CloseOther", "閉じる" ), sigc::mem_fun( *this, &Admin::slot_close_other_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseLeft_Menu", "左のタブを閉じる" ) );
    m_action_group->add( Gtk::Action::create( "CloseLeft", "閉じる" ), sigc::mem_fun( *this, &Admin::slot_close_left_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseRight_Menu", "右のタブを閉じる" ) );
    m_action_group->add( Gtk::Action::create( "CloseRight", "閉じる" ), sigc::mem_fun( *this, &Admin::slot_close_right_tabs ) );
    m_action_group->add( Gtk::Action::create( "CloseAll_Menu", "全てのタブを閉じる" ) );
    m_action_group->add( Gtk::Action::create( "CloseAll", "閉じる" ), sigc::mem_fun( *this, &Admin::slot_close_all_tabs ) );

    m_action_group->add( Gtk::Action::create( "ReloadAll_Menu", "全てのタブを更新" ) );
    m_action_group->add( Gtk::Action::create( "ReloadAll", "更新する" ), sigc::mem_fun( *this, &Admin::slot_reload_all_tabs ) );
    m_action_group->add( Gtk::Action::create( "CancelReloadAll", "更新キャンセル" ),
                         sigc::mem_fun( *this, &Admin::slot_cancel_reload_all_tabs ) );


    m_action_group->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く" ), sigc::mem_fun( *this, &Admin::slot_open_by_browser ) );
    m_action_group->add( Gtk::Action::create( "CopyURL", "URLをコピー" ), sigc::mem_fun( *this, &Admin::slot_copy_url ) );
    m_action_group->add( Gtk::Action::create( "CopyTitle", "タイトルとURLをコピー" ), sigc::mem_fun( *this, &Admin::slot_copy_title_url ) );

    m_ui_manager = Gtk::UIManager::create();    
    m_ui_manager->insert_action_group( m_action_group );

    // ポップアップメニューのレイアウト
    Glib::ustring str_ui = 

    "<ui>"

    // 通常
    "<popup name='popup_menu'>"
    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='CloseAll_Menu'>"
    "<menuitem action='CloseAll'/>"
    "</menu>"
    "<menu action='CloseOther_Menu'>"
    "<menuitem action='CloseOther'/>"
    "</menu>"
    "<menu action='CloseLeft_Menu'>"
    "<menuitem action='CloseLeft'/>"
    "</menu>"
    "<menu action='CloseRight_Menu'>"
    "<menuitem action='CloseRight'/>"
    "</menu>"
    "<separator/>"

    "<menu action='ReloadAll_Menu'>"
    "<menuitem action='ReloadAll'/>"
    "</menu>"
    "<menuitem action='CancelReloadAll'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitle'/>"

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

    // 先頭、最後に移動
    item = Gtk::manage( new Gtk::MenuItem( "先頭に移動" ) );
    m_move_menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &Admin::tab_head ) );

    item = Gtk::manage( new Gtk::MenuItem( "最後に移動" ) );
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


Admin::~Admin()
{
#ifdef _DEBUG
    std::cout << "Admin::~Admin " << m_url << std::endl;
#endif
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

    if( m_notebook ) delete m_notebook;
}



Gtk::Widget* Admin::get_widget()
{
    return dynamic_cast< Gtk::Widget*>( m_notebook );
}



const bool Admin::is_booting()
{
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
std::list<std::string> Admin::get_URLs()
{
    std::list<std::string> urls;
    
    int pages = m_notebook->get_n_pages();
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
        restore();
    }

    // viewを開く
    else if( command.command == "open_view" ){
        open_view( command );
    }
    // リストで開く
    // arg1 にはdatファイルを空白で区切って指定する
    //
    else if( command.command == "open_list" ){
        open_list( command.arg1 );
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
    else if( command.command == "close_view" ){
        if( command.arg1 == "true" ) close_all_view( command.url );
        else close_view( command.url );
    }
    else if( command.command == "close_currentview" ){
        close_current_view();
    }
    else if( command.command == "set_page" ){
        set_current_page( atoi( command.arg1.c_str() ) );
    }

    // フォーカスイン、アウト
    if( command.command == "focus_current_view" ){
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

    // タブに文字をセット、タブ幅調整
    else if( command.command  == "set_tablabel" ){
        set_tablabel( command.url, command.arg1 );
    }
    else if( command.command  == "adjust_tabwidth" ){
        m_notebook->adjust_tabwidth();
    }

    // タブのアイコンをセット
    else if( command.command  == "set_tabicon" ){
        set_tabicon( command.url, command.arg1 );
    }

    // 全てのビューを再描画
    else if( command.command == "relayout_all" ){

        std::list< SKELETON::View* > list_view = get_list_view();
        std::list< SKELETON::View* >::iterator it = list_view.begin();
        for( ; it != list_view.end(); ++it ){
            SKELETON::View* view = ( *it );
            if( view ) view->relayout();
        }
    }

    // タイトル表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_title" ){

        SKELETON::View* view = get_current_view();
        if( m_focus && view && view->get_url() == command.url )
            CORE::core_set_command( "set_title", command.url, command.arg1 );
    }

    // ステータス表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_status" ){
        set_status( command.url, command.arg1 );
    }

    // オートリロードのキャンセル
    else if( command.command == "cancel_reload" ){
        slot_cancel_reload_all_tabs();
    }

    // タブを隠す
    else if( command.command == "hide_tabs" ) m_notebook->set_show_tabs( false );

    // 個別のコマンド処理
    else command_local( command );
}



// リストで与えられたページをタブで連続して開く
//
// 連続してリロードかけるとサーバに負担をかけるので、オフラインで開いて
// タイミングをずらしながらリロードする
//
void Admin::open_list( const std::string& str_list )
{
    std::list< std::string > list_url = MISC::split_line( str_list );
    if( list_url.empty() ) return;

    int waittime = 0;
    bool online = SESSION::is_online();

    std::list< std::string >::iterator it = list_url.begin();
    for( ; it != list_url.end(); ++it, waittime += AUTORELOAD_MINSEC ){

        COMMAND_ARGS command_arg = get_open_list_args( ( *it ) );
        open_view( command_arg );

        // 一番最初のページは普通にオンラインで開く
        // 二番目からは ウェイトを入れてリロード
        if( online ){
            if( !waittime ) SESSION::set_online( false );
            else set_autoreload_mode( command_arg.url, AUTORELOAD_ONCE, waittime );
        }
    }

    SESSION::set_online( online );
    switch_view( *( list_url.begin() ) );
}



//
// ビューを開く
//
// command.arg1: "true" なら新しいtabを開く, "right" ならアクティブなtabの右に、"left"なら左に開く
// command.arg2: "true" なら既にurlを開いているかチェックしない
// command.arg3: モード。"auto"なら表示されていればリロードせずに切替え、されていなければ新しいタブで開いてロード
//
// その他のargは各ビュー別の設定
//
void Admin::open_view( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "Admin::open_view : " << command.url << std::endl;
#endif
    SKELETON::View* view;
    SKELETON::View* current_view = get_current_view();

    // 現在のviewのフォーカスを外し、列幅保存
    if( current_view ) {
        current_view->focus_out();
    }

    // urlを既に開いていたら表示してリロード
    if( ! ( command.arg2 == "true" ) ){
        view = get_view( command.url );
        if( view ){
            
            int page = m_notebook->page_num( *view );
#ifdef _DEBUG
            std::cout << "page = " << page << std::endl;
#endif        
            set_current_page( page );
            switch_admin();

            // オートモードは切り替えのみ
            if( command.arg3 == "auto" ) return;

            view->show_view();
            focus_current_view();
            return;
        }
    }

    view = create_view( command );
    if( !view ) return;


    int page = m_notebook->get_current_page();
    bool open_tab = (  page == -1 || command.arg1 == "true" || command.arg1 == "right" || command.arg1 == "left"
                       || command.arg3 == "auto" // オートモードの時もタブで開く
        );

    // タブで表示
    if( open_tab ){

#ifdef _DEBUG
        std::cout << "append page\n";
#endif
        // 現在のページの右に表示
        if( page != -1 && command.arg1 == "right" ) m_notebook->insert_page( command.url, *view, page+1 );

        // 現在のページの左に表示
        else if( page != -1 && command.arg1 == "left" ) m_notebook->insert_page( command.url, *view, page );

        // 最後に表示
        else m_notebook->append_page( command.url, *view );
    }

    // 開いてるviewを消してその場所に表示
    else{
#ifdef _DEBUG
        std::cout << "replace page\n";
#endif
        m_notebook->insert_page( command.url, *view, page );

        m_notebook->remove_page( page + 1 );

        if( current_view ) delete current_view;
    }

    switch_admin();
    view->show();
    view->show_view();
    set_current_page( m_notebook->page_num( *view ) );
    focus_current_view();
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

    // もし現在表示中のビューを消すときは予めひとつ右のビューにスイッチしておく
    // そうしないと左のビューを一度表示してしまうので遅くなる
    if( page == current_page ){
        SKELETON::View* newview = dynamic_cast< View* >( m_notebook->get_nth_page( page + 1 ) );
        if( newview ) switch_view( newview->get_url() );
    }

    m_notebook->remove_page( page );

    delete view;

#ifdef _DEBUG
    std::cout << "Admin::close_view : delete page = " << page << std::endl;
#endif

    // 全てのビューが無くなったらコアに知らせる
    if( m_notebook->get_n_pages() == 0 ){
#ifdef _DEBUG
        std::cout << "empty\n";
#endif
        CORE::core_set_command( "empty_page", m_url );
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

    SKELETON::View* view = get_view( url );
    if( view ) view->update_item( id );
}


//
// ビューに更新終了を知らせる
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
// ステータス表示
//
void Admin::set_status( const std::string& url, const std::string& stat )
{
    // アクティブなviewからコマンドが来たら表示する
    SKELETON::View* view = get_current_view();
    if( m_focus && view && view->get_url() == url ){
        CORE::core_set_command( "set_status", url, stat );
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
    
    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ) {
        view->focus_view();
        CORE::core_set_command( "set_title", "", view->get_title() );
        CORE::core_set_command( "set_url", view->url_for_copy() );
        CORE::core_set_command( "set_status", "", view->get_status() );
    }
}



//
// 現在のviewをフォーカスする
//
void Admin::focus_current_view()
{
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
    if( view ) m_notebook->set_tab_fulltext( str_label, m_notebook->page_num( *view ) );
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

            if( mode != AUTORELOAD_NOT ) set_tabicon( view->get_url(), "loading_stop" );
            else set_tabicon( view->get_url(), "default" );

            return true;
        }
    }

    return false;
}



//
// ビュークラス取得
//
// use_find が true なら == の代わりに findを使う
//
View* Admin::get_view( const std::string& url, bool use_find )
{
    int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view ){
                if( view->get_url() == url ) return view;
                else if( use_find && view->get_url().find ( url ) != std::string::npos ) return view;
            }
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
#ifdef _DEBUG
    std::cout << "Admin::slot_switch_page : " << m_url << " page = " << page << std::endl;
#endif

    // タブのアイコンを通常に戻す
    SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
    if( view ){
#ifdef _DEBUG
        std::cout << "url = " << view->get_url() << std::endl;
#endif
        set_tabicon( view->get_url(), "switch_page" );
    }

    // coreにページが切り替わったことを知らせて、core経由でviewをフォーカスする
    CORE::core_set_command( "switch_page", m_url );
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
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_menu " << page << std::endl;
#endif

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
                if( ! name.empty() ){
                    const unsigned int maxsize = 50;
                    label->set_text( MISC::cut_str( name, maxsize ) );
                }
            }
            m_move_menu->append( *m_vec_movemenu_items[ i ] );
            m_vec_movemenu_append[ i ] = true;
        }

        // コメント更新
        Gtk::Label* label = dynamic_cast< Gtk::Label* >( m_vec_movemenu_items[ MAX_TABS ]->get_child() );
        if( label ) label->set_text( "移動 [ タブ数 " + MISC::itostr( pages ) +" ]" );

        m_move_menu->show_all();

        popupmenu->popup( 0, gtk_get_current_event_time() );
    }
}


//
// 右クリックメニューの閉じる
//
void Admin::slot_close_tab()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_tab " << m_clicked_page << std::endl;
#endif

    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) close_view( view->get_url() );
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
// 右クリックメニューの全てを更新
//
void Admin::slot_reload_all_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_reload_all_tabs " << m_clicked_page << std::endl;
#endif

    if( ! SESSION::is_online() ) return;

    int waittime = 0;
    int pages = m_notebook->get_n_pages();

    // クリックしたタブから右側
    for( int i = m_clicked_page ; i < pages; ++i ){
        SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
        if( view ){
            if( set_autoreload_mode( view->get_url(), AUTORELOAD_ONCE, waittime ) ) waittime += AUTORELOAD_MINSEC;
        }
    }

    // クリックしたタブから左側
    for( int i = 0 ; i < m_clicked_page; ++i ){
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
    if( view ) COPYCLIP( view->url_for_copy() );
}



//
// 右クリックメニューのタイトルとURLコピー
//
void Admin::slot_copy_title_url()
{
    std::string str = m_notebook->get_tab_fulltext( m_clicked_page );
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) str += "\n" + view->url_for_copy();
    
    COPYCLIP( str );
}
