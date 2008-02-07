// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "bbslistviewbase.h"
#include "bbslistadmin.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "config/globalconf.h"
#include "config/buttonconfig.h"
#include "config/keyconfig.h"

#include "icons/iconmanager.h"

#include "xml/tools.h"

#include "selectdialog.h"
#include "cache.h"
#include "command.h"
#include "global.h"
#include "httpcode.h"
#include "controlutil.h"
#include "controlid.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "viewfactory.h"
#include "prefdiagfactory.h"
#include "colorid.h"
#include "fontid.h"
#include "updatemanager.h"
#include "session.h"
#include "compmanager.h"

#include <sstream>


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


// row -> path
#define GET_PATH( row ) m_treestore->get_path( row )


// ポップアップメニュー表示
#define SHOW_POPUPMENU(slot) do{\
std::string url = path2url( m_path_selected ); \
if( ! m_path_selected.empty() && url.empty() ) url = "dummy_url"; \
show_popupmenu( url, slot ); \
}while(0)



using namespace BBSLIST;

BBSListViewBase::BBSListViewBase( const std::string& url,const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_treeview( CONFIG::get_fontname( FONT_BBS ), COLOR_CHAR_BBS, COLOR_BACK_BBS, COLOR_BACK_BBS_EVEN ),
      m_ready_tree( false ),
      m_jump_y( -1 ),
      m_dnd_counter( 0 ),
      m_search_invert( 0 ),
      m_cancel_focus( 0 ),
      m_expand_collapse( 0 ),
      m_cancel_expand( false ),
      m_expanding( 0 )
{
    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

    pack_start( m_scrwin );
    show_all_children();    

    m_treestore = Gtk::TreeStore::create( m_columns ); 

#if GTKMMVER <= 260
    // gtkmm26以下にはunset_model()が無いのでここでset_model()しておく
    m_treeview.set_model( m_treestore );
    m_treeview.set_headers_visible( false );
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
    // 列の登録
    m_treeview.append_column( *create_column() );
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


    m_treeview.sig_drag_begin().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_drag_begin ) );
    m_treeview.sig_drag_motion().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_drag_motion ) );
    m_treeview.sig_drag_drop().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_drag_drop ) );
    m_treeview.sig_drag_end().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_drag_end ) );

    // D&Dマネージャのシグナルをコネクト
    CORE::get_dnd_manager()->sig_dnd_begin().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_receive_dnd_begin ) );
    CORE::get_dnd_manager()->sig_dnd_end().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_receive_dnd_end ) );

    ///////////////////
    
    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "OpenTab", "タブで開く(_T)"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_tab ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く(_W)"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "お気に入りに追加(_F)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_append_favorite ) );
    action_group()->add( Gtk::Action::create( "NewDir", "新規ディレクトリ(_N)"), sigc::mem_fun( *this, &BBSListViewBase::slot_newdir ) );
    action_group()->add( Gtk::Action::create( "NewCom", "コメント挿入(_I)"), sigc::mem_fun( *this, &BBSListViewBase::slot_newcomment ) );
    action_group()->add( Gtk::Action::create( "Rename", "名前変更(_R)"), sigc::mem_fun( *this, &BBSListViewBase::slot_rename ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "Delete", "お気に入りから削除する(_D)"), sigc::mem_fun( *this, &BBSListViewBase::delete_view ) );
    action_group()->add( Gtk::Action::create( "OpenRows", "選択した行を開く(_O)"), sigc::mem_fun( *this, &BBSListViewBase::open_selected_rows ) );

    action_group()->add( Gtk::Action::create( "CheckUpdateRows", "更新チェックのみ(_H)"), sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_selected_rows ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateOpenRows", "更新されたスレをタブで開く(_E)"),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_open_selected_rows ) );

    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー(_U)"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", "タイトルとURLをコピー(_L)"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "SelectDir", "全て選択(_A)"), sigc::mem_fun( *this, &BBSListViewBase::slot_select_all_dir ) );

    action_group()->add( Gtk::Action::create( "CheckUpdate_Menu", "更新チェック(_M)" ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateDir", "更新チェックのみ(_R)"), sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_dir ) );
    action_group()->add( Gtk::Action::create( "CheckUpdateOpenDir", "更新されたスレをタブで開く(_A)"),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_open_dir ) );
    action_group()->add( Gtk::Action::create( "CancelCheckUpdate", "キャンセル(_C)" ),
                         sigc::mem_fun( *this, &BBSListViewBase::slot_cancel_check_update ) );

    action_group()->add( Gtk::Action::create( "SearchCacheBoard", "キャッシュ内ログ検索(_S)"), sigc::mem_fun( *this, &BBSListViewBase::slot_search_cache_board ) );

    action_group()->add( Gtk::Action::create( "PreferenceArticle", "スレのプロパティ(_P)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_article ) );
    action_group()->add( Gtk::Action::create( "PreferenceBoard", "板のプロパティ(_B)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_board ) );
    action_group()->add( Gtk::Action::create( "PreferenceImage", "画像のプロパティ(_M)..."), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_image ) );


    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

    // ポップアップメニューのレイアウト
    Glib::ustring str_ui = 

    "<ui>"

    // 通常
    "<popup name='popup_menu'>"
    "<menuitem action='OpenTab'/>"
    "<menuitem action='OpenBrowser'/>"
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"
    "<separator/>"
    "<menuitem action='SearchCacheBoard'/>"
    "<separator/>"
    "<menuitem action='PreferenceBoard'/>"
    "</popup>"


    // 通常 + 複数
    "<popup name='popup_menu_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"
    "</popup>"



    // 通常ディレクトリメニュー
    "<popup name='popup_menu_dir'>"
    "<menuitem action='SelectDir'/>"
    "</popup>"

    // お気に入り
    "<popup name='popup_menu_favorite'>"
    "<menuitem action='OpenTab'/>"
    "<menuitem action='OpenBrowser'/>"
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='SearchCacheBoard'/>"
    "<separator/>"
    "<menuitem action='PreferenceArticle'/>"
    "<menuitem action='PreferenceBoard'/>"
    "<menuitem action='PreferenceImage'/>"
    "</popup>"

    // お気に入り+複数選択
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


    // お気に入り+何もないところをクリック
    "<popup name='popup_menu_favorite_space'>"
    "<menuitem action='NewDir'/>"
    "</popup>"


    // お気に入りディレクトリメニュー
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

    // お気に入りコメントメニュー
    "<popup name='popup_menu_favorite_com'>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    // 選択
    "<popup name='popup_menu_select'>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='SearchCacheBoard'/>"
    "<separator/>"
    "<menuitem action='PreferenceArticle'/>"
    "<menuitem action='PreferenceBoard'/>"
    "<menuitem action='PreferenceImage'/>"
    "</popup>"

    "</ui>";

    ui_manager()->add_ui_from_string( str_ui );

    // ポップアップメニューにキーアクセレータを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_dir" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_mul" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_space" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_dir" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_com" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_select" ) );
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
}


SKELETON::Admin* BBSListViewBase::get_admin()
{
    return BBSLIST::get_admin();
}


//
// コマンド
//
bool BBSListViewBase::set_command( const std::string& command, const std::string& arg )
{
    if( command == "append_item" ) append_item();
    else if( command == "save_xml" ) save_xml( false );
    else if( command == "toggle_icon" ) toggle_icon( arg );

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

    // D&D 中に画面の上か下の方にある場合はスクロールさせる
    if( m_treeview.reorderable() ){

        ++m_dnd_counter;
        if( m_dnd_counter >= 250 / TIMER_TIMEOUT ){

            m_dnd_counter = 0;

            if( CORE::DND_Now_dnd() ){

                Gtk::TreeModel::Path path = m_treeview.get_path_under_mouse();
                Gtk::Adjustment* adjust = m_treeview.get_vadjustment();

                if( m_treeview.get_row( path ) && adjust ){

                    int height = m_treeview.get_height();
                    int step = (int)adjust->get_step_increment() / 2;
                    int val = -1;
                    int x,y;
                    m_treeview.get_pointer( x, y );

                    if( y < step * 2 ){
                        val = MAX( 0, (int)adjust->get_value() - step );
                    }
                    else if( y > height - step * 2 ){
                        val = MIN( (int)adjust->get_value() + step, (int)( adjust->get_upper() - adjust->get_page_size() ) );
                    }

                    if( val != -1 ){
                        adjust->set_value( val );
                        path = m_treeview.get_path_under_mouse();
                        slot_drag_motion( path );
                    }
                }
            }
        }
    }
}




//
// 再描画
//
void BBSListViewBase::relayout()
{
    m_treeview.init_color( COLOR_CHAR_BBS, COLOR_BACK_BBS, COLOR_BACK_BBS_EVEN );
    m_treeview.init_font( CONFIG::get_fontname( FONT_BBS ) );
}


//
// フォーカス
//
void BBSListViewBase::focus_view()
{
    // 一回キャンセル
    if( m_cancel_focus ){
        m_cancel_focus = false;
        return;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::focus_view url = " << get_url() << std::endl;
#endif 

    m_treeview.grab_focus();

    // コンボボックス切り替え
    if( get_url() == URL_BBSLISTVIEW ) BBSLIST::get_admin()->set_command( "switch_combo_bbslist" );
    else if( get_url() == URL_FAVORITEVIEW ) BBSLIST::get_admin()->set_command( "switch_combo_favorite" );
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
// 内容更新
//
// URLを新しいアドレスに変更するだけ
//
void BBSListViewBase::update_item( const std::string& )
{
    update_urls();
}


//
// viewの操作
//
void  BBSListViewBase::operate_view( const int& control )
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
                if( path2type( path ) == TYPE_DIR && CONFIG::get_buttonconfig()->is_toggled_tab_button() ) open_tab = false;
                open_row( path, open_tab );
            }
            break;

        case CONTROL::OpenBoardButton:
            if( ! m_path_selected.empty() ){
                open_tab = false;
                // pathがディレクトリでタブで開くボタンを入れ替えている時は更新チェックにする
                if( path2type( path ) == TYPE_DIR && CONFIG::get_buttonconfig()->is_toggled_tab_button() ) open_tab = true;
                open_row( path, open_tab );
            }
            break;

        case CONTROL::OpenBoardTab:
            open_tab = true;
            // pathがディレクトリでタブで開くキーを入れ替えている時はディレクトリ開閉にする
            if( path2type( path ) == TYPE_DIR && CONFIG::get_keyconfig()->is_toggled_tab_key() ) open_tab = false;
            open_row( path, open_tab );
            break;

        case CONTROL::OpenBoard:
            open_tab = false;
            // pathがディレクトリでタブで開くキーを入れ替えている時は更新チェックにする
            if( path2type( path ) == TYPE_DIR && CONFIG::get_keyconfig()->is_toggled_tab_key() ) open_tab = true;
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

        case CONTROL::Quit:
            close_view();
            break;

        case CONTROL::Reload:
            reload();
            break;
            
        case CONTROL::Delete:
        {
            SKELETON::MsgDiag mdiag( NULL, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
            delete_view();
            break;
        }

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
            m_search_invert = false;
            BBSLIST::get_admin()->set_command( "focus_toolbar_search" );
            break;

        case CONTROL::SearchInvert:
            m_search_invert = true;
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
    }
}



//
// ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
//
// SKELETON::View::show_popupmenu() を参照すること
//
void BBSListViewBase::activate_act_before_popupmenu( const std::string& url )
{
    Glib::RefPtr< Gtk::Action > act_search, act_board, act_article, act_image, act_opentab;
    act_search = action_group()->get_action( "SearchCacheBoard" );
    act_board = action_group()->get_action( "PreferenceBoard" );
    act_article = action_group()->get_action( "PreferenceArticle" );
    act_image = action_group()->get_action( "PreferenceImage" );
    act_opentab = action_group()->get_action( "OpenTab" );

    if( act_search ) act_search->set_sensitive( false );
    if( act_board ) act_board->set_sensitive( false );
    if( act_article ) act_article->set_sensitive( false );
    if( act_image ) act_image->set_sensitive( false );
    if( act_opentab ) act_opentab->set_sensitive( true );

    int type = path2type( m_path_selected );
    switch( type ){

        case TYPE_BOARD:
            if( act_search ) act_search->set_sensitive( true );
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
    m_treeview.set_model( m_treestore );
    m_treeview.set_headers_visible( false );

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

    BBSLIST::get_admin()->set_command( "switch_admin" );

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
        Glib::ustring subject = row[ m_columns.m_col_name ];
        Glib::ustring url = row[ m_columns.m_col_url ];
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
    // セルの文字を編集中なら何もしない
    if( m_ren_text->property_editable() ) return true;

    int key = get_control().key_press( event );

    // キー入力でboardを開くとkey_pressイベントがboadviewに送られて
    // 一番上のスレが開くので、open_row() は slot_key_release() で処理する
    if( key == CONTROL::OpenBoard ) return true;
    if( key == CONTROL::OpenBoardTab ) return true;

    operate_view( key );
    return true;
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

    // セルの文字を編集中なら何もしない
    if( m_ren_text->property_editable() ) return true;

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



// 共通バッファに path をセット
void BBSListViewBase::set_info_to_sharedbuffer( Gtk::TreePath& path )
{
    CORE::DATA_INFO info;
    info.type = path2type( path );
    info.url = path2url( path );
    info.name = path2name( path );
    CORE::SBUF_append( info );
#ifdef _DEBUG    
    std::cout << "append " << info.name << std::endl;
#endif
}



//
// 列をお気に入りに追加
//
void BBSListViewBase::slot_append_favorite()
{
    // 共有バッファにデータ登録
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    if( list_it.size() ){

        CORE::SBUF_clear_info();

        std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
        for( ; it != list_it.end(); ++it ){

            Gtk::TreeModel::Row row = *( *it );
            Gtk::TreePath path = GET_PATH( row );

            // サブディレクトリの場合は中身もコピー
            // とりあえず再帰なしで一階層のみ
            if( is_dir( path ) ){

                set_info_to_sharedbuffer( path );
                path.down();

                while( m_treeview.get_row( path ) ){

                    set_info_to_sharedbuffer( path );
                    path.next();
                }

                CORE::DATA_INFO info;
                info.type = TYPE_DIR_END;
                CORE::SBUF_append( info );
            }
            else set_info_to_sharedbuffer( path );
        }

        CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
    }
}


//
// メニューでディレクトリを作るを選択
//
void BBSListViewBase::slot_newdir()
{
    // これは m_path_selected が空でも実行する

    Gtk::TreeModel::Path path = append_row( std::string(), "新規ディレクトリ", TYPE_DIR, m_path_selected, true );
    m_treeview.set_cursor( path );
    show_status();
    m_path_selected = path;
    slot_rename();
}



//
// メニューでコメント挿入を選択
//
void BBSListViewBase::slot_newcomment()
{
    if( m_path_selected.empty() ) return;

    Gtk::TreeModel::Path path = append_row( std::string(), "コメント", TYPE_COMMENT, m_path_selected, true );
    m_treeview.set_cursor( path );
    m_path_selected = path;
    slot_rename();
}



//
// 名前変更
//
void BBSListViewBase::slot_rename()
{
    if( m_path_selected.empty() ) return;

#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_rename\n";
#endif

    // edit可 slot_ren_text_on_edited() と slot_ren_text_on_canceled で false にする
    m_ren_text->property_editable() = true;
    m_treeview.set_cursor( m_path_selected, *m_treeview.get_column( COL_NAME ), true );

    // メニューが消えるとfocus_viewが呼ばれて名前変更モードが終了するのでfocus_viewをキャンセルする
    m_cancel_focus = true;
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
    if( is_dir( path ) ){

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
// ディレクトリ内を全選択
//
void BBSListViewBase::select_all_dir( Gtk::TreeModel::Path path )
{
    if( is_dir( path ) ){

        m_treeview.get_selection()->select( path );
        path.down();

        while( m_treeview.get_row( path ) ){

            m_treeview.get_selection()->select( path );
            select_all_dir( path );
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

    select_all_dir( m_path_selected );
}



//
// 全選択
//
void BBSListViewBase::slot_select_all()
{
    SKELETON::MsgDiag mdiag( NULL, "全ての行を選択しますか？\n\n(注意) ディレクトリは全て展開されます。", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    mdiag.set_default_response( Gtk::RESPONSE_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    Gtk::TreeModel::Children child = m_treestore->children();
    Gtk::TreeModel::Children::iterator it = child.begin();
    for( ; it != child.end() ; ++it ) expand_all_dir( m_treestore->get_path( *it ) );

    it = child.begin();
    m_treeview.scroll_to_row( m_treestore->get_path( *it ), 0 );
    for( ; it != child.end() ; ++it ){
        m_treeview.get_selection()->select( *it );
        select_all_dir( m_treestore->get_path( *it ) );
    }
}



//
// ディレクトリ内を全更新チェック
//
// 呼び出した後に CORE::get_checkupdate_manager()->run() を実行すること
//
void BBSListViewBase::check_update_dir( Gtk::TreeModel::Path path )
{
    if( path.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::check_update_dir path = " << path.to_string() << std::endl;
#endif

    if( is_dir( path ) ){

        path.down();

        while( m_treeview.get_row( path ) ){

            int type = path2type( path );
            std::string url = path2url( path );

            if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ) );
            else if( type == TYPE_DIR ) check_update_dir( path );

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
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_check_check_update_dir path = " << m_path_selected.to_string() << std::endl;
#endif

    check_update_dir( m_path_selected );
    CORE::get_checkupdate_manager()->run( false );
}

// 全更新チェックして開く
void BBSListViewBase::slot_check_update_open_dir()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    if( m_path_selected.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_check_check_update_open_dir path = " << m_path_selected.to_string() << std::endl;
#endif

    check_update_dir( m_path_selected );
    CORE::get_checkupdate_manager()->run( true );
}


//
// ルート以下を全て更新チェック( 再帰用 )
//
// 呼び出した後に CORE::get_checkupdate_manager()->run() を実行すること
//
void BBSListViewBase::check_update_root( const Gtk::TreeModel::Children& children )
{
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreeModel::Row row = *it;

        const int type = row2type( row );
        const std::string url = row2url( row );

        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ) );
        else if( type == TYPE_DIR ) check_update_root( row.children() );

        ++it;
    }
}


//
// ルート以下を全て更新チェック( command 呼び出し用 )
//
// tab_open はタブで開くか否か
//
void BBSListViewBase::check_update_root( const bool tab_open )
{
    check_update_root( m_treestore->children() );

    CORE::get_checkupdate_manager()->run( tab_open );
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
// 板プロパティ表示
//
void BBSListViewBase::slot_preferences_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARD, DBTREE::url_subject( url ) );
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLE, DBTREE::url_dat( url ) );
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_IMAGE, url );
    pref->run();
    delete pref;
}



//
// フォルダを開いた時に呼ばれる
//
void BBSListViewBase::slot_row_exp( const Gtk::TreeModel::iterator&, const Gtk::TreeModel::Path& path )
{
    if( m_cancel_expand ) return;

    // 他のフォルダを全て閉じる
    if( CONFIG::get_open_one_category() && m_expand_collapse ){

        if( m_expanding ) return;
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
// 名前を変更したときにCellRendererTextから呼ばれるslot
//
void BBSListViewBase::slot_ren_text_on_edited( const Glib::ustring& path, const Glib::ustring& text )
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_ren_text_on_edited\n";
#endif

    Gtk::TreeModel::Row row = m_treeview.get_row( Gtk::TreePath( path ) );
    if( row ) row[ m_columns.m_col_name ] = text;

    m_ren_text->property_editable() = false;
}


//
// 名前をキャンセルしたときにCellRendererTextから呼ばれるslot
//
void BBSListViewBase::slot_ren_text_on_canceled()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_ren_text_on_canceld\n";
#endif

    m_ren_text->property_editable() = false;
}


//
// このビューからD&Dを開始したときにtreeviewから呼ばれる
//
void BBSListViewBase::slot_drag_begin()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_drag_begin\n";
#endif

    CORE::DND_Begin( get_url() );
}


//
// D&Dマネージャから D&D 開始シグナルを受けた時に呼ばれる
//
void BBSListViewBase::slot_receive_dnd_begin()
{}



//
// D&D中にtreeviewから呼ばれる
//
void BBSListViewBase::slot_drag_motion( Gtk::TreeModel::Path path )
{
    if( !m_treeview.get_row( path ) ) return;
    
    draw_underline( m_drag_path_uline, false );

    // 移動先に下線を引く
    int cell_x, cell_y, cell_w, cell_h;
    m_treeview.get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );

    // 真ん中より上の場合
    if( cell_y < cell_h / 2 ){

        Gtk::TreeModel::Path path_tmp = m_treeview.prev_path( path );
        if( m_treeview.get_row( path_tmp ) ) path = path_tmp;

        if( ! is_dir( path ) ) draw_underline( path, true );

    }
    else draw_underline( path, true );

    m_drag_path_uline = path;

#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_drag_motion = " << path.to_string() << std::endl;
#endif
}



//
// D&Dでドロップされたときにtreeviewから呼ばれる
//
void BBSListViewBase::slot_drag_drop( Gtk::TreeModel::Path path )
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_drag_drop\n";
#endif

    bool after = true;
    draw_underline( m_drag_path_uline, false );

    // セル内の座標を見て真ん中より上だったら上に挿入
    if( m_treeview.get_row( path ) ){

        int cell_x, cell_y, cell_w, cell_h;
        m_treeview.get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );
        if( cell_y < cell_h / 2 ) after = false;

#ifdef _DEBUG    
        std::cout << "cell height = " << cell_h << " cell_y = " << cell_y << std::endl;
#endif
    }

    std::string url_from = CORE::DND_Url_from();

#ifdef _DEBUG
    std::cout << "path = " << path.to_string() << " after = " << after << " from " << url_from << std::endl;
#endif

    // 同じビュー内でディレクトリの移動
    if( url_from == get_url() ) move_selected_row( path, after );

    // 他のビューからD&Dされた
    else append_from_buffer( path, after, false );
}



//
// このビューからD&Dを開始した後にD&Dを終了するとtreeviewから呼ばれる
//
void BBSListViewBase::slot_drag_end()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::slot_drag_end\n";
#endif
    CORE::DND_End();

    draw_underline( m_drag_path_uline, false );
}



//
// D&Dマネージャから D&D 終了シグナルを受けたときに呼ばれる
//
void BBSListViewBase::slot_receive_dnd_end()
{
    if( !m_treeview.reorderable() ) return;

    draw_underline( m_drag_path_uline, false );
}




//
// 列の作成
//
// Gtk::mange　してるのでdeleteしなくてもよい
//
Gtk::TreeViewColumn* BBSListViewBase::create_column()
{
    Gtk::TreeViewColumn* col = Gtk::manage( new Gtk::TreeViewColumn( "name" ) );
    col->pack_start( m_columns.m_col_image, Gtk::PACK_SHRINK );

    m_ren_text = Gtk::manage( new Gtk::CellRendererText() );
    m_ren_text->signal_edited().connect( sigc::mem_fun( *this, &BBSListViewBase::slot_ren_text_on_edited ) );
    m_ren_text->signal_editing_canceled().connect( sigc::mem_fun( *this, &BBSListViewBase::slot_ren_text_on_canceled ) );
    m_ren_text->property_underline() = Pango::UNDERLINE_SINGLE;

    // 行間スペース
    m_ren_text->property_ypad() = CONFIG::get_tree_ypad();

    col->pack_start( *m_ren_text, true );
    col->add_attribute( *m_ren_text, "text", COL_NAME );
    col->add_attribute( *m_ren_text, "underline", COL_UNDERLINE );
    col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );

    col->set_cell_data_func( *col->get_first_cell_renderer(), sigc::mem_fun( m_treeview, &SKELETON::JDTreeView::slot_cell_data ) );    
    col->set_cell_data_func( *m_ren_text, sigc::mem_fun( m_treeview, &SKELETON::JDTreeView::slot_cell_data ) );    

    return col;
}




//
// 選択した行を開く 
//
const bool BBSListViewBase::open_row( Gtk::TreePath& path, const bool tab )
{
    if( !m_treeview.get_row( path ) ) return false;

    std::string str_tab = "false";
    if( tab ) str_tab = "true";

    Glib::ustring url = path2url( path );
    int type = path2type( path );

    if( type != TYPE_DIR && url.empty() ) return false;

    switch( type ){

        case TYPE_BOARD:
            CORE::core_set_command( "open_board", DBTREE::url_subject( url ), str_tab, "" );
            break;

        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            CORE::core_set_command( "open_article", DBTREE::url_dat( url ), str_tab, "" );
            break;

        case TYPE_IMAGE:

            if( DBIMG::get_abone( url )){
                SKELETON::MsgDiag mdiag( NULL, "あぼ〜んされています" );
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
void BBSListViewBase::checkupdate_selected_rows()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){

        Gtk::TreeModel::Row row = *( *it );
        Gtk::TreePath path = GET_PATH( row );

        int type = path2type( path );
        std::string url = path2url( path );

        if( type == TYPE_THREAD || TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ) );            
    }
}


void BBSListViewBase::slot_checkupdate_selected_rows()
{
    checkupdate_selected_rows();
    CORE::get_checkupdate_manager()->run( false );
}


void BBSListViewBase::slot_checkupdate_open_selected_rows()
{
    checkupdate_selected_rows();
    CORE::get_checkupdate_manager()->run( true );
}


//
// path -> url 変換
//
Glib::ustring BBSListViewBase::path2url( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return Glib::ustring();

    Glib::ustring url =  row[ m_columns.m_col_url ];
    if( url.empty() ) return url;

    // 移転があったら url を最新のものに変換しておく
    int type = path2type( path );
    switch( type ){

        case TYPE_BOARD:
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

    Glib::ustring url =  row[ m_columns.m_col_url ];
    if( url.empty() ) return url;

    // 移転があったら url を最新のものに変換しておく
    int type = row2type( row );
    switch( type ){

        case TYPE_BOARD:
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
    return row[ m_columns.m_col_name ];
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
// ディレクトリかどうかの判定
//
bool BBSListViewBase::is_dir( Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = ( *it );
    if( row[ m_columns.m_type ] == TYPE_DIR ) return true;
    return false;
}

bool BBSListViewBase::is_dir( const Gtk::TreePath& path )
{
    Gtk::TreeModel::iterator it = m_treestore->get_iter( path );
    return is_dir( it );
}




//
// 行追加
//
// after = false ならpath_dest の前に追加する( デフォルト after = true )
// path_dest がNULLなら一番最後に作る
// path_dest がディレクトリであり、かつ subdir = true なら path_dest の下に追加。
// path_dest がディレクトリでない、または subdir = falseなら path_dest の後に追加
// 戻り値は追加した行のpath
//
Gtk::TreeModel::Path BBSListViewBase::append_row( const std::string& url, const std::string& name, int type,
                                                  Gtk::TreeModel::Path path_dest, bool subdir, bool after  )
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::append_row " << url << " " << name << std::endl;
#endif    
    Gtk::TreeModel::Row row_new;

    // 一番下に追加
    if( ! m_treeview.get_row( path_dest ) ) row_new = *( m_treestore->append() );
    else{

        Gtk::TreeModel::Row row_dest = m_treeview.get_row( path_dest );
        if( row_dest )
        {
            // path_destがディレクトリなら下に追加してディレクトリを開く
            if( subdir && after && row_dest[ m_columns.m_type ] == TYPE_DIR ){
                row_new = *( m_treestore->prepend( row_dest.children() ) );
                m_treeview.expand_row( path_dest, false );
            }

            // destの下に追加
            else if( after ) row_new = *( m_treestore->insert_after( row_dest ) );

            // destの前に追加
            else row_new = *( m_treestore->insert( row_dest ) );
        }
    }
    m_columns.setup_row( row_new, url, name, type );
    return GET_PATH( row_new );
}


//
// 行の再帰コピー
//
// subdir = true　かつdestがディレクトリならサブディレクトリをその下に作ってそこにコピーする。false ならdestの後にコピー
// after = false の場合はdestの前に挿入する
// dest が NULL なら一番下にappend
//
// 成功したら dest にコピーした行のiteratorが入る
//
bool BBSListViewBase::copy_row( Gtk::TreeModel::iterator& src, Gtk::TreeModel::iterator& dest, bool subdir, bool after )
{
    if( !src ) return false;
    if( dest && src == dest ) return false;

    Gtk::TreeModel::iterator it_new;
    bool src_is_dir = false, dest_is_dir = false;

    Gtk::TreeModel::Row row_src = ( *src );
    Gtk::TreeModel::Row row_dest = ( *dest );

    Glib::ustring url = row_src[ m_columns.m_col_url ];
    Glib::ustring name = row_src[ m_columns.m_col_name ];
    int type = row_src[ m_columns.m_type ];

    if( type == TYPE_DIR ) src_is_dir = true;
    if( row_dest && row_dest[ m_columns.m_type ] == TYPE_DIR ) dest_is_dir = true;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::copy_row " << name << std::endl;
    if( src_is_dir ) std::cout << "src is directory\n";
    if( dest_is_dir ) std::cout << "dest is directory\n";
#endif    

    // destがNULLなら一番下に追加
    if( ! dest  ) it_new = m_treestore->append();

    // destの下にサブディレクトリ作成
    else if( subdir && after && dest_is_dir ){
        it_new = m_treestore->prepend( row_dest.children() );
    }

    // destの後に追加
    else if( after ) it_new = m_treestore->insert_after( dest );

    // destの前に追加
    else it_new = m_treestore->insert( dest );

    Gtk::TreeModel::Row row_tmp = *( it_new );
    m_columns.setup_row( row_tmp, url, name, type );

    // srcがdirならサブディレクトリ内の行も再帰的にコピー
    if( src_is_dir ){
        Gtk::TreeModel::iterator it_tmp = it_new;
        Gtk::TreeModel::iterator it_child = row_src.children().begin();
        bool subdir_tmp = true;
        for( ; it_child != row_src.children().end(); ++it_child ){
            copy_row( it_child, it_tmp, subdir_tmp );
            subdir_tmp = false;
        }
    }

    dest = it_new;
    return true;
}



//
// 選択した行をpathの所にまとめて移動
//
// after = true なら path の後に移動。falseなら前
//
void BBSListViewBase::move_selected_row( const Gtk::TreePath& path, bool after )
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::vector< bool > vec_cancel;
    vec_cancel.resize( list_it.size() );
    std::fill( vec_cancel.begin(), vec_cancel.end(), false );

    // 移動できるかチェック
    std::list< Gtk::TreeModel::iterator >::iterator it_src = list_it.begin();
    for( int i = 0 ; it_src != list_it.end(); ++i, ++it_src ){

        if( vec_cancel[ i ] ) continue;

        Gtk::TreeModel::Path path_src = GET_PATH( ( *it_src ) );

        // 移動先と送り側が同じならキャンセル
        if( path_src.to_string() == path.to_string() ) return;

        // 移動先がサブディレクトリに含まれないかチェック
        if( is_dir( ( *it_src ) ) ){
            if( path.to_string().find( path_src.to_string() ) != Glib::ustring::npos ){
                SKELETON::MsgDiag mdiag( NULL, "移動先は送り側のディレクトリのサブディレクトリです", false, Gtk::MESSAGE_ERROR );
                mdiag.run();
                return;
            }
        }

        // path_srcのサブディレクトリ内の行も範囲選択内に含まれていたらその行の移動をキャンセル
        std::list< Gtk::TreeModel::iterator >::iterator it_tmp = it_src;
        ++it_tmp;
        for( int i2 = 1 ; it_tmp != list_it.end(); ++i2, ++it_tmp ){

            Gtk::TreeModel::Path path_tmp = GET_PATH( ( *it_tmp ) );
            if( path_tmp.to_string().find( path_src.to_string() ) != Glib::ustring::npos ){
                vec_cancel[ i + i2 ] = true;
            }
        }
    }

    // 移動開始

    std::list< Gtk::TreeModel::Row > list_destrow;

    Gtk::TreeModel::iterator it_dest = m_treestore->get_iter( path );
    Gtk::TreeModel::iterator it_dest_bkup = it_dest;
    bool after_bkup = after;
    bool subdir = after;
    it_src = list_it.begin();
    for( int i = 0 ; it_src != list_it.end(); ++i, ++it_src ){

        if( vec_cancel[ i ] ) continue;

        // コピーして削除
        if( copy_row( ( *it_src ), it_dest, subdir, after ) ) m_treestore->erase( ( *it_src ) );
        subdir = false;
        after = true;
        list_destrow.push_back( *it_dest );
    }

    // 移動先がディレクトリなら開く
    if( is_dir( it_dest_bkup ) && after_bkup ) m_treeview.expand_row( GET_PATH( *it_dest_bkup ), false );

    // 範囲選択
    m_treeview.get_selection()->unselect_all();
    std::list< Gtk::TreeModel::Row >::iterator it_destrow = list_destrow.begin();
    for( ; it_destrow != list_destrow.end(); ++it_destrow ){

        Gtk::TreeModel::Row row_tmp = ( *it_destrow );
        m_treeview.get_selection()->select( row_tmp );

        if( row_tmp[ m_columns.m_type ] == TYPE_DIR ){
            m_treeview.expand_row( GET_PATH( row_tmp ), false );
            select_all_dir( GET_PATH( row_tmp ) );
        }
    }
}



//
// 下線を引く
//
void BBSListViewBase::draw_underline( const Gtk::TreePath& path, bool draw )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return;

    row[ m_columns.m_underline ] = draw;
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
// 共有バッファからデータを取り出して pathの所に追加
//
// after = false ならpathの前に追加
// scroll = true なら追加した行にスクロールする
//
void BBSListViewBase::append_from_buffer( Gtk::TreeModel::Path path, bool after, bool scroll )
{
    Gtk::TreeModel::Path path_top = Gtk::TreeModel::Path();

#ifdef _DEBUG
    std::cout << "BBSListViewBase::append_from_buffer\n";
#endif

    m_treeview.get_selection()->unselect_all();

    bool subdir = true;
    std::list< CORE::DATA_INFO > infolist = CORE::SBUF_infolist();
    std::list< CORE::DATA_INFO >::iterator it = infolist.begin();
    for( ; it != infolist.end() ; ++it ){

        CORE::DATA_INFO& info = ( *it );
#ifdef _DEBUG    
        std::cout << "append name = " << info.name << std::endl;
        std::cout << "url " << info.url << std::endl;
        std::cout << "type " << info.type << std::endl;
#endif

        if( info.type != TYPE_DIR_END ){

            int type = info.type;

            // スレの時はdat落ちしているか更新しているか確認
            if( type == TYPE_THREAD ){

                if( DBTREE::article_status( info.url ) & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;
                if( DBTREE::article_status( info.url ) & STATUS_OLD ) type = TYPE_THREAD_OLD;

#ifdef _DEBUG
                std::cout << "-> type =  " << type << std::endl;
#endif

                // ブックマークセット
                DBTREE::set_bookmarked_thread( info.url, true );
            }

            path = append_row( info.url, info.name, type, path, subdir, after );
            if( m_treeview.get_row( path ) ){
                if( ! m_treeview.get_row( path_top ) ){
                    path_top = path;
                    m_treeview.set_cursor( path_top );
                }
                m_treeview.get_selection()->select( path );
                after = true;
            }
        }

        if( info.type == TYPE_DIR ) subdir = true;
        else{

            subdir = false;
            if( info.type == TYPE_DIR_END ) path.up();
            else if( info.type == TYPE_IMAGE ) DBIMG::set_protect( info.url, true );
        }
    }

    // 追加した行にスクロール
    if( scroll && m_treeview.get_row( path_top ) ){
        m_treeview.scroll_to_row( path_top, 0.1 );
    }
}



//
// 選択した行をまとめて削除
//
void BBSListViewBase::delete_selected_rows()
{
    // iterator 取得
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();

    if( ! list_it.size() ) return;

    // ディレクトリが無いか確認
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){

        if( is_dir( (*it ) ) ){
            SKELETON::MsgDiag mdiag( NULL, "ディレクトリを削除するとディレクトリ内の行も全て削除されます。削除しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
            break;
        }
    }

    // カーソルを一番最後の行の次の行に移動する
    it = list_it.end();
    --it;

    // ディレクトリを閉じないとそのディレクトリの先頭の列が next になり、ディレクトリが削除されるため
    // キーボードフォーカスが外れる
    if( is_dir( *it ) ) m_treeview.collapse_row( GET_PATH( *it ) ); 
    Gtk::TreePath next = m_treeview.next_path( GET_PATH( *it ), true );

    // もしnextが存在しなかったら全ての行を削除してから一番下に移動
    bool gotobottom = ( ! m_treeview.get_row( next ) );
    if( ! gotobottom ) m_treeview.set_cursor( next );

#ifdef _DEBUG
    std::cout << " BBSListViewBase::delete_selected_rows : ";
    std::cout << GET_PATH( *it ).to_string() << " -> " << next.to_string() << std::endl;
#endif

    // まとめて削除
    // ディレクトリ内の行を同時に選択している場合があるので後から消す
    it = list_it.end();
    while( it != list_it.begin() ) m_treestore->erase( ( *(--it) ) );

    if( gotobottom ) m_treeview.goto_bottom();
}



//
// 全てのツリーに m_columns.m_expand の値をセットする( tree2xml()で使用 )
//
void BBSListViewBase::set_expanded_row( const Gtk::TreeModel::Children& children )
{
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreePath path = m_treestore->get_path( *it );

        // ツリーが開いているか
        if( m_treeview.row_expanded( path ) ) (*it)[ m_columns.m_expand ] = true;
        else (*it)[ m_columns.m_expand ] = false;

        // 再帰
        if( ! (*it)->children().empty() ) set_expanded_row( (*it)->children() );

        ++it;
    }
}


//
// tree -> XML 変換
//
//
void BBSListViewBase::tree2xml( const std::string& root_name )
{
    if( ! m_ready_tree ) return;

    if( m_treestore->children().empty() )
    {
        m_document.clear();
        return;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::tree2xml\n";
#endif

    // 全てのツリーに row[ m_columns.expand ] の値をセットする
    set_expanded_row( m_treestore->children() );

    // m_treestore からノードツリーを作成
    m_document.init( m_treestore, root_name );

#ifdef _DEBUG
    std::cout << " ルートノード名=" << root_name;
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif

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
    root->setAttribute( "y", y );
    root->setAttribute( "path", path );
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
    m_treestore->clear();

#if GTKMMVER >= 280
    m_treeview.unset_model();
#endif

    // 新規に文字列からDOMノードツリーを作成する場合
    if( ! xml.empty() ) m_document.init( xml );

#ifdef _DEBUG
    std::cout << " ルートノード名=" << root_name;
    std::cout << " 子ノード数=" << m_document.childNodes().size() << std::endl;
#endif

    // 開いてるツリーの格納用
    std::list< Gtk::TreePath > list_path_expand;

    // Domノードから Gtk::TreeStore をセット
    m_document.set_treestore( m_treestore, root_name, list_path_expand );

#if GTKMMVER >= 280
    m_treeview.set_model( m_treestore );
    m_treeview.set_headers_visible( false );
#endif

    // ディレクトリオープン
    std::list< Gtk::TreePath >::iterator it_path = list_path_expand.begin();
    while( it_path != list_path_expand.end() )
    {
        m_treeview.expand_parents( *it_path );
        m_treeview.expand_row( *it_path, false );
        ++it_path;
    }

    // ルート要素を取り出す
    XML::Dom* root = m_document.get_root_element( root_name );

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

    m_ready_tree = true;
}



//
// 移転があったときに行に含まれるURlを変更する
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

            Glib::ustring url = row[ m_columns.m_col_url ];
            int type = row[ m_columns.m_type ];
            std::string url_new;

            switch( type ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_BOARD: // 板
                    url_new = DBTREE::is_board_moved( url );
                    if( ! url_new.empty() ){
                        url_new = DBTREE::url_boardbase( url );
#ifdef _DEBUG
                        std::cout << url << " -> " << url_new << std::endl;
#endif
                        row[ m_columns.m_col_url ] = url_new;
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
                        row[ m_columns.m_col_url ] = url_new;
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
// アイコン表示の切り替え
//
void BBSListViewBase::toggle_icon( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return; 
   
    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    std::string urldat = DBTREE::url_dat( url );
    std::string urlcgi = DBTREE::url_readcgi( url, 0, 0 );

    int type = TYPE_THREAD;
    int status = DBTREE::article_status( url );
    if( status & STATUS_OLD ) type = TYPE_THREAD_OLD;
    if( status & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;
    
#ifdef _DEBUG
    std::cout << "BBSListViewBase::toggle_icon url = " << url << " type = " << type << std::endl;
#endif

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            Glib::ustring url_row = row[ m_columns.m_col_url ];

            switch( row[ m_columns.m_type ] ){

                case TYPE_DIR: // サブディレクトリ
                    path.down();
                    break;

                case TYPE_THREAD:
                case TYPE_THREAD_UPDATE:
                case TYPE_THREAD_OLD:
                    if( urldat == url_row || urlcgi == url_row ){
                        row[ m_columns.m_type ] = type;
                        row[ m_columns.m_col_image ] = XML::get_icon( type );
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
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH, query );
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
    m_search_invert = true;
    exec_search();
}



// 後検索
void BBSListViewBase::down_search()
{
    m_search_invert = false;
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
// お気に入りにアイテム追加
//
// あらかじめ共有バッファにデータを入れておくこと
//
void BBSListViewBase::append_item()
{
    if( CORE::SBUF_size() == 0 ) return;
    
    SelectListDialog diag( get_url(), get_treestore() );
    if( diag.run() != Gtk::RESPONSE_OK ) return;
    append_from_buffer( diag.get_path(), true, true );
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

