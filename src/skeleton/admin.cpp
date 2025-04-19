// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "admin.h"
#include "window.h"
#include "view.h"
#include "dragnote.h"
#include "msgdiag.h"
#include "tabswitchmenu.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "history/historymanager.h"
#include "history/viewhistoryitem.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "config/globalconf.h"

#include "command.h"
#include "global.h"
#include "session.h"
#include "sign.h"
#include "updatemanager.h"

#include <algorithm>


// open_view で使うモード
enum
{
    OPEN_MODE_AUTO = 1,
    OPEN_MODE_NOSWITCH = 2,
    OPEN_MODE_LOCK = 4,
    OPEN_MODE_OFFLINE = 8,
    OPEN_MODE_REGET = 16
};


using namespace SKELETON;


Admin::Admin( const std::string& url )
    : m_url( url )
    , m_notebook{ std::make_unique<DragableNoteBook>() }
    , m_prev_n_pages{ -1 }
{
    m_notebook->signal_switch_page().connect( sigc::mem_fun( *this, &Admin::slot_switch_page ) );
    m_notebook->set_scrollable( true );

    m_notebook->sig_tab_clicked().connect( sigc::mem_fun( *this, &Admin::slot_tab_clicked ) );
    m_notebook->sig_tab_scrolled().connect( sigc::mem_fun( *this, &Admin::slot_tab_scrolled ) );
    m_notebook->sig_tab_close().connect( sigc::mem_fun( *this, &Admin::slot_tab_close ) );
    m_notebook->sig_tab_reload().connect( sigc::mem_fun( *this, &Admin::slot_tab_reload ) );
    m_notebook->sig_tab_menu().connect( sigc::mem_fun( *this, &Admin::slot_tab_menu ) );

    m_notebook->get_tabswitch_button().signal_clicked().connect( sigc::mem_fun(*this, &Admin::slot_show_tabswitchmenu ) );

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

    const int pages = m_notebook->get_n_pages();

#ifdef _DEBUG
    std::cout << "pages = " << pages << std::endl;
#endif
    if( pages ){

        for( int i = 0; i < pages; ++i ){

            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( 0 ) );

            m_notebook->remove_page( 0, false );

            if( view ) delete view;
        }
    }
    m_list_command.clear();

    Admin::close_window();
}


void Admin::save_session()
{
    std::list< SKELETON::View* > list_view = get_list_view();
    for( SKELETON::View* view : list_view ) {
        if( view ) view->save_session();
    }
}


//
// メニューのセットアップ
//
void Admin::setup_menu()
{
    // 右クリックメニュー
    m_action_group = Gio::SimpleActionGroup::create();
    m_action_group->add_action( "Quit", sigc::mem_fun( *this, &Admin::slot_close_tab ) );

    m_action_group->add_action_bool( "LockTab", sigc::mem_fun( *this, &Admin::slot_lock ), false );

    m_action_group->add_action( "CloseOther", sigc::mem_fun( *this, &Admin::slot_close_other_tabs ) );
    m_action_group->add_action( "CloseLeft", sigc::mem_fun( *this, &Admin::slot_close_left_tabs ) );
    m_action_group->add_action( "CloseRight", sigc::mem_fun( *this, &Admin::slot_close_right_tabs ) );
    m_action_group->add_action( "CloseAll", sigc::mem_fun( *this, &Admin::slot_close_all_tabs ) );
    m_action_group->add_action( "CloseSameIcon", sigc::mem_fun( *this, &Admin::slot_close_same_icon_tabs ) );

    m_action_group->add_action( "CheckUpdateAll", sigc::mem_fun( *this, &Admin::slot_check_update_all_tabs ) );
    m_action_group->add_action( "CheckUpdateReloadAll", sigc::mem_fun( *this, &Admin::slot_check_update_reload_all_tabs ) );
    m_action_group->add_action( "ReloadAll", sigc::mem_fun( *this, &Admin::slot_reload_all_tabs ) );
    m_action_group->add_action( "CancelReloadAll", sigc::mem_fun( *this, &Admin::slot_cancel_reload_all_tabs ) );

    m_action_group->add_action( "OpenBrowser", sigc::mem_fun( *this, &Admin::slot_open_by_browser ) );
    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &Admin::slot_copy_url ) );
    m_action_group->add_action( "CopyTitleURL", sigc::mem_fun( *this, &Admin::slot_copy_title_url ) );

    m_action_group->add_action( "AppendFavorite", sigc::mem_fun( *this, &Admin::slot_append_favorite ) );
    m_action_group->add_action( "Preference", sigc::mem_fun( *this, &Admin::show_preference ) );

    // 戻る、進む
    m_action_group->add_action( "PrevView", sigc::bind( sigc::mem_fun( *this, &Admin::back_clicked_viewhistory ), 1 ) );
    m_action_group->add_action( "NextView", sigc::bind( sigc::mem_fun( *this, &Admin::forward_clicked_viewhistory ), 1 ) );

    m_action_group->add_action( "tab-head-focus", sigc::mem_fun( *this, &Admin::tab_head_focus ) );
    m_action_group->add_action( "tab-tail-focus", sigc::mem_fun( *this, &Admin::tab_tail_focus ) );

    m_action_group->add_action_with_parameter( "tab-switch", Glib::Variant<gint32>::variant_type(),
                                               sigc::mem_fun( *this, &Admin::set_current_page_focus ) );

    m_notebook->insert_action_group( "admin", m_action_group );

    // ポップアップメニューのレイアウト
    Glib::ustring str_ui = R"(
<interface>
  <menu id="popup_menu">
    <section>
      <item>
        <attribute name="label" translatable="yes">タブをロックする(_K)</attribute>
        <attribute name="action">admin.LockTab</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_QUIT R"((_C)</attribute>
        <attribute name="action">admin.Quit</attribute>
      </item>
    </section>
    <section>
      <submenu>
        <attribute name="label" translatable="yes">複数のタブを閉じる(_T)</attribute>
        <item>
          <attribute name="label" translatable="yes">全てのタブ(_A)</attribute>
          <attribute name="action">admin.CloseAll</attribute>
        </item>
        <item>
          <attribute name="label" translatable="yes">他のタブ(_O)</attribute>
          <attribute name="action">admin.CloseOther</attribute>
        </item>
        <item>
          <attribute name="label" translatable="yes">左←のタブ(_L)</attribute>
          <attribute name="action">admin.CloseLeft</attribute>
        </item>
        <item>
          <attribute name="label" translatable="yes">右→のタブ(_R)</attribute>
          <attribute name="action">admin.CloseRight</attribute>
        </item>
        <item>
          <attribute name="label" translatable="yes">同じアイコンのタブ(_I)</attribute>
          <attribute name="action">admin.CloseSameIcon</attribute>
        </item>
      </submenu>
    </section>
    <section>
      <submenu>
        <attribute name="label" translatable="yes">全てのタブの再読み込み(_A)</attribute>
        <section>
          <item>
            <attribute name="label" translatable="yes">再読み込み(_R)</attribute>
            <attribute name="action">admin.ReloadAll</attribute>
          </item>
          <item>
            <attribute name="label" translatable="yes">更新チェックのみ(_U)</attribute>
            <attribute name="action">admin.CheckUpdateAll</attribute>
          </item>
          <item>
            <attribute name="label" translatable="yes">更新されたタブを再読み込み(_A)</attribute>
            <attribute name="action">admin.CheckUpdateReloadAll</attribute>
          </item>
        </section>
        <section>
          <item>
            <attribute name="label" translatable="yes">更新キャンセル(_C)</attribute>
            <attribute name="action">admin.CancelReloadAll</attribute>
          </item>
        </section>
      </submenu>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_OPEN_BROWSER R"((_W)</attribute>
        <attribute name="action">admin.OpenBrowser</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_COPY_URL R"((_U)</attribute>
        <attribute name="action">admin.CopyURL</attribute>
      </item>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_COPY_TITLE_URL R"((_L)</attribute>
        <attribute name="action">admin.CopyTitleURL</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">お気に入りに追加(_F)...</attribute>
        <attribute name="action">admin.AppendFavorite</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">プロパティ(_P)...</attribute>
        <attribute name="action">admin.Preference</attribute>
      </item>
    </section>
  </menu>
  <menu id="move_submenu">
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_BACK R"((_P)</attribute>
        <attribute name="action">admin.PrevView</attribute>
      </item>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_FORWARD R"((_N)</attribute>
        <attribute name="action">admin.NextView</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">先頭のタブに移動(_H)</attribute>
        <attribute name="action">admin.tab-head-focus</attribute>
      </item>
      <item>
        <attribute name="label" translatable="yes">最後のタブに移動(_T)</attribute>
        <attribute name="action">admin.tab-tail-focus</attribute>
      </item>
    </section>
  </menu>
</interface>)";

    // アクセラレーターキーやマウスジェスチャーの追加は行わない
    // CONTROL::set_menu_motion() はGTK4で廃止されるGtk::MenuのAPIを使っている
    auto builder = Gtk::Builder::create_from_string( str_ui );
    m_model_popup = Glib::RefPtr<Gio::Menu>::cast_dynamic( builder->get_object( "popup_menu" ) );

    // タブの切り替えメニュー
    m_model_tabswitch = SKELETON::TabSwitchMenu::create( m_notebook.get() );

    // 移動サブメニュー
    auto submenu = Glib::RefPtr<Gio::Menu>::cast_dynamic( builder->get_object( "move_submenu" ) );
    submenu->append_section( m_model_tabswitch );
    m_item_sub = Gio::MenuItem::create( ITEM_NAME_GO, submenu );

    m_model_popup->prepend_item( m_item_sub );
}


void Admin::slot_popupmenu_deactivate()
{
    if( m_model_tabswitch ) m_model_tabswitch->deactivate();
}


Gtk::Widget* Admin::get_widget()
{
    return static_cast<Gtk::Widget*>( m_notebook.get() );
}


Gtk::Window* Admin::get_win()
{
    return static_cast<Gtk::Window*>( m_win.get() );
}


void Admin::set_jdwin( std::unique_ptr<JDWindow> win )
{
    m_win = std::move( win );
}


void Admin::delete_jdwin()
{
    m_win.reset();
}


bool Admin::is_booting()
{
    if( get_jdwin() && get_jdwin()->is_booting() ) return true;

    return ( has_commands() );
}


//
// ページが含まれていないか
//
bool Admin::empty() const
{
    return ( m_notebook->get_n_pages() == 0 );
}


// タブの数
int Admin::get_tab_nums()
{
    return m_notebook->get_n_pages();
}


//
// 含まれているページのURLのリスト取得
//
std::vector<std::string> Admin::get_URLs()
{
    std::vector<std::string> urls;

    const int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            const SKELETON::View* view = dynamic_cast<SKELETON::View*>( m_notebook->get_nth_page( i ) );
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
    const int pages = m_notebook->get_n_pages();
    if( pages ){
        for( int i = 0; i < pages; ++i ){
            view = dynamic_cast< SKELETON::View* >( m_notebook->get_nth_page( i ) );
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
                         const std::string& arg5, const std::string& arg6,
                         const std::string& arg7, const std::string& arg8 )
{
    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;
    command_arg.arg2 = arg2;
    command_arg.arg3 = arg3;
    command_arg.arg4 = arg4;
    command_arg.arg5 = arg5;
    command_arg.arg6 = arg6;
    command_arg.arg7 = arg7;
    command_arg.arg8 = arg8;

    set_command_impl( false, command_arg );
}


//
// コマンド受付(即実行)
//
void Admin::set_command_immediately( const std::string& command, const std::string& url,
                                     const std::string& arg1, const std::string& arg2,
                                     const std::string& arg3, const std::string& arg4,
                                     const std::string& arg5, const std::string& arg6,
                                     const std::string& arg7, const std::string& arg8 )
{
    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;
    command_arg.arg2 = arg2;
    command_arg.arg3 = arg3;
    command_arg.arg4 = arg4;
    command_arg.arg5 = arg5;
    command_arg.arg6 = arg6;
    command_arg.arg7 = arg7;
    command_arg.arg8 = arg8;

    set_command_impl( true, command_arg );
}



//
// コマンド受付
//
// immediately = false の場合はすぐにコマンドを実行しないで一旦Dispatcherで
// メインスレッドにコマンドを渡してからメインスレッドで実行する。通常は
// immediately = false で呼び出して、緊急にコマンドを実行させたい場合は
// immediately = true とすること。
//
void Admin::set_command_impl( const bool immediately, const COMMAND_ARGS& command_arg )
{
#ifdef _DEBUG
    std::cout << "Admin::set_command : immediately = " << immediately
              <<  " command = " << command_arg.command
              << " url = " << command_arg.url << std::endl
              << command_arg.arg1 << " " << command_arg.arg2 << std::endl
              << command_arg.arg3 << " " << command_arg.arg4 << std::endl
              << command_arg.arg5 << " " << command_arg.arg6 << std::endl
              << command_arg.arg7 << " " << command_arg.arg8 << std::endl;
#endif

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
    else if( command.command == "reload_view" ){
        reload_view( command.url );
    }
    else if( command.command == "tab_left" ){
        tab_left( TabMove::next );
    }
    else if( command.command == "tab_right" ){
        tab_right( TabMove::next );
    }
    else if( command.command == "tab_left_updated" ){
        tab_left( TabMove::updated );
    }
    else if( command.command == "tab_right_updated" ){
        tab_right( TabMove::updated );
    }
    else if( command.command == "tab_left_updatable" ){
        tab_left( TabMove::updatable );
    }
    else if( command.command == "tab_right_updatable" ){
        tab_right( TabMove::updatable );
    }
    else if( command.command == "tab_num" ){
        tab_num( command.arg1 );
    }
    else if( command.command == "tab_head" ){
        tab_head();
    }
    else if( command.command == "tab_tail" ){
        tab_tail();
    }

    else if( command.command == "set_tab_operating" ){

#ifdef _DEBUG
        std::cout << command.command << " " << m_url << " " << command.arg1 << std::endl;
#endif

        if( command.arg1 == "true" ) SESSION::set_tab_operating( m_url, true );
        else{

            SESSION::set_tab_operating( m_url, false );

            if( ! empty() ){

                // 現在開いているタブへの switch 処理
                const bool focus_tmp = m_focus;
                m_focus = true;
                slot_switch_page( nullptr,  m_notebook->get_current_page() );
                m_focus = focus_tmp;
            }
        }

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

    // ビューを閉じる
    else if( command.command == "close_view" ){
        if( command.arg1 == "closeall" ) close_all_view( command.url );   // command.url を含むタブを全て閉じる
        else if( command.arg1 == "closealltabs" ) slot_close_all_tabs();  // 全て閉じる
        else if( command.arg1 == "closeother" ) close_other_views( command.url );
        else close_view( command.url );
    }
    else if( command.command == "close_currentview" ){
        close_current_view();
    }

    // 最後に閉じたタブを復元する
    else if( command.command == "restore_lasttab" ){
        restore_lasttab();
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
    else if( command.command == "relayout_all" ){
        const bool completely = ( command.arg1 == "completely" );
        relayout_all( completely );
    }

    // タイトル表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_title" ){

        const bool force = ( command.arg2 == "force" );
        set_title( command.url, command.arg1, force );
    }

    // ステータス表示
    // アクティブなviewから依頼が来たらコアに渡す
    else if( command.command == "set_status" ){

        const bool force = ( command.arg2 == "force" );
        set_status( command.url, command.arg1, force );
    }

    // ステータスの色を変える
    else if( command.command == "set_status_color" ){

        const bool force = ( command.arg2 == "force" );
        set_status_color( command.url, command.arg1, force );
    }

    // マウスジェスチャ
    else if( command.command  == "set_mginfo" ){
        if( m_win ) m_win->set_mginfo( command.arg1 );
    }

    // 全タブオートリロード
    else if( command.command == "reload_all_tabs" ){
        reload_all_tabs();
    }

    // 全タブ更新チェックして開く
    else if( command.command == "check_update_reload_all_tabs" ){
        check_update_all_tabs( true );
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

    // ツールバー表示/非表示切り替え
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

    // ツールバーボタンのアイコンを再読み込み
    else if( command.command == "reload_ui_icon" ){
        reload_ui_icon();
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
    else if( command.command == "show_preferences" ){
        SKELETON::View* view = get_view( command.url );
        if( view ) view->show_preference();
    }

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

    // ポップアップを隠す(インスタンスは削除しない)
    else if( command.command == "hide_popup" ) hide_popup();

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

    SESSION::set_tab_operating( m_url, true );

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
        if( ! command_list.arg2.empty() ) command_arg.arg3 += " " + command_list.arg2;

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

    set_command( "set_tab_operating", "", "false" );

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

    const int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url().rfind( url_old, 0 ) == 0 ){

                std::list< std::string >::iterator it
                    = std::find( m_list_switchhistory.begin(), m_list_switchhistory.end(), view->get_url() );
                view->update_url( url_old, url_new );
                if( it != m_list_switchhistory.end() ) (*it) = view->get_url();
            }
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

    const int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            SKELETON::View* view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url().rfind( url, 0 ) == 0 ) view->update_boardname();
        }

        redraw_toolbar();
    }
}


//
// URLやステータスを更新
//
void Admin::update_status( const View* view, const bool force )
{
    if( view ){
        set_title( view->get_url(), view->get_title(), force );
        set_url( view->get_url(), view->url_for_copy(), force );
        set_status( view->get_url(), view->get_status(), force );
        set_status_color( view->get_url(), view->get_color(), force );
    }
}



//
// ビューを開く
//
// command.arg1: 開く位置
//
// "false" ならアクティブなタブを置き換える ( デフォルト )
// "true" なら一番右側に新しいタブを開く
// "right" ならアクティブなタブの右に開く
// "newtab" "opentab" なら設定によってアクティブなタブの右に開くか一番右側に開くか切り替える
// "left" ならアクティブなタブの左に開く
// "page数字" なら指定した位置にタブで開く
// "replace" なら arg3 に指定したタブがあれば置き換え、なければ "newtab" で動作する
//
// command.arg2: "true" なら既に command.url を開いているかチェックしない
//
// command.arg3: モード  ( 複数指定する場合は空白で空ける )
//
// "auto"なら表示されていればリロードせずに切替える
//     されていなければarg1で指定した場所に新しいタブで開いてロード
//     スレ番号ジャンプなどで使用する
// "noswitch"ならタブを切り替えない(連続して開くときに使用)
// "lock" なら開いてからロックする
// "offline" なら オフラインで開く
// "reget" なら読み込み時にキャッシュ等を消してから再読み込みする
// "boardnext" など、 "replace" で置き換えるタブの種類を指定する
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
        if( command.arg3.find( "reget" ) != std::string::npos ) mode |= OPEN_MODE_REGET;
    }

    if( current_view ) current_view->focus_out();

    // urlを既に開いていたら表示してリロード
    if( ! nocheck_opened ){

        view = get_view( command_to_url( command ) );
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

            // ロード時にキャッシュを削除してからスレを再読み込みする
            if( mode & OPEN_MODE_REGET ) view->set_reget( true );

            view->show_view();
            SESSION::set_online( online );

            return;
        }
    }

    // 開く位置の基準を、アクティブなタブに仮定
    int page = m_notebook->get_current_page();

    std::string open_method = command.arg1;
    if( open_method.empty() ){
        open_method = "false";
    }

    // 置き替えるページを探す
    if( open_method == "replace" ){
        // 該当するタブが見つからない場合のために、新しいタブで開くモードに仮設定
        open_method = "newtab";

        // タブの種類 (command.arg3) に該当するタブを探す
        int find_page = find_view( command.arg3 );
#ifdef _DEBUG
        std::cout << "replace mode = " << command.arg3
                << " page = " << page << " find = " << find_page << std::endl;
#endif
        if( find_page >= 0 ){
            SKELETON::View* found_view = dynamic_cast< View* >( m_notebook->get_nth_page( page ) );
            if( found_view ){
                // 指定したタブを置き換えるモードに設定
                open_method = "false";

                // 開く位置の基準を、見つかったタブに設定
                page = find_page;
                current_view = found_view;
            }
        }
    }

    view = create_view( command );
    if( !view ) return;

    const bool open_tab = (  page == -1
                             || open_method != "false"
                             || ( mode & OPEN_MODE_AUTO ) // オートモードの時もタブで開く
                             || is_locked( page )
        );

    // ツールバー表示
    if( page == -1 ) show_toolbar();

    // タブで表示
    if( open_tab ){

        // 一番右側に新しいタブを開くに仮定
        int openpage = -1;

        // 現在のページの右に表示
        if( open_method == "right" ) openpage = page +1;

        // 設定によって切り替える
        else if( open_method == "newtab" ){
            switch( CONFIG::get_newtab_pos() ){
                case 1:  openpage = page +1; break;
                case 2:  openpage = page; break;
            }
        }
        else if( open_method == "opentab" ){
            switch( CONFIG::get_opentab_pos() ){
                case 1:  openpage = page +1; break;
                case 2:  openpage = page; break;
            }
        }

        // 現在のページの左に表示
        else if( open_method == "left" ) openpage = page;

        // 指定した位置に表示
        else if( open_method.rfind( "page", 0 ) == 0 ) openpage = atoi( open_method.c_str() + 4 );

        // ロックされていたら右に表示
        else if( open_method == "false" && page != -1 && is_locked( page ) ) openpage = page +1;

#ifdef _DEBUG
        std::cout << "append openpage = " << openpage << " / page = " << page << std::endl;
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

        // タブを入れ替えたときに隣のタブの再描画を防ぐ
        set_command_immediately( "set_tab_operating", "", "true" );

        // タブ入れ替え
        m_notebook->insert_page( command.url, *view, page );

        m_notebook->remove_page( page + 1, false );

        if( current_view ){

            const std::string url_current = current_view->get_url();
            delete current_view;

            remove_switchhistory( url_current );

            if( m_use_viewhistory ){

                // Aを開く -> B をタブで開く -> A を閉じる -> A を開いて B と置き換える(*) -> B をタブで開く
                // とすると、History_Manager::get_viewhistory()のキャッシュが誤作動して
                // ビューの戻る進むが変になるので、(*)の所で進む戻るの履歴を引き継ぐのをキャンセルする
                if( ! m_last_closed_url.empty() && view->get_url() == m_last_closed_url ){
                    HISTORY::get_history_manager()->delete_viewhistory( m_last_closed_url );
                    m_last_closed_url = std::string();
                }

                HISTORY::get_history_manager()->append_viewhistory( url_current, view->get_url() );    
            }

        }

#ifdef _DEBUG
        std::cout << "replace done\n";
#endif

        set_command( "set_tab_operating", "", "false" );
    }

    m_notebook->show_all();
    view->show();

    // オフラインで開く
    if( mode & OPEN_MODE_OFFLINE ) SESSION::set_online( false );

    // ロード時にキャッシュを削除してからスレを再読み込みする
    if( mode & OPEN_MODE_REGET ) view->set_reget( true );

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


/**
 * @brief ツールバーボタンのアイコンを再読み込み
 */
void Admin::reload_ui_icon()
{
    m_notebook->reload_ui_icon();
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
// 指定したURLのビューを再読み込み
//
void Admin::reload_view( const std::string& url )
{
    SKELETON::View* view = get_view( url );
    if( view ) view->reload();
}


/** @brief タブ左移動
 *
 * @param[in] mode タブ移動の種類
 */
void Admin::tab_left( const TabMove mode )
{
    const int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    int page = m_notebook->get_current_page();
    if( page == -1 ) return;

#ifdef _DEBUG
    std::cout << "Admin::tab_left mode << " << static_cast<int>( mode ) << " page = " << page;
#endif

    for( int i = 0; i < pages ; ++i ){

        --page;
        if( page < 0 ) page = pages -1;
        if( mode == TabMove::next ) break;

        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );
        if( ! view ) return;

        // アイコンが表示されているタブを見つける
        const char* icon_name = "";
        if( mode == TabMove::updated ) icon_name = "updated";
        else if( mode == TabMove::updatable ) icon_name = "update";

        if( get_notebook()->get_tabicon( page ) == view->get_icon( icon_name ) ) break;
    }

#ifdef _DEBUG
    std::cout << " -> " << page << std::endl;
#endif

    if( page != m_notebook->get_current_page() ) set_current_page( page );
}



/** @brief タブ右移動
 *
 * @param[in] mode タブ移動の種類
 */
void Admin::tab_right( const TabMove mode )
{
    const int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    int page = m_notebook->get_current_page();
    if( page == -1 ) return;

#ifdef _DEBUG
    std::cout << "Admin::tab_right mode << " << static_cast<int>( mode ) << " page = " << page;
#endif

    for( int i = 0; i < pages; ++i ){

        ++page;
        if( page >= pages ) page = 0;
        if( mode == TabMove::next ) break;

        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );
        if( ! view ) return;

        // アイコンが表示されているタブを見つける
        const char* icon_name = "";
        if( mode == TabMove::updated ) icon_name = "updated";
        else if( mode == TabMove::updatable ) icon_name = "update";

        if( get_notebook()->get_tabicon( page ) == view->get_icon( icon_name ) ) break;
    }

#ifdef _DEBUG
    std::cout << " -> " << page << std::endl;
#endif

    if( page != m_notebook->get_current_page() ) set_current_page( page );
}


//
// タブ位置(1-9)で移動
//
void Admin::tab_num( const std::string& str_num )
{
    if( str_num.empty() ) return;

    const int num = strtol( str_num.c_str(), nullptr, 10 );

    // 最後のタブに移動
    if( num == -1 ) {
        // GtkNotebook の API はマイナスの値で最後のタブを指定する
        set_current_page( -1 );
        return;
    }

    // Firefoxの動作に合わせた
    // 0 → 無視
    // 8以下で存在する数より多い → 無視
    // 9(存在しない) → 最後のタブ
    if ( num < 1 || num > 9 ||
         ( num < 9 && num > get_tab_nums() ) ) return;

    // 9は存在しなくてもそのまま渡してしまう
    set_current_page( num - 1 );
}


//
// タブ先頭移動
//
void Admin::tab_head()
{
    const int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    set_current_page( 0 );
}


//
// フォーカスしてタブ先頭移動
//
void Admin::tab_head_focus()
{
    if( ! m_focus ) switch_admin();
    tab_head();
}


//
// タブ最後に移動
//
void Admin::tab_tail()
{
    const int pages = m_notebook->get_n_pages();
    if( pages == 1 ) return;

    set_current_page( pages-1 );
}


//
// タブ最後に移動
//
void Admin::tab_tail_focus()
{
    if( ! m_focus ) switch_admin();
    tab_tail();
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

    for( SKELETON::View* view : list_view ) {
        if( view && view == current_view ) view->redraw_view();
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

    remove_switchhistory( view->get_url() );

    if( page == current_page ){

        // その前に開いていたビューに切り替える
        const std::string hist_url = get_valid_switchhistory();
        if( ! hist_url.empty() ) switch_view( hist_url );

        // もし現在表示中のビューを消すときは予めひとつ右のビューにスイッチしておく
        // そうしないと左のビューを一度表示してしまうので遅くなる
        else{
            const SKELETON::View* newview = dynamic_cast<View*>( m_notebook->get_nth_page( page + 1 ) );
            if( newview ) switch_view( newview->get_url() );
        }
    }

    m_notebook->remove_page( page, true );

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

    for( SKELETON::View* view : list_view ) {
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

    for( SKELETON::View* view : list_view ) {
        close_view( view );
    }
}


//
// url 以外のビューを全て閉じる
//
void Admin::close_other_views( const std::string& url )
{
    const int pages = m_notebook->get_n_pages();
    for( int i = 0; i < pages; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view && view->get_url() != url ) set_command( "close_view", view->get_url() );
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

    const SKELETON::View* view = get_current_view();
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
    for( SKELETON::View* view : list_view ) {
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
    if( m_win ) m_win->set_title( "JDim - " + title );
    else{

        const SKELETON::View* view = get_current_view();
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

        const SKELETON::View* view = get_current_view();
        if( view ){

            // アクティブなviewからコマンドが来たら表示する
            if( force || ( m_focus && view->get_url() == url ) ){
                CORE::core_set_command( "set_url", url_show );

                // ツリービューのURLを選択
                if( CONFIG::get_select_item_sync() != 0 ){
                    CORE::core_set_command( "select_sidebar_item", url );
                    CORE::core_set_command( "select_board_item", url );
                }
            }
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
        
        const SKELETON::View* view = get_current_view();
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
// ステータスの色を変える
//
void Admin::set_status_color( const std::string& url, const std::string& color, const bool force )
{
    if( m_win ) m_win->set_status_color( color );
    else{

        const SKELETON::View* view = get_current_view();
        if( view ){
            
            // アクティブなviewからコマンドが来たら色を変える
            if( force || ( m_focus && view->get_url() == url ) ){
                CORE::core_set_command( "set_status_color", url, color );
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

        const std::string label = MISC::to_plain( str_label );
        m_notebook->set_tab_fulltext( label, m_notebook->page_num( *view ) );

        // View履歴のタイトルも更新
        if( m_use_viewhistory ){
            HISTORY::get_history_manager()->replace_current_title_viewhistory( view->get_url(), label );
        }
    }
}



//
// 再レイアウト実行
//
void Admin::relayout_all( const bool completely )
{
    std::list< SKELETON::View* > list_view = get_list_view();
    for( SKELETON::View* view : list_view ) {
        if( view ) view->relayout( completely );
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

        // 古い
        else if( view->is_old() ) iconname = "old";

        // 更新チェックして更新可能 ( 更新はしていない )
        else if( view->is_check_update() ) iconname = "update";

        // 更新済み ( HTTP_OK 又は HTTP_PARTIAL_CONTENT )
        else if( view->is_updated() ){

            // タブがアクティブの時は通常アイコンを表示
            if( get_notebook()->page_num( *view ) == get_notebook()->get_current_page() ) iconname = "default";

            else iconname = "updated";
        }

#ifdef _DEBUG
        std::cout << "name = " << iconname << std::endl;
#endif
        const int id = view->get_icon( iconname );
        get_notebook()->set_tabicon( iconname, get_notebook()->page_num( *view ), id );

        if( m_model_tabswitch ) m_model_tabswitch->update_icons();
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


// ポップアップを隠す(インスタンスは削除しない)
void Admin::hide_popup()
{
    SKELETON::View* view = get_current_view();
    if( view ) view->set_command( "hide_popup" );
}


//
// ビュークラス取得
//
View* Admin::get_view( const std::string& url )
{
    SKELETON::View* view = get_current_view();
    if( view && view->get_url() == url ) return view;

    const int pages = m_notebook->get_n_pages();
    if( pages ){

        for( int i = 0; i < pages; ++i ){
            view = dynamic_cast< View* >( m_notebook->get_nth_page( i ) );
            if( view && view->get_url() == url ) return view;
        }
    }
    
    return nullptr;
}



//
// ビュークラスのリスト取得
//
// url を含むビューのリストを返す
//
std::list< View* > Admin::get_list_view( const std::string& url )
{
    std::list< View* > list_view;
    
    const int pages = m_notebook->get_n_pages();
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
    
    const int pages = m_notebook->get_n_pages();
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
    if( page == -1 ) return nullptr;
    SKELETON::View* view =  dynamic_cast< View* >( m_notebook->get_nth_page( page ) );

    return view;
}


/** @brief 配置用のウィジェットを取得する
 *
 * @param[in] id 紐づけのID
 * @return id に紐づけられたウィジェット、無ければ nullptr を返す
 */
Gtk::Widget* Admin::get_anchor_widget( const std::size_t id )
{
    if( m_vec_anchor_widget.size() <= id ) return nullptr;
    return m_vec_anchor_widget[id];
}


/** @brief 配置用のウィジェットを設定する
 *
 * @note Admin では anchor_widget の寿命を管理しない。
 * @param[in] id            紐づけのID
 * @param[in] anchor_widget IDに紐づける配置用ウィジェット
 */
void Admin::set_anchor_widget( const std::size_t id, Gtk::Widget* anchor_widget )
{
    if( m_vec_anchor_widget.size() <= id ) {
        m_vec_anchor_widget.resize( id + 1 );
    }
    m_vec_anchor_widget[id] = anchor_widget;
}


//
// 指定したページに表示切替え
//
void Admin::set_current_page( const int page )
{
    m_notebook->set_current_page( page );
}


//
// フォーカスしてから指定したページに表示切替え
//
void Admin::set_current_page_focus( const Glib::VariantBase& page )
{
    if( ! m_focus ) switch_admin();
    auto v = Glib::VariantBase::cast_dynamic<Glib::Variant<gint32>>( page );
    m_notebook->set_current_page( v.get() );
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
    const SKELETON::View* view = get_current_view();
    if( ! view ) return std::string();
    return view->get_url();
}


//
// notebookのタブのページが切り替わったら呼ばれるslot
//
void Admin::slot_switch_page( Gtk::Widget*, guint page )
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

    // タブ操作中
    if( SESSION::is_tab_operating( m_url ) ) return;

#ifdef _DEBUG
    std::cout << "Admin::slot_switch_page : " << m_url << " page = " << page << " focus = " << m_focus << std::endl;
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

        append_switchhistory( view->get_url() );

        CORE::core_set_command( "page_switched", m_url, view->get_url() );
    }
}


//
// タブをクリックした
//
void Admin::slot_tab_clicked( const int page )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_clicked " << page << std::endl;
#endif

    if( ! m_focus ) switch_admin();
}


// タブの上でホイールを回した
void Admin::slot_tab_scrolled( GdkEventScroll* event )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_scrolled\n";
#endif

    if( ! m_focus ) switch_admin();
}


//
// タブを閉じる
//
void Admin::slot_tab_close( const int page )
{
#ifdef _DEBUG
    std::cout << "Admin::slot_tab_close " << page << std::endl;
#endif

    // 閉じる
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );
    if( view ) close_view( view->get_url() );
}


//
// タブ再読み込み
//
void Admin::slot_tab_reload( const int page )
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
    if( ! m_model_popup ) return;

#ifdef _DEBUG
    std::cout << "Admin::slot_tab_menu " << page << std::endl;
#endif

    Glib::RefPtr<Gio::SimpleAction> act;
    m_clicked_page = -1; // メニューのactive状態を変えたときにslot関数が呼び出されるのをキャンセル

    auto get_action = [this]( const char* name ) {
        return Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( name ) );
    };

    // ロック
    act = get_action( "LockTab" );
    if( page >= 0 && act ){

        if( ! is_lockable( page ) ) act->set_enabled( false );
        else {
            act->set_enabled( true );

            auto state = Glib::Variant<bool>::create( is_locked( page ) );
            act->set_state( state );
        }
    }

    // 閉じる
    if( act = get_action( "Quit" ); act ) {
        act->set_enabled( ! is_locked( page ) );
    }

    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );

    // 進む、戻る
    if( view ){
        if( act = get_action( "PrevView" ); act ) {
            const bool enabled = HISTORY::get_history_manager()->can_back_viewhistory( view->get_url(), 1 );
            act->set_enabled( enabled );
        }
        if( act = get_action( "NextView" ); act ) {
            const bool enabled = HISTORY::get_history_manager()->can_forward_viewhistory( view->get_url(), 1 );
            act->set_enabled( enabled );
        }

        // 仮想板や抽出ビューなどはお気に入りに追加を選択不可にする
        if( act = get_action( "AppendFavorite" ); act ) {
            const std::string& url = view->get_url();

            // 仮想板 global.h を参照
            if( url.rfind( "jdview://", 0 ) == 0 ) {
                act->set_enabled( false );
            }
            // 抽出ビュー sign.h と ArticleAdmin::command_to_url() を参照
            // XXX: 判定に使うマクロ文字列がurl部分にマッチすると誤って選択不可になってしまう可能性がある
            else if( const auto i = url.find( '_' ); i != std::string::npos
                     && ( url.find( ARTICLE_SIGN, i ) != std::string::npos
                          || url.find( POSTLOG_SIGN, i ) != std::string::npos
                          || url.find( BOARD_SIGN KEYWORD_SIGN, i ) != std::string::npos
                          || url.find( TITLE_SIGN KEYWORD_SIGN, i ) != std::string::npos ) ) {
                act->set_enabled( false );
            }
            else {
                act->set_enabled( true );
            }
        }
    }

    m_clicked_page = page;

    if( m_model_tabswitch ){
        m_model_tabswitch->update_labels_and_icons();

        // 起動時間を短縮するためメニューを表示するタイミングでGio::Menuをバインドする
        if( ! m_tablabel_menu.get_attach_widget() ) {
            m_tablabel_menu.bind_model( m_model_popup, true );
            m_tablabel_menu.attach_to_widget( *m_notebook );
            m_tablabel_menu.signal_deactivate().connect( sigc::mem_fun( *this, &Admin::slot_popupmenu_deactivate ) );
        }

        // メニュー項目ラベルの更新は重いためタブ数が変わらないときは更新しない
        const int n_pages = m_notebook->get_n_pages();
        if( n_pages != m_prev_n_pages ) {
            // Gio::Menuを使うとメニューに追加された項目のラベルは変更できない
            // 動的なメニュー項目を実現するには一旦メニューから項目を取り除く必要がある
            m_model_popup->remove( 0 );
            auto label = Glib::ustring::compose( ITEM_NAME_GO " [ タブ数 %1 ](_M)", n_pages );
            m_item_sub->set_label( label );
            m_model_popup->prepend_item( m_item_sub );

            m_prev_n_pages = n_pages;
        }

        m_tablabel_menu.popup_at_pointer( nullptr ); // current event
    }
}


// タブ切り替えメニュー表示
void Admin::slot_show_tabswitchmenu()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_show_tabswitchmenu\n";
#endif

    if( ! m_model_tabswitch ) {
        m_model_tabswitch = SKELETON::TabSwitchMenu::create( m_notebook.get() );
    }

    m_model_tabswitch->update_labels_and_icons();

    // 起動時間を短縮するためメニューを表示するタイミングでGio::Menuをバインドする
    if( ! m_tabswitchmenu.get_attach_widget() ) {
        m_tabswitchmenu.bind_model( m_model_tabswitch, true );
        m_tabswitchmenu.attach_to_widget( *m_notebook );
        m_tabswitchmenu.signal_deactivate().connect( sigc::mem_fun( *this, &Admin::slot_popupmenu_deactivate ) );
    }

    // Specify the current event by nullptr.
    m_tabswitchmenu.popup_at_widget( &( m_notebook->get_tabswitch_button() ),
                                     Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_NORTH_EAST, nullptr );
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

    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
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

    set_command( "set_tab_operating", "", "true" );

    std::string url;
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) url = view->get_url();

    close_other_views( url );

    set_command( "set_tab_operating", "", "false" );
}



//
// 右クリックメニューの左を閉じる
//
void Admin::slot_close_left_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_left_tabs " << m_clicked_page << std::endl;
#endif

    set_command( "set_tab_operating", "", "true" );

    for( int i = 0; i < m_clicked_page; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }

    set_command( "set_tab_operating", "", "false" );
}


//
// 右クリックメニューの右を閉じる
//
void Admin::slot_close_right_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_right_tabs " << m_clicked_page << std::endl;
#endif

    set_command( "set_tab_operating", "", "true" );

    const int pages = m_notebook->get_n_pages();
    for( int i = m_clicked_page +1; i < pages; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }

    set_command( "set_tab_operating", "", "false" );
}




//
// 右クリックメニューの全てを閉じる
//
void Admin::slot_close_all_tabs()
{
#ifdef _DEBUG
    std::cout << "Admin::slot_close_all_tabs " << m_clicked_page << std::endl;
#endif

    set_command( "set_tab_operating", "", "true" );

    const int pages = m_notebook->get_n_pages();
    for( int i = 0; i < pages; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view ) set_command( "close_view", view->get_url() );
    }

    set_command( "set_tab_operating", "", "false" );
}


//
// 右クリックメニューの同じアイコンのタブを閉じる
//
void Admin::slot_close_same_icon_tabs()
{
    const int id = get_notebook()->get_tabicon( m_clicked_page );

#ifdef _DEBUG
    std::cout << "Admin::slot_close_same_icon_tabs page = " << m_clicked_page << " id = " << id << std::endl;
#endif

    set_command( "set_tab_operating", "", "true" );

    const int pages = m_notebook->get_n_pages();
    for( int i = 0; i < pages; ++i ){

        if( get_notebook()->get_tabicon( i ) == id ){

            const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
            if( view ) set_command( "close_view", view->get_url() );
        }
    }

    set_command( "set_tab_operating", "", "false" );
}


//
// 右クリックメニューの全てのタブの更新チェック
//
void Admin::slot_check_update_all_tabs()
{
    check_update_all_tabs( m_clicked_page, false );
}


//
// 右クリックメニューの全てのタブの更新チェックと再読み込み
//
void Admin::slot_check_update_reload_all_tabs()
{
    check_update_all_tabs( m_clicked_page, true );
}


//
// 開いているタブから全てのタブの更新チェックと再読み込み
//
void Admin::check_update_all_tabs( const bool open )
{
    check_update_all_tabs( m_notebook->get_current_page(), open );
}


//
// from_page から全てのタブを更新チェック
//
void Admin::check_update_all_tabs( const int from_page, const bool open )
{
#ifdef _DEBUG
    std::cout << "Admin::check_update_all_tabs from = " << from_page << std::endl;
#endif

    if( ! SESSION::is_online() ) return;

    const int pages = m_notebook->get_n_pages();

    // 開始タブから右側
    for( int i = from_page ; i < pages; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view && view->get_enable_autoreload() ) CORE::get_checkupdate_manager()->push_back( view->get_url(), open );
    }

    // 開始タブから左側
    for( int i = 0 ; i < from_page; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view && view->get_enable_autoreload() ) CORE::get_checkupdate_manager()->push_back( view->get_url(), open );
    }

    CORE::get_checkupdate_manager()->run();
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
    const int pages = m_notebook->get_n_pages();

    // クリックしたタブから右側
    for( int i = from_page ; i < pages; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
        if( view ){
            if( set_autoreload_mode( view->get_url(), AUTORELOAD_ONCE, waittime ) ) waittime += AUTORELOAD_MINSEC;
        }
    }

    // クリックしたタブから左側
    for( int i = 0 ; i < from_page; ++i ){
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
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
        const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( i ) );
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

    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) CORE::core_set_command( "open_url_browser", view->url_for_copy() );
}


//
// 右クリックメニューのURLコピー
//
void Admin::slot_copy_url()
{
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) MISC::CopyClipboard( view->url_for_copy() );
}



//
// 右クリックメニューのタイトルとURLコピー
//
void Admin::slot_copy_title_url()
{
    std::string str = m_notebook->get_tab_fulltext( m_clicked_page );
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( view ) str += "\n" + view->url_for_copy();
    
    MISC::CopyClipboard( str );
}


/**
 * @brief 右クリックメニューのお気に入りに追加
 */
void Admin::slot_append_favorite()
{
    if( m_clicked_page < 0 ) return;

    const SKELETON::View* view = dynamic_cast<SKELETON::View*>( get_notebook()->get_nth_page( m_clicked_page ) );
    if( ! view ) return;

    const std::string url = view->get_url();
    append_favorite_impl( url );
}


// ページがロックされているかリストで取得
std::vector<char> Admin::get_locked()
{
    std::vector<char> locked;
    
    const int pages = m_notebook->get_n_pages();
    if( pages ){
        for( int i = 0; i < pages; ++i ) locked.push_back( is_locked( i ) );
    }

    return locked;
}

// タブのロック/アンロック
bool Admin::is_lockable( const int page )
{
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );
    if( view ) return view->is_lockable();

    return false;
}

bool Admin::is_locked( const int page )
{
    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( page ) );
    if( view ) return view->is_locked();

    return false;
}

bool Admin::is_locked( const std::string& url )
{
    const SKELETON::View* view = get_view( url );
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

// urlで指定されるタブが存在するか
bool Admin::exist_tab( const std::string& url )
{
    const SKELETON::View* view = get_view( url );
    if( view ) return true;

    return false;
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

    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( ! view ) return;
    
    set_current_page( m_clicked_page );
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

    const SKELETON::View* view = dynamic_cast<View*>( m_notebook->get_nth_page( m_clicked_page ) );
    if( ! view ) return;

    set_current_page( m_clicked_page );
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

    const SKELETON::View* view = get_view( url );
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
                mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Cancel" ), Gtk::RESPONSE_CANCEL );

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

            // Admin::open_view() 中の append_viewhistory() を実行しないでここで View履歴の現在位置を変更する
            // Admin::Open_view()も参照すること
            const bool use_history = get_use_viewhistory();
            set_use_viewhistory( false );

            if( use_history ){
                if( back ) HISTORY::get_history_manager()->back_viewhistory( url, count, true );
                else HISTORY::get_history_manager()->forward_viewhistory( url, count, true );
            }

            const COMMAND_ARGS command_arg = url_to_openarg( historyitem->url, false, false );
            open_view( command_arg );

            set_use_viewhistory( use_history );

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
    for( const SKELETON::View* view : list_view ) {
        if( view ){
            HISTORY::get_history_manager()->delete_viewhistory( view->get_url() );
            HISTORY::get_history_manager()->create_viewhistory( view->get_url() );

            const int page = m_notebook->page_num( *view );
            const std::string& str_label = m_notebook->get_tab_fulltext( page );
            HISTORY::get_history_manager()->replace_current_title_viewhistory( view->get_url(), str_label );
        }
    }

    if( ! m_last_closed_url.empty() ) HISTORY::get_history_manager()->delete_viewhistory( m_last_closed_url );
    m_last_closed_url = std::string();

    update_toolbar_button();
}

//
// タブの切り替え履歴を更新
//
void Admin::append_switchhistory( const std::string& url )
{
    if( ! m_use_switchhistory ) return;

    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "Admin::append_switchhistory url = " << url << std::endl;
#endif

    m_list_switchhistory.remove( url );
    m_list_switchhistory.push_back( url );

#ifdef _DEBUG
    for( const std::string& hist_url : m_list_switchhistory ) {
        std::cout << hist_url << std::endl;
    }
#endif
}


void Admin::remove_switchhistory( const std::string& url )
{
    if( ! m_use_switchhistory ) return;

    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "Admin::remove_switchhistory url = " << url << std::endl;
#endif

    m_list_switchhistory.remove( url );

#ifdef _DEBUG
    for( const std::string& hist_url : m_list_switchhistory ) {
        std::cout << hist_url << std::endl;
    }
#endif
}


//
// タブの切り替え履歴から、有効な最後のURLを返す
//
// 表示されているビューに存在しないURLは履歴から削除して、表示されているビューのURLを返す。
// 見つからないときは空文字列を返す。
//
// NOTE:
//   タブの切り替え履歴にあるビューは、表示されているビューのうちのどれかであるはずだが、
//   検索ビューなどでURLが変更になる場合があり、 ( View::set_url()で変更できてしまう。 )
//   タブの切り替え履歴にあるURLと、表示されているビューのURLが不一致となることがある。
//   また、その不一致なタブの切り替え履歴を、セッション情報 ( article_switchhistoryなど ) に
//   保存してしまっていたため、ここで不一致な履歴の削除を行うことで、履歴を修復する。
//
std::string Admin::get_valid_switchhistory()
{
    if( ! m_use_switchhistory ) return std::string();

    while( m_list_switchhistory.size() > 0 ){

        // タブの切り替え履歴から、最後のURLを取り出す
        const std::string url = m_list_switchhistory.back();

        // 表示されているビューであれば、そのURLを返す
        const SKELETON::View* view = get_view( url );
        if( view ) return url;

        // 表示されていないので、最後のURLを削除する
        m_list_switchhistory.pop_back();
#ifdef _DEBUG
        std::cout << "Admin::get_valid_switchhistory remove = " << url << std::endl;
#endif
    }
    return std::string();
}
