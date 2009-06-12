// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "bbslistviewbase.h"
#include "bbslistadmin.h"
#include "selectdialog.h"
#include "editlistwin.h"
#include "addetcdialog.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "icons/iconmanager.h"

#include "xml/tools.h"

#include "config/globalconf.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "cache.h"
#include "command.h"
#include "global.h"
#include "type.h"
#include "httpcode.h"
#include "sharedbuffer.h"
#include "viewfactory.h"
#include "prefdiagfactory.h"
#include "colorid.h"
#include "fontid.h"
#include "updatemanager.h"
#include "session.h"
#include "compmanager.h"
#include "dndmanager.h"

#include <sstream>


// row -> path
#define GET_PATH( row ) m_treestore->get_path( row )


// ポップアップメニュー表示
#define SHOW_POPUPMENU(slot) do{\
std::string url = path2url( m_path_selected ); \
if( ! m_path_selected.empty() && url.empty() ) url = "dummy_url"; \
show_popupmenu( url, slot ); \
}while(0)


#define POPUPMENU_BOARD1 \
    "<menuitem action='OpenTab'/>" \
    "<menuitem action='OpenBrowser'/>" \
    "<separator/>" \
    "<menuitem action='CopyURL'/>" \
    "<menuitem action='CopyTitleURL'/>" \
    "<separator/>" \
    "<menuitem action='AppendFavorite'/>" \
    "<separator/>"

#define POPUPMENU_BOARD2 \
    "<menuitem action='SearchCacheBoard'/>" \
    "<separator/>" \
    "<menuitem action='ImportDat'/>" \
    "<separator/>" \
    "<menuitem action='PreferenceBoard'/>" \
    "</popup>" \

#define POPUPMENU_SELECT \
    "<menuitem action='CopyURL'/>" \
    "<menuitem action='CopyTitleURL'/>" \
    "<separator/>" \
    "<menuitem action='Rename'/>" \
    "<menuitem action='NewDir'/>" \
    "<menuitem action='NewCom'/>" \
    "<separator/>" \
    "<menu action='Delete_Menu'>" \
    "<menuitem action='Delete'/>" \
    "</menu>" \
    "<separator/>" \
    "<menuitem action='SearchCacheBoard'/>" \
    "<separator/>" \
    "<menuitem action='ImportDat'/>" \
    "<separator/>" \
    "<menuitem action='PreferenceArticle'/>" \
    "<menuitem action='PreferenceBoard'/>" \
    "<menuitem action='PreferenceImage'/>"

using namespace BBSLIST;

BBSListViewBase::BBSListViewBase( const std::string& url,const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_treeview( url, DNDTARGET_FAVORITE, m_columns, true, CONFIG::get_fontname( FONT_BBS ), COLOR_CHAR_BBS, COLOR_BACK_BBS, COLOR_BACK_BBS_EVEN ),
      m_ready_tree( false ),
      m_jump_y( -1 ),
      m_search_invert( false ),
      m_open_only_onedir( false ),
      m_cancel_expand( false ),
      m_expanding( 0 ),
      m_editlistwin( NULL )
{
    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

    pack_start( m_scrwin );
    show_all_children();    

    m_treestore = Gtk::TreeStore::create( m_columns ); 

#if GTKMMVER <= 260
    // gtkmm26以下にはunset_model()が無いのでここでset_model()しておく
    // それ以上は m_treeview.xml2tree() でセットする
    m_treeview.set_treestore( m_treestore );
#endif

    // Gtk::TreeStoreでset_fixed_height_mode()を使うとexpandしたときに
    // スクロールバーが誤動作するので使わないこと
/*
#if GTK_CHECK_VERSION(2,6,0)
    // セルを固定の高さにする
    // append_column する前に columnに対して set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) すること
    m_treeview.set_fixed_height_mode( true );

#ifdef _DEBUG
    std::cout << "BBSListViewBase::BBSListViewBase : m_treeview.set_fixed_height_mode\n";
#endif

#endif
*/
    // 共有UNDOバッファをセット
    m_treeview.set_undo_buffer( BBSLIST::get_undo_buffer_favorite() );

    // 列の登録
    m_treeview.create_column( CONFIG::get_tree_ypad() );
    m_treeview.set_column_for_height( 0 );

    // treeviewのシグナルにコネクト
    m_treeview.signal_row_expanded().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_row_exp ) );
    m_treeview.signal_row_collapsed().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_row_col ) );        

    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_button_release ) );
    m_treeview.sig_motion_notify().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_motion_notify ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_release ) );
    m_treeview.sig_scroll_event().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_scroll_event ) );
    m_treeview.sig_dropped_from_other().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_dropped_from_other ) );

    ///////////////////
    
    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "OpenTab", "タブで開く(_T)"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_tab ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く(_W)"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "お気に入りに追加(_F)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_append_favorite ) );
    action_group()->add( Gtk::Action::create( "NewDir", "新規ディレクトリ(_N)"), sigc::mem_fun( *this, &BBSListViewBase::slot_newdir ) );
    action_group()->add( Gtk::Action::create( "NewCom", "コメント挿入(_I)"), sigc::mem_fun( *this, &BBSListViewBase::slot_newcomment ) );
    action_group()->add( Gtk::Action::create( "NewEtc", "外部板追加(_E)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_newetcboard ) );
    action_group()->add( Gtk::Action::create( "MoveEtc", "編集(_E)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_moveetcboard ) );
    action_group()->add( Gtk::Action::create( "Rename", "名前変更(_R)"), sigc::mem_fun( *this, &BBSListViewBase::slot_rename ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "Delete", "お気に入りから削除する(_D)"), sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    action_group()->add( Gtk::Action::create( "Delete_etc", "外部板を削除する(_D)"), sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    action_group()->add( Gtk::Action::create( "OpenRows", "選択した行を開く(_O)"), sigc::mem_fun( *this, &BBSListViewBase::open_selected_rows ) );

    action_group()->add( Gtk::Action::create( "CheckUpdateRows", "更新チェックのみ(_H)"), sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_selected_rows ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateOpenRows", "更新された行をタブで開く(_E)"),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_open_selected_rows ) );

    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー(_U)"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", "タイトルとURLをコピー(_L)"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "SelectDir", "全て選択(_A)"), sigc::mem_fun( *this, &BBSListViewBase::slot_select_all_dir ) );

    action_group()->add( Gtk::Action::create( "CheckUpdate_Menu", "更新チェック(_M)" ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateDir", "更新チェックのみ(_R)"), sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_dir ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateOpenDir", "更新された行をタブで開く(_A)"),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_open_dir ) );
    action_group()->add( Gtk::Action::create( "CancelCheckUpdate", "キャンセル(_C)" ),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_cancel_check_update ) );

    action_group()->add( Gtk::Action::create( "SearchCacheBoard", "キャッシュ内ログ検索(_S)"), sigc::mem_fun( *this, &BBSListViewBase::slot_search_cache_board ) );
    action_group()->add( Gtk::Action::create( "ImportDat", "datのインポート(_I)"), sigc::mem_fun( *this, &BBSListViewBase::slot_import_dat ) );

    action_group()->add( Gtk::Action::create( "PreferenceArticle", "スレのプロパティ(_P)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_article ) );
    action_group()->add( Gtk::Action::create( "PreferenceBoard", "板のプロパティ(_B)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_board ) );
    action_group()->add( Gtk::Action::create( "PreferenceImage", "画像のプロパティ(_M)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_image ) );


    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

    // ポップアップメニューのレイアウト
    Glib::ustring str_ui = 

    "<ui>"

    // 板一覧 + 板
    "<popup name='popup_menu'>"
    POPUPMENU_BOARD1
    POPUPMENU_BOARD2

    // 板一覧 + 外部板
    "<popup name='popup_menu_etc'>"
    POPUPMENU_BOARD1
    "<menuitem action='MoveEtc'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete_etc'/>"
    "</menu>"
    "<separator/>"
    POPUPMENU_BOARD2

    // 板一覧 + 複数選択
    "<popup name='popup_menu_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"
    "</popup>"

    // 板一覧 + 複数選択 + 外部板含む
    "<popup name='popup_menu_mul_etc'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"

    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete_etc'/>"
    "</menu>"

    "</popup>"

    // 板一覧 + ディレクトリ
    "<popup name='popup_menu_dir'>"
    "<menuitem action='SelectDir'/>"
    "</popup>"

    // 板一覧 + 外部板ディレクトリ
    "<popup name='popup_menu_etcdir'>"
    "<menuitem action='SelectDir'/>"
    "<separator/>"
    "<menuitem action='NewEtc'/>"
    "</popup>"


    /////////////////////////////////////////


    // お気に入り
    "<popup name='popup_menu_favorite'>"
    "<menuitem action='OpenTab'/>"
    "<menuitem action='OpenBrowser'/>"
    "<separator/>"
    POPUPMENU_SELECT
    "</popup>"

    // お気に入り + 複数選択
    "<popup name='popup_menu_favorite_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menu action='CheckUpdate_Menu'>"
    "<menuitem action='CheckUpdateRows'/>"
    "<menuitem action='CheckUpdateOpenRows'/>"
    "<separator/>"
    "<menuitem action='CancelCheckUpdate'/>"
    "</menu>"
    "<separator/>"

    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    // お気に入り + 何もないところをクリック
    "<popup name='popup_menu_favorite_space'>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "</popup>"

    // お気に入り + ディレクトリ
    "<popup name='popup_menu_favorite_dir'>"
    "<menu action='CheckUpdate_Menu'>"
    "<menuitem action='CheckUpdateDir'/>"
    "<menuitem action='CheckUpdateOpenDir'/>"
    "<separator/>"
    "<menuitem action='CancelCheckUpdate'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='SelectDir'/>"
    "<separator/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    // お気に入り + コメント
    "<popup name='popup_menu_favorite_com'>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"


    //////////////////////////////////////


    // 選択(selectlistview)
    "<popup name='popup_menu_select'>"
    POPUPMENU_SELECT
    "</popup>"

    "</ui>";

    ui_manager()->add_ui_from_string( str_ui );

    // ポップアップメニューにキーアクセレータを表示
    Gtk::Menu* popupmenu = id2popupmenu(  "/popup_menu" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_etc" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_mul" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_mul_etc" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_dir" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_favorite" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_favorite_mul" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_favorite_space" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_favorite_dir" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_favorite_com" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = id2popupmenu(  "/popup_menu_select" );
    CONTROL::set_menu_motion( popupmenu );

    // マウスジェスチャ可
    set_enable_mg( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_BBSLIST );
}


BBSListViewBase::~BBSListViewBase()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::~BBSListViewBase : " << get_url() << std::endl;
#endif

    if( m_editlistwin ) delete m_editlistwin;
    m_editlistwin = NULL;
}


SKELETON::Admin* BBSListViewBase::get_admin()
{
    return BBSLIST::get_admin();
}


//
// treeviewのD&Dによる編集を可能にする
//
void BBSListViewBase::set_editable( const bool editable )
{
    get_treeview().set_editable_view( editable );
}


//
// 親ウィンドウをセット
//
void BBSListViewBase::set_parent_win( Gtk::Window* parent_win )
{
    SKELETON::View::set_parent_win( parent_win );
    get_treeview().set_parent_win( parent_win );
}


//
// コマンド
//
const bool BBSListViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    if( command == "append_item" ) append_item();
    else if( command == "remove_item" ) remove_item( arg1 );
    else if( command == "edit_tree" ) edit_tree();
    else if( command == "save_xml" ) save_xml( false );
    else if( command == "toggle_articleicon" ) toggle_articleicon( arg1 );
    else if( command == "toggle_boardicon" ) toggle_boardicon( arg1 );
    else if( command == "replace_thread" ) replace_thread( arg1, arg2 );

    else if( command == "check_update_root" ) check_update_root( false );
    else if( command == "check_update_open_root" ) check_update_root( true );
    else if( command == "cancel_check_update" ) slot_cancel_check_update();

    return true;
}


//
// shutdown( SIGHUP )用
//
void BBSListViewBase::shutdown()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::shutdown\n";
#endif

    save_xml( true );
}


//
// クロック入力
//
void BBSListViewBase::clock_in()
{
    View::clock_in();

    m_treeview.clock_in();
    if( m_editlistwin ) m_editlistwin->clock_in();

    // スクロールバー移動
    // 初期化直後など、まだスクロールバーが表示されてない時があるので表示されるまでジャンプしない
    if( m_jump_y != -1 ){

        Gtk::Adjustment* adjust = m_treeview.get_vadjustment();
        if( adjust && adjust->get_upper() > m_jump_y ){

#ifdef _DEBUG    
            std::cout << "BBSListViewBase::clock_in jump to = " << m_jump_y << " upper = " << (int)adjust->get_upper() << std::endl;
#endif

            // 何故か先頭にジャンプ出来ないので 1 にジャンプする
            if( m_jump_y == 0 ) m_jump_y = 1;

            adjust->set_value( m_jump_y );
            m_jump_y = -1;
        }
    }
}



//
// コメント色変更( 再帰用 )
//
void BBSListViewBase::set_fgcolor_of_comment( const Gtk::TreeModel::Children& children )
{
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        Gtk::TreeModel::Row row = *it;

        const int type = row2type( row );
        if( type == TYPE_COMMENT ) row[ m_columns.m_fgcolor ] = Gdk::Color( CONFIG::get_color( COLOR_CHAR_BBS_COMMENT ) );
        else if( type == TYPE_DIR ) set_fgcolor_of_comment( row.children() );
    }
}


//
// 再描画
//
void BBSListViewBase::relayout()
{
    m_treeview.init_color( COLOR_CHAR_BBS, COLOR_BACK_BBS, COLOR_BACK_BBS_EVEN );
    m_treeview.init_font( CONFIG::get_fontname( FONT_BBS ) );

    set_fgcolor_of_comment( m_treestore->children() );
}


//
// フォーカス
//
void BBSListViewBase::focus_view()
{
    // 行の名前を編集中なら何もしない
    if( m_treeview.is_renaming_row() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::focus_view url = " << get_url() << std::endl;
#endif 

    m_treeview.grab_focus();
}


//
// フォーカスアウト
//
void BBSListViewBase::focus_out()
{
    SKELETON::View::focus_out();

    m_treeview.hide_tooltip();
    m_treeview.delete_popup();
}


//
// 閉じる
//
void BBSListViewBase::close_view()
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::close_view\n";
#endif 

    CORE::core_set_command( "toggle_sidebar" );
}



//
// 削除
//
// BBSListViewBaseの場合は選択行の削除
//
void BBSListViewBase::delete_view()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    delete_view_impl();
}

// BBSListViewBaseの場合は選択行の削除
void BBSListViewBase::delete_view_impl()
{
    const bool force = false;
    m_treeview.delete_selected_rows( force );
}



//
// 内容更新
//
// URLを新しいアドレスに変更するだけ
//
void BBSListViewBase::update_item( const std::string& url, const std::string& id )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::update_item " << url << " / " << get_url() << std::endl;
#endif

    if( url == get_url() ) update_urls();
}


//
// viewの操作
//
const bool BBSListViewBase::operate_view( const int control )
{
    Gtk::TreePath path = m_treeview.get_current_path();
    Gtk::TreeModel::Row row;
    bool open_tab = false;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::operate_view = " << control << std::endl;
#endif 

    switch( control ){

        case CONTROL::Down:
            row_down();
            break;

        case CONTROL::Up:
            row_up();
            break;

        case CONTROL::PageUp:
            page_up();
            break;

        case CONTROL::PageDown:
            page_down();
            break;

        case CONTROL::Home:
            goto_top();
            break;
            
        case CONTROL::End:
            goto_bottom();
            break;

            // 全て選択
        case CONTROL::SelectAll:
            slot_select_all();
            break;

            // 開く
        case CONTROL::OpenBoardTabButton:
            if( ! m_path_selected.empty() ){
                open_tab = true;
                // pathがディレクトリでタブで開くボタンを入れ替えている時はディレクトリ開閉にする
                if( path2type( path ) == TYPE_DIR && CONTROL::is_toggled_tab_button() ) open_tab = false;
                open_row( path, open_tab );
            }
            break;

        case CONTROL::OpenBoardButton:
            if( ! m_path_selected.empty() ){
                open_tab = false;
                // pathがディレクトリでタブで開くボタンを入れ替えている時は更新チェックにする
                if( path2type( path ) == TYPE_DIR && CONTROL::is_toggled_tab_button() ) open_tab = true;
                open_row( path, open_tab );
            }
            break;

        case CONTROL::OpenBoardTab:
            open_tab = true;
            // pathがディレクトリでタブで開くキーを入れ替えている時はディレクトリ開閉にする
            if( path2type( path ) == TYPE_DIR && CONTROL::is_toggled_tab_key() ) open_tab = false;
            open_row( path, open_tab );
            break;

        case CONTROL::OpenBoard:
            open_tab = false;
            // pathがディレクトリでタブで開くキーを入れ替えている時は更新チェックにする
            if( path2type( path ) == TYPE_DIR && CONTROL::is_toggled_tab_key() ) open_tab = true;
            open_row( path, open_tab );
            break;

        case CONTROL::Right:

            if( m_treeview.get_row( path ) ){
                if( ! m_treeview.expand_row( path, false ) ) switch_rightview();
            }
            break;

        case CONTROL::Left:

            if( row = m_treeview.get_row( path ) ){

                if( ( path2type( path ) != TYPE_DIR || ! m_treeview.row_expanded( path ) ) && row.parent() ){
                    path = GET_PATH( row.parent() );
                    m_treeview.set_cursor( path );
                }
                m_treeview.collapse_row( path );
            }
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

        case CONTROL::TabLeft:
            BBSLIST::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            BBSLIST::get_admin()->set_command( "tab_right" );
            break;

        // タブ位置(1-9)で移動
        case CONTROL::TabNum1:
            BBSLIST::get_admin()->set_command( "tab_num", "", "1" );
            break;

        case CONTROL::TabNum2:
            BBSLIST::get_admin()->set_command( "tab_num", "", "2" );
            break;

        case CONTROL::TabNum3:
            BBSLIST::get_admin()->set_command( "tab_num", "", "3" );
            break;

        case CONTROL::TabNum4:
            BBSLIST::get_admin()->set_command( "tab_num", "", "4" );
            break;

        case CONTROL::TabNum5:
            BBSLIST::get_admin()->set_command( "tab_num", "", "5" );
            break;

        case CONTROL::TabNum6:
            BBSLIST::get_admin()->set_command( "tab_num", "", "6" );
            break;

        case CONTROL::TabNum7:
            BBSLIST::get_admin()->set_command( "tab_num", "", "7" );
            break;

        case CONTROL::TabNum8:
            BBSLIST::get_admin()->set_command( "tab_num", "", "8" );
            break;

        case CONTROL::TabNum9:
            BBSLIST::get_admin()->set_command( "tab_num", "", "9" );
            break;

        // 終了
        case CONTROL::Quit:
            close_view();
            break;

        case CONTROL::Reload:
            reload();
            break;
            
        case CONTROL::Delete:
            delete_view();
            break;

        // ポップアップメニュー表示
        case CONTROL::ShowPopupMenu:
        {
            if( m_treeview.get_selection()->get_selected_rows().size() >= 1 ){
                m_path_selected = * (m_treeview.get_selection()->get_selected_rows().begin() );
            }
            SHOW_POPUPMENU(true);
            break;
        }

        // 検索
        case CONTROL::Search:
            set_search_invert( false );
            BBSLIST::get_admin()->set_command( "focus_toolbar_search" );
            break;

        case CONTROL::SearchInvert:
            set_search_invert( true );
            BBSLIST::get_admin()->set_command( "focus_toolbar_search" );
            break;

        case CONTROL::SearchNext:
            down_search();
            break;
    
        case CONTROL::SearchPrev:
            up_search();
            break;

            // サイドバー表示/非表示
        case CONTROL::ShowSideBar:
            CORE::core_set_command( "toggle_sidebar" );
            break;

            // メニューバー表示/非表示
        case CONTROL::ShowMenuBar:
            CORE::core_set_command( "toggle_menubar" );
            break;

            // お気に入り更新チェック
        case CONTROL::CheckUpdateRoot:
            CORE::core_set_command( "check_update_root" );
            break;

        case CONTROL::CheckUpdateOpenRoot:
            CORE::core_set_command( "check_update_open_root" );
            break;

        case CONTROL::StopLoading:
            CORE::core_set_command( "cancel_check_update" );
            break;

        case CONTROL::Undo:
            undo();
            break;

        case CONTROL::Redo:
            redo();
            break;

        default:
            return false;
    }

    return true;
}



//
// ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
//
// SKELETON::View::show_popupmenu() を参照すること
//
void BBSListViewBase::activate_act_before_popupmenu( const std::string& url )
{
    Glib::RefPtr< Gtk::Action > act_search, act_import, act_board, act_article, act_image, act_opentab;
    act_search = action_group()->get_action( "SearchCacheBoard" );
    act_import = action_group()->get_action( "ImportDat" );
    act_board = action_group()->get_action( "PreferenceBoard" );
    act_article = action_group()->get_action( "PreferenceArticle" );
    act_image = action_group()->get_action( "PreferenceImage" );
    act_opentab = action_group()->get_action( "OpenTab" );

    if( act_search ) act_search->set_sensitive( false );
    if( act_import ) act_import->set_sensitive( false );
    if( act_board ) act_board->set_sensitive( false );
    if( act_article ) act_article->set_sensitive( false );
    if( act_image ) act_image->set_sensitive( false );
    if( act_opentab ) act_opentab->set_sensitive( true );

    int type = path2type( m_path_selected );
    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            if( act_search ) act_search->set_sensitive( true );
            if( act_import ) act_import->set_sensitive( true );
            if( act_board ) act_board->set_sensitive( true );
            break;

        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            if( act_article ) act_article->set_sensitive( true );
            break;

        case TYPE_IMAGE:
            if( act_image && ! DBIMG::get_abone( url ) ) act_image->set_sensitive( true );
            break;

        case TYPE_DIR:
            break;

        case TYPE_LINK:
            if( act_opentab ) act_opentab->set_sensitive( false );
            break;
    }
}


// idからポップアップメニュー取得
Gtk::Menu* BBSListViewBase::id2popupmenu( const std::string& id )
{
    return dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( id ) );
}


//
// 先頭に戻る
//
void BBSListViewBase::goto_top()
{
    m_treeview.goto_top();
    show_status();
}


//
// 一番最後へ
//
void BBSListViewBase::goto_bottom()
{
    m_treeview.goto_bottom();
    show_status();
}


//
// 上へ移動
//
void BBSListViewBase::row_up()
{
    m_treeview.row_up();
    show_status();
}    


//
// 下へ移動
//
void BBSListViewBase::row_down()
{
    m_treeview.row_down();
    show_status();
} 
   

//
// page up
//
void BBSListViewBase::page_up()
{
    m_treeview.page_up();
}    


//
// page down
//
void BBSListViewBase::page_down()
{
    m_treeview.page_down();
} 


//
// 他のviewのtreestoreをcopyして表示
//
void BBSListViewBase::copy_treestore( Glib::RefPtr< Gtk::TreeStore >& store )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::copy_treestore\n";
#endif
    m_treestore = store;
    m_treeview.set_treestore( m_treestore );

    if( m_treestore->children().begin() ){

        Gtk::TreePath path = GET_PATH( *( m_treestore->children().begin() ) );
        if( m_treeview.get_row( path ) ){
            m_treeview.collapse_all();
            m_treeview.scroll_to_row( path, 0 );
            m_treeview.get_selection()->unselect_all();
            m_treeview.set_cursor( path );
        }
    }
}




//
// マウスボタン押した
//
bool BBSListViewBase::slot_button_press( GdkEventButton* event )
{
    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    // ダブルクリック
    // button_release_eventでは event->type に必ず GDK_BUTTON_RELEASE が入る
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

    // 親ウィンドウがメインウィンドウならフォーカスを移す
    if( ! get_parent_win() ) BBSLIST::get_admin()->set_command( "switch_admin" );

    return true;
}


//
// マウスボタン離した
//
bool BBSListViewBase::slot_button_release( GdkEventButton* event )
{
    /// マウスジェスチャ
    int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない 
    if( get_control().MG_wheel_end( event ) ) return true;

    if( mg != CONTROL::None && enable_mg() ){
        operate_view( mg );
        return true;
    }

    show_status();

    m_path_selected = m_treeview.get_path_under_xy( (int)event->x, (int)event->y );

    // ダブルクリックの処理のため一時的にtypeを切替える
    GdkEventType type_copy = event->type;
    if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

    // 行を開く
    if( get_control().button_alloted( event, CONTROL::OpenBoardButton ) ) operate_view( CONTROL::OpenBoardButton );

    // タブで開く
    else if( get_control().button_alloted( event, CONTROL::OpenBoardTabButton ) ) operate_view( CONTROL::OpenBoardTabButton );

    // ポップアップメニューボタン
    else if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ) SHOW_POPUPMENU( false );

    // その他の操作
    else operate_view( get_control().button_press( event ) );

    event->type = type_copy;

    return true;
}




//
// マウス動かした
//
bool BBSListViewBase::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

    int x = (int)event->x;
    int y = (int)event->y;
    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* column;
    int cell_x;
    int cell_y;

    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) && m_treeview.get_row( path ) ){

        const int mrg = 16; // アイコンの横幅。計算するのが面倒だったのでとりあえず
        
        Gtk::TreeModel::Row row = m_treeview.get_row( path );
        Glib::ustring subject = row[ m_columns.m_name ];
        Glib::ustring url = row[ m_columns.m_url ];
        int type = row[ m_columns.m_type ];

        // 画像ポップアップ
        if( type == TYPE_IMAGE ){

            m_treeview.hide_tooltip();

            if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN && DBIMG::get_code( url ) != HTTP_INIT ){

                if( m_treeview.pre_popup_url() != url ){

                    m_treeview.delete_popup();
                    SKELETON::View* view = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP,  url );
                    m_treeview.show_popup( url, view );
                }
            }
            else m_treeview.delete_popup();
        }

        // ツールチップ
        else{

            m_treeview.delete_popup();

            Gdk::Rectangle rect;
            m_treeview.get_cell_area( path, *column, rect );
            m_treeview.set_tooltip_min_width( rect.get_width() - mrg );
            m_treeview.set_str_tooltip( subject );
        }
    }
    else{
        m_treeview.hide_tooltip();
        m_treeview.delete_popup();
    }
    
    return true;
}



//
// キーを押した
//
bool BBSListViewBase::slot_key_press( GdkEventKey* event )
{
    // 行の名前を編集中なら何もしない
    if( m_treeview.is_renaming_row() ) return false;

    const int key = get_control().key_press( event );

    // キー入力でboardを開くとkey_pressイベントがboadviewに送られて
    // 一番上のスレが開くので、open_row() は slot_key_release() で処理する
    if( key == CONTROL::OpenBoard ) return true;
    if( key == CONTROL::OpenBoardTab ) return true;

    if( operate_view( key ) ) return true;

    return false;
}


//
// キー上げた
//
bool BBSListViewBase::slot_key_release( GdkEventKey* event )
{
#ifdef _DEBUG
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;

    std::cout << "BBSListViewBase::slot_key_release key = " << event->keyval << " ctrl = " << ctrl << " shift = " << shift << std::endl;
#endif

    // 行の名前を編集中なら何もしない
    if( m_treeview.is_renaming_row() ) return false;

    // キー入力でboardを開くとkey_pressイベントがboadviewに送られて
    // 一番上のスレが開くので、open_row() は slot_key_release() で処理する
    int key = get_control().key_press( event );
    if( key == CONTROL::OpenBoard || key == CONTROL::OpenBoardTab ) operate_view( key );

    return true;
}


//
// マウスホイールイベント
//
bool BBSListViewBase::slot_scroll_event( GdkEventScroll* event )
{
    // ホイールマウスジェスチャ
    int control = get_control().MG_wheel_scroll( event );
    if( enable_mg() && control != CONTROL::None ){
        operate_view( control );
        return true;
    }

    m_treeview.wheelscroll( event );
    return true;
}


//
// 他のwidgetからドロップされた
//
void BBSListViewBase::slot_dropped_from_other( const CORE::DATA_INFO_LIST& list_info )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase:slot_dropped_from_other\n";
#endif    

    CORE::DATA_INFO_LIST::const_iterator it = list_info.begin();
    for( ; it != list_info.end() ; ++it ){

        const CORE::DATA_INFO& info = ( *it );
        const int type = info.type;

        // ブックマークセット
        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE || type == TYPE_THREAD_OLD ){

            if( CONFIG::get_bookmark_drop() ) DBTREE::set_bookmarked_thread( info.url, true );
        }

        else if( type == TYPE_IMAGE ) DBIMG::set_protect( info.url, true );
    }
}


//
// popupmenu でタブで開くを選択
//
void BBSListViewBase::slot_open_tab()
{
    if( m_path_selected.empty() ) return;

    open_row( m_path_selected, true );
}


//
// ブラウザで開く
//
void BBSListViewBase::slot_open_browser()
{
    if( m_path_selected.empty() ) return;

    std::string url = path2url( m_path_selected );
    CORE::core_set_command( "open_url_browser", url );
}


//
// 選択行をお気に入りに追加
//
void BBSListViewBase::slot_append_favorite()
{
    CORE::DATA_INFO_LIST list_info;
    const bool dir = true;
    m_treeview.get_info_in_selection( list_info, dir );

    if( list_info.size() ){

        CORE::SBUF_set_list( list_info );
        CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
    }
}


//
// メニューでディレクトリを作るを選択
//
void BBSListViewBase::slot_newdir()
{
    m_path_selected = m_treeview.create_newdir( m_path_selected );
}


//
// メニューでコメント挿入を選択
//
void BBSListViewBase::slot_newcomment()
{
    m_path_selected = m_treeview.create_newcomment( m_path_selected );
}


//
// メニューで外部板追加を選択
//
void BBSListViewBase::slot_newetcboard()
{
    add_newetcboard( false, "", "", "", "" );
}


//
// メニューで外部板編集を選択
//
void BBSListViewBase::slot_moveetcboard()
{
    add_newetcboard( true, "", "", "", "" );
}


// 外部板追加
void BBSListViewBase::add_newetcboard( const bool move, // true なら編集モード
                                       const std::string& _url, const std::string& _name, const std::string& _id, const std::string& _passwd )
{
    if( m_path_selected.empty() ) return;

    std::string url_old;
    std::string name_old;

    std::string url = _url;
    std::string name = _name;
    std::string id = _id;
    std::string passwd = _passwd;
    std::string basicauth;

#ifdef _DEBUG
        std::cout << "BBSListViewBase::add_newetcboard\n"
                  << "move = " << move << std::endl;
#endif

    if( move ){
        url_old = path2url( m_path_selected );
        name_old = path2name( m_path_selected );

#ifdef _DEBUG
        std::cout << "url_old = " << url_old << std::endl
                  << "name_old = " << name_old << std::endl
                  << "board_name = " << DBTREE::board_name( url_old ) << std::endl;
#endif

        if( DBTREE::board_name( url_old ) != name_old ) return;

        if( url.empty() ) url = url_old;
        if( name.empty() ) name = name_old;

        basicauth = DBTREE::board_basicauth( url_old );
        size_t i = basicauth.find( ":" );

        if( id.empty() && i != std::string::npos ){

            id = basicauth.substr( 0, i );
            passwd = basicauth.substr( i+1 );
        }

#ifdef _DEBUG
        std::cout << "basicauth = " << basicauth << " i = " << i << " id = " << id
                  << "passwd = " << passwd << std::endl;
#endif
    }

    BBSLIST::AddEtcDialog diag( move, url, name, id, passwd );
    if( diag.run() == Gtk::RESPONSE_OK ){

        diag.hide();

        std::string url_org = MISC::remove_space( diag.get_url() );
        name = MISC::remove_space( diag.get_name() );
        url = url_org;
        id = MISC::remove_space( diag.get_id() );
        passwd = MISC::remove_space( diag.get_passwd() );
        if( ! id.empty() && ! passwd.empty() ) basicauth = id + ":" + passwd; 

        if( name.empty() || url.empty() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "板名またはアドレスが空白です", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name, id, passwd );
            return;
        }

        // http が無ければ付ける
        if( url.find( "http://" ) != 0 ) url = "http://" + url;

        // .htmlを取り除く
        JDLIB::Regex regex;
        if( regex.exec( "(.*)/[^/]+\\.html?$" , url ) ) url = regex.str( 1 );

        // 末尾の / を取り除く
        while( url.rfind( "/" ) == url.length() -1 ) url = url.substr( 0, url.length() -1 );

        // url の最後に/を付ける
        url += "/";

        // boardid 取得
        if( ! regex.exec( "(http://.*)/([^/]*)/$" , url ) ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "アドレスが不正な形式になっています", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name, id, passwd );
            return;
        }

#ifdef _DEBUG
        std::cout << "url_old = " << url_old << std::endl
                  << "name_old = " << name_old << std::endl
                  << "url = " << url << std::endl
                  << "name = " << name << std::endl
                  << "basicauth = " << basicauth << std::endl;
#endif

        std::string boardid = regex.str( 2 );

        if( boardid.empty() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "板IDを取得出来ません", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name, id, passwd );
            return;
        }

#ifdef _DEBUG
        std::cout << "boardid = " << boardid << std::endl;
#endif

        // 既に登録されているか確認
        if( ! move && ! DBTREE::board_name( url ).empty() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), name + "\n" + url + "\n\nは既に登録されています", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name,  id, passwd );
            return;
        }

        // データベースに登録してツリーに表示
        if( ! move && DBTREE::add_etc( url, name, basicauth, boardid ) ){

            CORE::DATA_INFO_LIST list_info;
            CORE::DATA_INFO info;
            info.type = TYPE_BOARD;
            info.url = url;
            info.name = name;
            info.data = std::string();
            info.path = m_path_selected.to_string();
            list_info.push_back( info );

            const bool before = false;
            const bool scroll = false;
            const bool force = true;  // 強制的に追加
            m_treeview.append_info( list_info, m_path_selected, before, scroll, force );
            m_path_selected = m_treeview.get_current_path();

            // etc.txt保存
            DBTREE::save_etc();
        }

        // 編集
        else if( move && DBTREE::move_etc( url_old, url, name_old, name, basicauth, boardid ) ){

            Gtk::TreeModel::Row row = m_treeview.get_row( m_path_selected );
            if( row ){
                row[ m_columns.m_url ] = url;
                row[ m_columns.m_name ] = name;
            }
        }

        else{

            SKELETON::MsgDiag mdiag( get_parent_win(), "外部板の登録に失敗しました。アドレスを確認してください", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name, id, passwd );
            return;
        }
    }
}



//
// 名前変更
//
void BBSListViewBase::slot_rename()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_rename\n";
#endif

    m_treeview.rename_row( m_path_selected );
}


//
// URLをコピー
//
void BBSListViewBase::slot_copy_url()
{
    if( m_path_selected.empty() ) return;

    std::string url = path2url( m_path_selected );
    MISC::CopyClipboard( url );
}


// 名前とURLをコピー
//
void BBSListViewBase::slot_copy_title_url()
{
    if( m_path_selected.empty() ) return;

    std::string url = path2url( m_path_selected );
    std::string name = path2name( m_path_selected );
    std::stringstream ss;
    ss << name << std::endl
       << url << std::endl;

    MISC::CopyClipboard( ss.str() );
}


//
// 指定したディレクトリ以下のディレクトリを全て開く
//
void BBSListViewBase::expand_all_dir( Gtk::TreeModel::Path path )
{
    if( m_treeview.is_dir( path ) ){

        if( ! m_treeview.row_expanded( path ) ){
            m_cancel_expand = true; // slot_row_exp()の呼び出しをキャンセル
            m_treeview.expand_row( path, false );
            m_cancel_expand = false;
        }
        path.down();

        while( m_treeview.get_row( path ) ){
            expand_all_dir( path );
            path.next();
        }
    }
}



//
// ディレクトリ内を全選択(メニューから呼び出す)
//
// m_path_selected にパスをセットしておくこと
//
void BBSListViewBase::slot_select_all_dir()
{
    if( m_path_selected.empty() ) return;

    m_treeview.select_all_dir( m_path_selected );
}



//
// 全選択
//
void BBSListViewBase::slot_select_all()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "全ての行を選択しますか？\n\n(注意) ディレクトリは全て展開されます。", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    mdiag.set_default_response( Gtk::RESPONSE_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    Gtk::TreeModel::Children child = m_treestore->children();
    Gtk::TreeModel::Children::iterator it = child.begin();
    for( ; it != child.end() ; ++it ) expand_all_dir( m_treestore->get_path( *it ) );

    it = child.begin();
    m_treeview.scroll_to_row( m_treestore->get_path( *it ), 0 );
    for( ; it != child.end() ; ++it ){
        m_treeview.get_selection()->select( *it );
        m_treeview.select_all_dir( m_treestore->get_path( *it ) );
    }
}



//
// ディレクトリ内を全更新チェック
//
// 呼び出した後に CORE::get_checkupdate_manager()->run() を実行すること
//
void BBSListViewBase::check_update_dir( Gtk::TreeModel::Path path, const bool open )
{
    if( path.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::check_update_dir path = " << path.to_string() << std::endl;
#endif

    if( m_treeview.is_dir( path ) ){

        path.down();

        while( m_treeview.get_row( path ) ){

            int type = path2type( path );
            std::string url = path2url( path );

            if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ), open );
            else if( CONFIG::get_check_update_board() && ( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ) )
                CORE::get_checkupdate_manager()->push_back( DBTREE::url_subject( url ), open );
            else if( type == TYPE_DIR ) check_update_dir( path, open );

            path.next();
        }
    }
}


//
// ディレクトリ内を全更新チェック
//
// m_path_selected にパスをセットしておくこと
//
void BBSListViewBase::slot_check_update_dir()
{
    if( m_path_selected.empty() ) return;

    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_check_check_update_dir path = " << m_path_selected.to_string() << std::endl;
#endif

    check_update_dir( m_path_selected, false );
    CORE::get_checkupdate_manager()->run();
}

// 全更新チェックして開く
void BBSListViewBase::slot_check_update_open_dir()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

    if( m_path_selected.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_check_check_update_open_dir path = " << m_path_selected.to_string() << std::endl;
#endif

    check_update_dir( m_path_selected, true );
    CORE::get_checkupdate_manager()->run();
}


//
// ルート以下を全て更新チェック( 再帰用 )
//
// 呼び出した後に CORE::get_checkupdate_manager()->run() を実行すること
//
void BBSListViewBase::check_update_root( const Gtk::TreeModel::Children& children, const bool open )
{
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreeModel::Row row = *it;

        const int type = row2type( row );
        const std::string url = row2url( row );

        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ), open );
        else if( CONFIG::get_check_update_board() && ( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ) )
            CORE::get_checkupdate_manager()->push_back( DBTREE::url_subject( url ), open );
        else if( type == TYPE_DIR ) check_update_root( row.children(), open );

        ++it;
    }
}


//
// ルート以下を全て更新チェック( command 呼び出し用 )
//
// tab_open はタブで開くか否か
//
void BBSListViewBase::check_update_root( const bool open )
{
    check_update_root( m_treestore->children(), open );

    CORE::get_checkupdate_manager()->run();
}


//
// 更新チェックキャンセル
//
void BBSListViewBase::slot_cancel_check_update()
{
    CORE::get_checkupdate_manager()->stop();
}


//
// キャッシュ内のログ検索
//
void BBSListViewBase::slot_search_cache_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    CORE::core_set_command( "open_article_searchlog", url );
}


//
// datのインポート
//
void BBSListViewBase::slot_import_dat()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    CORE::core_set_command( "import_dat", url, "show_diag" );
}


//
// 板プロパティ表示
//
void BBSListViewBase::slot_preferences_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_BOARD, DBTREE::url_subject( url ) );
    pref->run();
    delete pref;
}



//
// スレプロパティ表示
//
void BBSListViewBase::slot_preferences_article()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_ARTICLE, DBTREE::url_dat( url ) );
    pref->run();
    delete pref;
}


//
// 画像プロパティ表示
//
void BBSListViewBase::slot_preferences_image()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_IMAGE, url );
    pref->run();
    delete pref;
}



//
// フォルダを開いた時に呼ばれる
//
void BBSListViewBase::slot_row_exp( const Gtk::TreeModel::iterator&, const Gtk::TreeModel::Path& path )
{
    if( m_cancel_expand ) return;
    if( m_expanding ) return;

    // 他のフォルダを全て閉じる
    if( m_open_only_onedir
        && path.get_depth() == 1  // 子フォルダの時は閉じない
        ){
        m_expanding = true;
        m_treeview.collapse_all();
        m_treeview.expand_row( path, false );
        m_expanding = false;
    }

    m_treeview.set_cursor( path );
    m_treeview.scroll_to_row( path, 0.1 );
    show_status();
}

//
// フォルダを閉じた時に呼ばれる
//
void BBSListViewBase::slot_row_col( const Gtk::TreeModel::iterator&, const Gtk::TreeModel::Path& path )
{
    if( m_expanding ) return;

    m_treeview.set_cursor( path );
    show_status();
}


//
// 選択した行を開く 
//
const bool BBSListViewBase::open_row( Gtk::TreePath& path, const bool tab )
{
    if( !m_treeview.get_row( path ) ) return false;

    std::string str_tab = "false";
    if( tab ) str_tab = "true";

    const std::string str_mode = "";

    Glib::ustring url = path2url( path );
    int type = path2type( path );

    if( type != TYPE_DIR && url.empty() ) return false;

    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            CORE::core_set_command( "open_board", DBTREE::url_subject( url ), str_tab, str_mode );
            break;

        case TYPE_THREAD_OLD:
            toggle_articleicon( url ); // break;しない
        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
            CORE::core_set_command( "open_article", DBTREE::url_dat( url ), str_tab, str_mode );
            break;

        case TYPE_IMAGE:

            if( DBIMG::get_abone( url )){
                SKELETON::MsgDiag mdiag( get_parent_win(), "あぼ〜んされています" );
                mdiag.run();
            }
            else{
                CORE::core_set_command( "open_image", url );
                CORE::core_set_command( "switch_image" );
            }
            break;

        case TYPE_LINK:
            CORE::core_set_command( "open_url_browser", url );
            break;

        case TYPE_DIR:
            if( tab ) slot_check_update_open_dir();
            else if( ! m_treeview.row_expanded( path ) ) m_treeview.expand_row( path, false );
            else m_treeview.collapse_row( path );
            break;
    }
    
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::open_row : path = " << path.to_string() << " tab = " << tab << std::endl;
#endif        

    // treeviewが編集されていたらxml保存
    if( type != TYPE_DIR && m_treeview.is_updated() ){
        save_xml( false );
        m_treeview.set_updated( false );
    }

    return true;
}


//
// 右のビュー(board)に切り替え
//
void BBSListViewBase::switch_rightview()
{
    CORE::core_set_command( "switch_rightview" );
}



//
// 選択した行をまとめて開く
//
void BBSListViewBase::open_selected_rows()
{
    std::string list_url_board;
    std::string list_url_article;

    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){

        Gtk::TreeModel::Row row = *( *it );
        Gtk::TreePath path = GET_PATH( row );

        int type = path2type( path );
        std::string url = path2url( path );

        switch( type ){

            case TYPE_BOARD:
            case TYPE_BOARD_UPDATE:
                url = DBTREE::url_subject( url );
                if( !list_url_board.empty() ) list_url_board += " ";
                list_url_board += url;
                break;

            case TYPE_THREAD:
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:
                url = DBTREE::url_dat( url );
                if( !list_url_article.empty() ) list_url_article += " ";
                list_url_article += url;
                break;
        }
    }

    if( !list_url_board.empty() ) CORE::core_set_command( "open_board_list", std::string(), list_url_board );
    if( !list_url_article.empty() ) CORE::core_set_command( "open_article_list", std::string(), list_url_article );
}



//
// 選択したスレを更新チェック
//
// このあとで CORE::get_checkupdate_manager()->run() を実行すること
//
void BBSListViewBase::checkupdate_selected_rows( const bool open )
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){

        Gtk::TreeModel::Row row = *( *it );
        Gtk::TreePath path = GET_PATH( row );

        int type = path2type( path );
        std::string url = path2url( path );

        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ), open );
        else if( CONFIG::get_check_update_board() && ( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ) )
            CORE::get_checkupdate_manager()->push_back( DBTREE::url_subject( url ), open );
    }
}


void BBSListViewBase::slot_checkupdate_selected_rows()
{
    checkupdate_selected_rows( false );
    CORE::get_checkupdate_manager()->run();
}


void BBSListViewBase::slot_checkupdate_open_selected_rows()
{
    checkupdate_selected_rows( true );
    CORE::get_checkupdate_manager()->run();
}


//
// path -> url 変換
//

Glib::ustring BBSListViewBase::path2rawurl( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return Glib::ustring();
    Glib::ustring url =  row[ m_columns.m_url ];
    return url;
}

// 移転をチェックするバージョン
Glib::ustring BBSListViewBase::path2url( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return Glib::ustring();

    Glib::ustring url =  row[ m_columns.m_url ];
    if( url.empty() ) return url;

    // 移転があったら url を最新のものに変換しておく
    int type = path2type( path );
    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            url = DBTREE::url_boardbase( url );
            break;

        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            url = DBTREE::url_readcgi( url, 0, 0 );
            break;
    }

    return url;
}


//
// row -> url 変換
//
Glib::ustring BBSListViewBase::row2url( const Gtk::TreeModel::Row& row )
{
    if( ! row ) return Glib::ustring();

    Glib::ustring url =  row[ m_columns.m_url ];
    if( url.empty() ) return url;

    // 移転があったら url を最新のものに変換しておく
    int type = row2type( row );
    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            url = DBTREE::url_boardbase( url );
            break;

        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            url = DBTREE::url_readcgi( url, 0, 0 );
            break;
    }

    return url;
}



//
// path -> name 変換
//
Glib::ustring BBSListViewBase::path2name( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return Glib::ustring();
    return row[ m_columns.m_name ];
}



//
// path -> type 変換
//
int BBSListViewBase::path2type( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return TYPE_UNKNOWN;
    return row[ m_columns.m_type ];
}


//
// row -> type 変換
//
int BBSListViewBase::row2type( const Gtk::TreeModel::Row& row )
{
    if( ! row ) return TYPE_UNKNOWN;
    return row[ m_columns.m_type ];
}


//
// 外部板のディレクトリか
//
bool BBSListViewBase::is_etcdir( Gtk::TreePath path )
{
    std::string name = path2name( path );

#ifdef _DEBUG
    std::cout << "BBSListViewBase:is_etcdir path = " << path.to_string() << " name = " << name << std::endl;
#endif

    if( name == SUBDIR_ETCLIST ) return true;

    return false;
}


//
// 外部板か
//
bool BBSListViewBase::is_etcboard( Gtk::TreeModel::iterator& it )
{
    Gtk::TreePath path = get_treestore()->get_path( *it );
    return is_etcboard( path );
}


bool BBSListViewBase::is_etcboard( Gtk::TreePath path )
{
    path.up();
    return is_etcdir( path );
}



//
// ステータス表示
//
void BBSListViewBase::show_status()
{
    set_status( path2url( m_treeview.get_current_path() ) );
    BBSLIST::get_admin()->set_command( "set_status", get_url(), get_status() );
}


//
// tree -> XML 変換
//
//
void BBSListViewBase::tree2xml( const std::string& root_name )
{
    if( ! m_ready_tree ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::tree2xml\n";
#endif

    m_treeview.tree2xml( m_document, root_name );

    // 座標
    int y = 0;
    Gtk::Adjustment* adjust = m_treeview.get_vadjustment();
    if( adjust )
    {
        if( m_jump_y != -1 && adjust->get_upper() > m_jump_y ) y = m_jump_y;
        else  y = ( int ) adjust->get_value();
    }
    else if( m_jump_y != -1 ) y = m_jump_y;

    // 選択中のパス
    std::string path;
    Gtk::TreeModel::Path focused_path = m_treeview.get_current_path();
    if( ! focused_path.empty() ) path = focused_path.to_string();

    // ルート要素に属性( path, y )の値を設定
    XML::Dom* root = m_document.get_root_element( root_name );
    if( root ){
        root->setAttribute( "y", y );
        root->setAttribute( "path", path );
    }
}


//
// XML -> tree 変換
//
void BBSListViewBase::xml2tree( const std::string& root_name, const std::string& xml )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::xml2tree\n";
#endif

    m_ready_tree = false;
    m_jump_y = 0;

    // 新規に文字列からDOMノードツリーを作成する場合
    if( ! xml.empty() ) m_document.init( xml );

#ifdef _DEBUG
    std::cout << " ルートノード名=" << root_name;
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif

    m_treeview.xml2tree( m_document, m_treestore, root_name );

    // ルート要素を取り出す
    XML::Dom* root = m_document.get_root_element( root_name );
    if( root ){

	// ルート要素から属性( path, y )の値を取得
	std::string focused_path = root->getAttribute( "path" );
	int y = atoi( root->getAttribute( "y" ).c_str() );

	// 前回閉じた位置まで移動
	if( focused_path.empty() )
	{
		focused_path = "0";
		y = 0;
	}

	Gtk::TreePath path = Gtk::TreePath( focused_path );
	if( m_treeview.get_row( path ) ) m_treeview.set_cursor( path );
	else
	{
		m_treeview.get_selection()->unselect_all();
		y = 0;
	}

	// この段階ではまだスクロールバーが表示されてない時があるのでclock_in()で移動する
	m_jump_y = y;
    }

    m_ready_tree = true;
}



//
// 起動時や移転があったときなどに行に含まれるURlを変更する
//
void BBSListViewBase::update_urls()
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return; 
   
#ifdef _DEBUG
    std::cout << "BBSListViewBase::update_urls()\n";
#endif
    
    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            const Glib::ustring url = row[ m_columns.m_url ];
            const int type = row[ m_columns.m_type ];
            std::string url_new;

            switch( type ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_BOARD: // 板
                case TYPE_BOARD_UPDATE:
                    url_new = DBTREE::is_board_moved( url );
                    if( ! url_new.empty() ){
                        url_new = DBTREE::url_boardbase( url );
#ifdef _DEBUG
                        std::cout << url << " -> " << url_new << std::endl;
#endif
                        row[ m_columns.m_url ] = url_new;
                    }
                    path.next();
                    break;

                case TYPE_THREAD: // スレ
                case TYPE_THREAD_UPDATE:
                case TYPE_THREAD_OLD:
                    url_new = DBTREE::is_board_moved( url );
                    if( ! url_new.empty() ){

                        url_new = DBTREE::url_dat( url );
#ifdef _DEBUG
                        std::cout << url << " -> " << url_new << std::endl;
#endif
                        row[ m_columns.m_url ] = url_new;
                    }
                    path.next();
                    break;

                default:
                    path.next();
                    break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){
                path.up();
                path.next();
            }
            else break;
        }
    }
}


//
// アイコン表示(スレ)の切り替え
//
void BBSListViewBase::toggle_articleicon( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return; 
   
    const std::string url_dat = DBTREE::url_dat( url );
    const std::string url_cgi = DBTREE::url_readcgi( url, 0, 0 );

    int type = TYPE_THREAD;
    const int status = DBTREE::article_status( url );
    if( status & STATUS_OLD ) type = TYPE_THREAD_OLD;
    else if( status & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;
    
#ifdef _DEBUG
    std::cout << "BBSListViewBase::toggle_articleicon url = " << url_dat << " type = " << type << std::endl;
#endif

    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            const Glib::ustring url_row = row[ m_columns.m_url ];

            switch( row[ m_columns.m_type ] ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_THREAD:
                case TYPE_THREAD_UPDATE:
                case TYPE_THREAD_OLD:
                    if( url_dat == url_row || url_cgi == url_row ){
#ifdef _DEBUG
                        std::cout << "hit " << url_dat << " == " << url_row << std::endl;
#endif
                        row[ m_columns.m_type ] = type;
                        row[ m_columns.m_image ] = XML::get_icon( type );
                    }
                    path.next();
                    break;

                default:
                    path.next();
                    break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){
                path.up();
                path.next();
            }
            else break;
        }
    }
}



//
// アイコン表示(板)の切り替え
//
void BBSListViewBase::toggle_boardicon( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return;
   
    const std::string url_subject = DBTREE::url_subject( url );

    int type = TYPE_BOARD;
    const int status = DBTREE::board_status( url );
    if( status & STATUS_UPDATE ) type = TYPE_BOARD_UPDATE;
    
#ifdef _DEBUG
    std::cout << "BBSListViewBase::toggle_boardicon url = " << url_subject << " type = " << type << std::endl;
#endif

    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            const Glib::ustring url_row = row[ m_columns.m_url ];

            switch( row[ m_columns.m_type ] ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_BOARD:
                case TYPE_BOARD_UPDATE:

                    if( url_subject == DBTREE::url_subject( url_row ) ){
#ifdef _DEBUG
                        std::cout << "hit " << url_subject << " == " << url_row << std::endl;
#endif
                        row[ m_columns.m_type ] = type;
                        row[ m_columns.m_image ] = XML::get_icon( type );
                    }
                    path.next();
                    break;

                default:
                    path.next();
                    break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){
                path.up();
                path.next();
            }
            else break;
        }
    }
}



//
// スレの url と 名前を変更
//
void BBSListViewBase::replace_thread( const std::string& url, const std::string& url_new )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::replace_thread url = " << url
              << " url_new = " << url_new << std::endl;
#endif

    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return; 

    const std::string urldat_new = DBTREE::url_dat( url_new );
    if( urldat_new.empty() ) return;

    const std::string name_new = DBTREE::article_subject( urldat_new );
    if( name_new.empty() ) return;
   
    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    const std::string urldat = DBTREE::url_dat( url );
    const std::string urlcgi = DBTREE::url_readcgi( url, 0, 0 );
    const std::string name_old = MISC::remove_space( DBTREE::article_subject( urldat ) );

    int type = TYPE_THREAD;
    const int status = DBTREE::article_status( urldat_new );
    if( status & STATUS_OLD ) type = TYPE_THREAD_OLD;
    if( status & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;

#ifdef _DEBUG
    std::cout << "name_new = " << name_new << std::endl
              << "name_old = " << name_old << std::endl
              << "type = " << type << std::endl;
#endif

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            const Glib::ustring url_row = row[ m_columns.m_url ];

            switch( row[ m_columns.m_type ] ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_THREAD:
                case TYPE_THREAD_UPDATE:
                case TYPE_THREAD_OLD:

                    if( urldat == url_row || urlcgi == url_row ){

                        row[ m_columns.m_url ] = urldat_new;

                        // 名前が古いものであったら更新
                        // 変更されていたらそのまま
                        const Glib::ustring name_row = row[ m_columns.m_name ];
#ifdef _DEBUG
                        std::cout << "name_row = " << name_row << std::endl;
#endif 
                        if( MISC::remove_space( name_row ) == name_old ){
#ifdef _DEBUG
                            std::cout << "replace name\n";
#endif 
                            row[ m_columns.m_name ] = name_new;
                        }

                        row[ m_columns.m_type ] = type;
                        row[ m_columns.m_image ] = XML::get_icon( type );

                        // 行を開いた時にxmlを保存
                        m_treeview.set_updated( true ); 
                    }

                default:
                    path.next();
                    break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){
                path.up();
                path.next();
            }
            else break;
        }
    }
}


//
// 検索
//
// m_search_invert = true なら前方検索
//
void BBSListViewBase::exec_search()
{
    JDLIB::Regex regex_name;
    JDLIB::Regex regex_url;

    CORE::core_set_command( "set_info", "", "" );

    std::string query = get_search_query();
    if( query.empty() ){
        focus_view();
        return;
    }
    
    Gtk::TreePath path = m_treeview.get_current_path();
    if( !m_treeview.get_row( path ) ){
        goto_top();
        path = m_treeview.get_current_path();
    }

    // queryが新しいのに更新されたらひとつ前か後から検索をかける
    if( query != m_pre_query ){
        if( !m_search_invert ) path = m_treeview.prev_path( path, false );
        else path = m_treeview.next_path( path, false );
        m_pre_query = query;
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_BBSLIST, query );
    } 

    Gtk::TreePath path_start = path;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::exec_search() path = " << path.to_string() << " query = " << query << std::endl;
#endif
	
    regex_name.compile( query, true, true, true );
    regex_url.compile( query, true );

    bool hit = false;
    for(;;){

        // 後方
        if( !m_search_invert ){

            path = m_treeview.next_path( path, false );

            // 一番最後を過ぎたら先頭に戻る
            if( ! m_treeview.get_row( path ) ) path =  GET_PATH( *( m_treestore->children().begin() ) );
        }

        // 前方
        else{

            // 先頭にいるときは最後に戻る
            if( path == GET_PATH( *( m_treestore->children().begin() ) ) ){

                path = GET_PATH( *( m_treestore->children().rbegin() ) );
                Gtk::TreePath path_tmp = path;
                while( m_treeview.get_row( path_tmp ) ){
                    path = path_tmp;
                    path_tmp = m_treeview.next_path( path_tmp, false );
                }
            }

            else path = m_treeview.prev_path( path, false );
        }

        Glib::ustring name = path2name( path );
        Glib::ustring url = path2url( path );

        if( regex_name.exec( name, 0 ) || regex_url.exec( url, 0 ) ) hit = true;

        // 一周したら終わり
        if( path == path_start ) break;

        // カーソル移動
        if( hit ){
            m_treeview.expand_parents( path );
            m_treeview.scroll_to_row( path, 0.1 );
            m_treeview.set_cursor( path );
            show_status();
            break;
        }
    }

    if( hit ) focus_view();
    else CORE::core_set_command( "set_info", "", "検索結果： ヒット無し" );
}


// 前検索
void BBSListViewBase::up_search()
{
    set_search_invert( true );
    exec_search();
}



// 後検索
void BBSListViewBase::down_search()
{
    set_search_invert( false );
    exec_search();
}


//
// 検索entryの操作
//
void BBSListViewBase::operate_search( const std::string& controlid )
{
    int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ) focus_view();
}


//
// 挿入先ダイアログを表示してアイテム追加
//
// あらかじめ共有バッファにデータを入れておくこと
//
void BBSListViewBase::append_item()
{
    if( m_editlistwin ){
        m_editlistwin->append_item();
        return;
    }

    if( CORE::SBUF_size() == 0 ) return;

    // 挿入先ダイアログ内で編集を行うとバッファがクリアされてしまうので
    // バックアップを取っておく
    const CORE::DATA_INFO_LIST list_info_bkup = CORE::SBUF_list_info();

    // 挿入先ダイアログ表示
    SelectListDialog diag( get_url(), get_treestore() );
    if( diag.run() != Gtk::RESPONSE_OK ) return;

    bool before = false;
    Gtk::TreePath path;
    const bool scroll = true;

    const std::string path_str = diag.get_path();

    // 先頭
    if( path_str == "-1" ){
        path = Gtk::TreePath( "0" );
        before = true;
    }
    // 最後
    else if( path_str.empty() ) path = Gtk::TreePath();

    else path = Gtk::TreePath( path_str );

    const CORE::DATA_INFO_LIST list_info = m_treeview.append_info( list_info_bkup, path, before, scroll );
    CORE::SBUF_clear_info();
    slot_dropped_from_other( list_info );
}


//
// アイテム削除
//
void BBSListViewBase::remove_item( const std::string& url )
{
    const std::string url_target = DBTREE::url_dat( url );
    if( url_target.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::remove_item url = " << url_target << std::endl;
#endif

    CORE::DATA_INFO_LIST list_info;
    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;
    std::list< Gtk::TreePath > list_path;

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            const Glib::ustring url = row[ m_columns.m_url ];
            const int type = row[ m_columns.m_type ];
            std::string url_new;

            switch( type ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_THREAD: // スレ
                case TYPE_THREAD_UPDATE:
                case TYPE_THREAD_OLD:
                    url_new = DBTREE::url_dat( url );
                    if( url_new == url_target ) list_path.push_back( path );
                    path.next();
                    break;

                default:
                    path.next();
                    break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){
                path.up();
                path.next();
            }
            else break;
        }
    }

    const bool force = false;
    m_treeview.delete_path( list_path, force );
}


//
// ツリーの編集ウィンドウを開く
//
void BBSListViewBase::edit_tree()
{
    if( m_editlistwin ) m_editlistwin->present();

    else{

        m_editlistwin = new EditListWin( get_url(), get_treestore() );
        m_editlistwin->signal_hide().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_hide_editlistwin ) );
        m_editlistwin->show();
    }
}


//
// ツリーの編集ウィンドウが閉じた
//
void BBSListViewBase::slot_hide_editlistwin()
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_hide_editlistwin\n";
#endif

    if( m_editlistwin ) delete m_editlistwin;
    m_editlistwin = NULL;
}


//
// XML保存
//
// remove_dir != empty()の時はその名前のディレクトリを削除する
//
void BBSListViewBase::save_xml_impl( const std::string& file, const std::string& root, const std::string& remove_dir )
{
    if( file.empty() ) return;
    if( root.empty() ) return;
    if( ! get_ready_tree() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::save_xml file = " << file << " root = " << root
              << " remove_dir = " << remove_dir << std::endl;
#endif

    tree2xml( root );

    // 指定したディレクトリを取り除く
    if( ! remove_dir.empty() )
    {
        XML::Dom* domroot = m_document.get_root_element( root );
        XML::DomList domlist = domroot->childNodes();
        std::list< XML::Dom* >::iterator it = domlist.begin();
        while( it != domlist.end() )
        {
            if( (*it)->nodeName() == "subdir"
                && (*it)->getAttribute( "name" ) == remove_dir )    
            {
                domroot->removeChild( *it );
                break;
            }
            ++it;
        }
    }

    CACHE::save_rawdata( file, m_document.get_xml() );
}


// undo, redo
void BBSListViewBase::undo()
{
    m_treeview.undo();
}

void BBSListViewBase::redo()
{
    m_treeview.redo();
}
