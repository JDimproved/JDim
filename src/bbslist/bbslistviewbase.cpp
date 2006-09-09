// ライセンス: 最新のGPL

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "bbslistviewbase.h"
#include "bbslistadmin.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

#include "command.h"
#include "global.h"
#include "httpcode.h"
#include "controlid.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "viewfactory.h"
#include "xml.h"
#include "prefdiagfactory.h"

#include <sstream>
//#include <gtk/gtkversion.h> // GTK_CHECK_VERSION


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
SKELETON::View::show_popupmenu( url, slot ); \
}while(0)



using namespace BBSLIST;

BBSListViewBase::BBSListViewBase( const std::string& url,const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_treeview( CONFIG::get_fontname_tree(), CONFIG::get_color_back_tree() ),
      m_ready_tree( false ),
      m_jump_y( -1 ),
      m_dnd_counter( 0 ),
      m_search_invert( 0 ),
      m_cancel_focus( 0 ),
      m_expand_collapse( 0 ),
      m_expanding( 0 )
{
    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

    m_toolbar.m_entry_search.signal_activate().connect( sigc::mem_fun( *this, &BBSListViewBase::search ) );
    m_toolbar.m_button_up_search.signal_clicked().connect( sigc::mem_fun( *this, &BBSListViewBase::slot_push_up_search ) );
    m_toolbar.m_button_down_search.signal_clicked().connect( sigc::mem_fun( *this, &BBSListViewBase::slot_push_down_search ) );
    m_toolbar.m_entry_search.signal_operate().connect( sigc::mem_fun( *this, &BBSListViewBase::slot_entry_operate ) );

    pack_start( m_toolbar, Gtk::PACK_SHRINK );    
    pack_start( m_scrwin );
    show_all_children();    

    m_treestore = Gtk::TreeStore::create( m_columns ); 
    m_treeview.set_model( m_treestore );
    m_treeview.set_headers_visible( false );

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
    m_treeview.sig_motion().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_motion ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_release ) );

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
    action_group()->add( Gtk::Action::create( "OpenTab", "タブで開く"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_tab ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く"), sigc::mem_fun( *this, &BBSListViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "AppendFavorite"), sigc::mem_fun( *this, &BBSListViewBase::slot_append_favorite ) );
    action_group()->add( Gtk::Action::create( "NewDir", "新規ディレクトリ"), sigc::mem_fun( *this, &BBSListViewBase::slot_newdir ) );
    action_group()->add( Gtk::Action::create( "NewCom", "コメント挿入"), sigc::mem_fun( *this, &BBSListViewBase::slot_newcomment ) );
    action_group()->add( Gtk::Action::create( "Rename", "名前変更"), sigc::mem_fun( *this, &BBSListViewBase::slot_rename ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "Delete", "お気に入りから削除する"), sigc::mem_fun( *this, &BBSListViewBase::delete_view ) );
    action_group()->add( Gtk::Action::create( "OpenRows", "選択した行を開く"), sigc::mem_fun( *this, &BBSListViewBase::open_selected_rows ) );
    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", "タイトルとURLをコピー"), sigc::mem_fun( *this, &BBSListViewBase::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "Unselect", "選択解除"), sigc::mem_fun( *this, &BBSListViewBase::slot_unselect_all ) );
    action_group()->add( Gtk::Action::create( "PreferenceArticle", "スレのプロパティ"), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_article ) );
    action_group()->add( Gtk::Action::create( "PreferenceBoard", "板のプロパティ"), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_board ) );
    action_group()->add( Gtk::Action::create( "PreferenceImage", "画像のプロパティ"), sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_image ) );


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
    "<menuitem action='Unselect'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"
    "<separator/>"
    "<menuitem action='PreferenceBoard'/>"
    "</popup>"


    // 通常 + 複数
    "<popup name='popup_menu_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='Unselect'/>"
    "<separator/>"
    "<menuitem action='AppendFavorite'/>"
    "</popup>"


    // お気に入り
    "<popup name='popup_menu_favorite'>"
    "<menuitem action='OpenTab'/>"
    "<menuitem action='OpenBrowser'/>"
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='Unselect'/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewCom'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='PreferenceArticle'/>"
    "<menuitem action='PreferenceBoard'/>"
    "<menuitem action='PreferenceImage'/>"
    "</popup>"

    // お気に入り+複数選択
    "<popup name='popup_menu_favorite_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='Unselect'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"


    // お気に入り+何もないところをクリック
    "<popup name='popup_menu_favorite_space'>"
    "<menuitem action='NewDir'/>"
    "</popup>"


    // 選択
    "<popup name='popup_menu_select'>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='Unselect'/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    "</ui>";

    ui_manager()->add_ui_from_string( str_ui );

    // ポップアップメニューにキーアクセレータを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_mul" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_space" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_select" ) );
    CONTROL::set_menu_motion( popupmenu );

    // マウスジェスチャ可
    SKELETON::View::set_enable_mg( true );

    // コントロールモード設定
    SKELETON::View::get_control().set_mode( CONTROL::MODE_BBSLIST );
}



BBSListViewBase::~BBSListViewBase()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::~BBSListViewBase : " << get_url() << std::endl;
#endif
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
    m_treeview.init_color( CONFIG::get_color_back_tree() );
    m_treeview.init_font( CONFIG::get_fontname_tree() );
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
// viewの操作
//
void  BBSListViewBase::operate_view( const int& control )
{
    Gtk::TreePath path = m_treeview.get_current_path();
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

            // 展開 or 選択
        case CONTROL::OpenBoardTab:
            open_tab = true;
        case CONTROL::OpenBoard:

            if( m_treeview.get_row( path ) ){

                if( ! m_treeview.row_expanded( path ) ){
                    if( ! open_row( path, open_tab ) ){
                        m_treeview.expand_row( path, false );
                    }
                }
                else m_treeview.collapse_row( path );
            }
            break;

        case CONTROL::Right:

            if( m_treeview.get_row( path ) ){
                if( ! m_treeview.expand_row( path, false ) ) CORE::core_set_command( "switch_board" );
            }
            break;

        case CONTROL::Left:

            if( m_treeview.get_row( path ) ){
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

        case CONTROL::Reload:
            reload();
            break;
            
        case CONTROL::Delete:
        {
            Gtk::MessageDialog mdiag( "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() != Gtk::RESPONSE_OK ) return;
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
            m_toolbar.m_entry_search.grab_focus();
            break;

        case CONTROL::SearchInvert:
            m_search_invert = true;
            m_toolbar.m_entry_search.grab_focus();
            break;

        case CONTROL::SearchNext:
            slot_push_down_search();
            break;
    
        case CONTROL::SearchPrev:
            slot_push_up_search();
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
    Glib::RefPtr< Gtk::Action > act_board, act_article, act_image;
    act_board = action_group()->get_action( "PreferenceBoard" );
    act_article = action_group()->get_action( "PreferenceArticle" );
    act_image = action_group()->get_action( "PreferenceImage" );
    if( act_board ) act_board->set_sensitive( false );
    if( act_article ) act_article->set_sensitive( false );
    if( act_image ) act_image->set_sensitive( false );

    int type = path2type( m_path_selected );
    switch( type ){

        case TYPE_BOARD:
            if( act_board ) act_board->set_sensitive( true );
            break;

        case TYPE_THREAD:
            if( act_article ) act_article->set_sensitive( true );
            break;

        case TYPE_IMAGE:
            if( act_image ) act_image->set_sensitive( true );
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

    if( m_treestore->children().begin() ){

        Gtk::TreePath path = GET_PATH( *( m_treestore->children().begin() ) );
        if( m_treeview.get_row( path ) ){
            m_treeview.collapse_all();
            m_treeview.scroll_to_row( path, 0 );
            m_treeview.get_selection()->unselect_all();
        }
    }
}




//
// マウスボタン押した
//
bool BBSListViewBase::slot_button_press( GdkEventButton* event )
{
    // マウスジェスチャ
    SKELETON::View::get_control().MG_start( event );

    // ダブルクリック
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

    CORE::core_set_command( "switch_bbslist" );

    return true;
}


//
// マウスボタン離した
//
bool BBSListViewBase::slot_button_release( GdkEventButton* event )
{
    /// マウスジェスチャ
    int mg = SKELETON::View::get_control().MG_end( event );
    if( mg != CONTROL::None && enable_mg() ){
        operate_view( mg );
        return true;
    }

    show_status();

    m_path_selected = m_treeview.get_path_under_xy( (int)event->x, (int)event->y );

    // ダブルクリックの処理のため一時的にtypeを切替える
    GdkEventType type_copy = event->type;
    if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

    // 板を開く
    bool openboard = SKELETON::View::get_control().button_alloted( event, CONTROL::OpenBoardButton );
    bool openboardtab = SKELETON::View::get_control().button_alloted( event, CONTROL::OpenBoardTabButton );
    if( openboard || openboardtab ){
        if( m_treeview.get_row( m_path_selected ) ) open_row( m_path_selected, openboardtab );
    }
    // ポップアップメニューボタン
    else if( SKELETON::View::get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){

        SHOW_POPUPMENU( false );
    }

    event->type = type_copy;

    return true;
}




//
// マウス動かした
//
bool BBSListViewBase::slot_motion( GdkEventMotion* event )
{
    /// マウスジェスチャ
    SKELETON::View::get_control().MG_motion( event );

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

            if( DBIMG::is_loadable( url ) && DBIMG::get_code( url ) != HTTP_ERR ){

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
    operate_view( SKELETON::View::get_control().key_press( event ) );
    return true;
}


//
// キー上げた
//
bool BBSListViewBase::slot_key_release( GdkEventKey* event )
{
#ifdef _DEBUG
    guint key = event->keyval;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;

    std::cout << "BBSListViewBase::slot_key_release key = " << key << " ctrl = " << ctrl << " shift = " << shift << std::endl;
#endif
    
    return true;
}



//
// popupmenu でタブで開くを選択
//
void BBSListViewBase::slot_open_tab()
{
    open_row( m_path_selected, true );
}


//
// ブラウザで開く
//
void BBSListViewBase::slot_open_browser()
{
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
    Gtk::TreeModel::Path path = append_row( std::string(), "New Directory", TYPE_DIR, m_path_selected, true );
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
    Gtk::TreeModel::Path path = append_row( std::string(), "Comment", TYPE_COMMENT, m_path_selected, true );
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

    m_ren_text->property_editable() = true;     // edit可
    m_treeview.set_cursor( m_path_selected, *m_treeview.get_column( 0 ), true );
    // メニューが消えるとfocus_viewが呼ばれて名前変更モードが終了するのでfocus_viewをキャンセルする
    m_cancel_focus = true;
    m_ren_text->property_editable() = false;
}


//
// URLをコピー
//
void BBSListViewBase::slot_copy_url()
{
    if( m_path_selected.empty() ) return;

    std::string url = path2url( m_path_selected );
    COPYCLIP( url );
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

    COPYCLIP( ss.str() );
}


//
// 選択解除
//
void BBSListViewBase::slot_unselect_all()
{
    m_treeview.get_selection()->unselect_all();
}



//
// 板プロパティ表示
//
void BBSListViewBase::slot_preferences_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_BOARD, DBTREE::url_subject( url ) );
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_ARTICLE, DBTREE::url_dat( url ) );
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_IMAGE, url );
    pref->run();
    delete pref;
}



//
// フォルダを開いた時に呼ばれる
//
void BBSListViewBase::slot_row_exp( const Gtk::TreeModel::iterator&, const Gtk::TreeModel::Path& path )
{
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
    Gtk::TreeModel::Row row = m_treeview.get_row( Gtk::TreePath( path ) );
    if( row ) row[ m_columns.m_col_name ] = text;
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
    
    // 移動先に下線を引く
    int cell_x, cell_y, cell_w, cell_h;
    m_treeview.get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );
    if( cell_y < cell_h / 2 ){
        Gtk::TreeModel::Path path_tmp = m_treeview.prev_path( path );
        if( m_treeview.get_row( path_tmp ) ) path = path_tmp;
    }
    draw_underline( m_drag_path_uline, false );
    draw_underline( path, true );
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
    else append_from_buffer( path, after );
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
    m_ren_text->property_underline().set_value( Pango::UNDERLINE_SINGLE );

    col->pack_start( *m_ren_text, true );
    col->add_attribute( *m_ren_text, "text", 0 );
    col->add_attribute( *m_ren_text, "underline", 1 );
    col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    col->add_attribute( *m_ren_text, "underline", 1 );
    
    return col;
}




//
// 選択した行を開く 
//
bool BBSListViewBase::open_row( Gtk::TreePath& path, bool tab )
{
    if( !m_treeview.get_row( path ) ) return false;

    std::string str_tab = "false";
    if( tab ) str_tab = "true";

    Glib::ustring url = path2url( path );
    if( url.empty() ) return false;

    int type = path2type( path );

    switch( type ){

        case TYPE_BOARD:
            CORE::core_set_command( "open_board", DBTREE::url_subject( url ), str_tab, "" );
            break;

        case TYPE_THREAD:
            CORE::core_set_command( "open_article", DBTREE::url_dat( url ), str_tab, "" );
            break;

        case TYPE_IMAGE:
            CORE::core_set_command( "open_image", url );
            CORE::core_set_command( "switch_image" );
            break;

        case TYPE_LINK:
            CORE::core_set_command( "open_url_browser", url );
            break;
    }
    
#ifdef _DEBUG    
    std::cout << "BBSListViewBase::open_row : path = " << path.to_string() << std::endl;
#endif        

    return true;
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
    setup_row( row_new, url, name, type );
    return GET_PATH( row_new );
}



//
// 行に値をセット
//
void BBSListViewBase::setup_row( Gtk::TreeModel::Row& row, Glib::ustring url, Glib::ustring name, int type )
{
    row[ m_columns.m_col_url ] = url;
    row[ m_columns.m_col_name ] = name;
    row[ m_columns.m_type ] = type;
    row[ m_columns.m_underline ] = false;

    switch( type ){

        case TYPE_DIR:
            row[ m_columns.m_col_image ] = ICON::get_icon( ICON::DIR );
            break;

        case TYPE_BOARD:
            row[ m_columns.m_col_image ] = ICON::get_icon( ICON::BOARD );
            break;

        case TYPE_THREAD:
            row[ m_columns.m_col_image ] = ICON::get_icon( ICON::THREAD );
            break;

        case TYPE_IMAGE:
            row[ m_columns.m_col_image ] = ICON::get_icon( ICON::IMAGE );
            break;

        case TYPE_LINK:
            row[ m_columns.m_col_image ] = ICON::get_icon( ICON::LINK );
            break;
    }
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
        m_treeview.expand_row( GET_PATH( *dest ), false );
    }

    // destの後に追加
    else if( after ) it_new = m_treestore->insert_after( dest );

    // destの前に追加
    else it_new = m_treestore->insert( dest );

    Gtk::TreeModel::Row row_tmp = *( it_new );
    setup_row( row_tmp, url, name, type );

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
                Gtk::MessageDialog mdiag( "移動先は送り側のディレクトリのサブディレクトリです", false, Gtk::MESSAGE_ERROR );
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

    // 移動
    Gtk::TreeModel::iterator it_dest = m_treestore->get_iter( path );
    bool subdir = after;
    it_src = list_it.begin();
    for( int i = 0 ; it_src != list_it.end(); ++i, ++it_src ){

        if( vec_cancel[ i ] ) continue;

        if( copy_row( ( *it_src ), it_dest, subdir, after ) ) m_treestore->erase( ( *it_src ) );
        subdir = false;
        after = true;;
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
// after = false ならpathの前に追加( デフォルト true )
//
void BBSListViewBase::append_from_buffer( Gtk::TreeModel::Path path, bool after )
{
    Gtk::TreeModel::Path path_bkup = path;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::append_from_buffer\n";
#endif

    bool subdir = true;
    std::list< CORE::DATA_INFO > infolist = CORE::SBUF_infolist();
    std::list< CORE::DATA_INFO >::iterator it = infolist.begin();
    for( ; it != infolist.end() ; ++it ){

        CORE::DATA_INFO& info = ( *it );
#ifdef _DEBUG    
        std::cout << "append " << info.name << std::endl;
#endif

        if( info.type != TYPE_DIR_END ){
            path = append_row( info.url, info.name, info.type, path, subdir, after );
            after = true;
        }

        if( info.type == TYPE_DIR ) subdir = true;
        else{

            subdir = false;
            if( info.type == TYPE_DIR_END ) path.up();
            else if( info.type == TYPE_IMAGE ) DBIMG::set_protect( info.url, true );
        }
    }

    path = m_treeview.next_path( path_bkup );
    if( m_treeview.get_row( path ) ){
        m_treeview.scroll_to_row( path, 0 );
        m_treeview.get_selection()->unselect_all();
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
            Gtk::MessageDialog mdiag( "ディレクトリを削除すると中のファイルも削除されます。削除しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() != Gtk::RESPONSE_OK ) return;

            break;
        }
    }

    // まとめて削除
    // ディレクトリ内の列を同時に選択している場合があるので後から消す
    it = list_it.end();
    while( it != list_it.begin() ) m_treestore->erase( ( *(--it) ) );
}



//
// tree -> XML 変換
//
//
std::string BBSListViewBase::tree2xml()
{
    if( ! m_ready_tree ) return std::string();
    
#ifdef _DEBUG
    std::cout << "BBSListViewBase::tree2xml\n";
#endif

    std::stringstream xml;

    if( m_treestore->children().empty() ) return std::string();
    
    Gtk::TreePath path = GET_PATH( m_treestore->children().begin() );
    Gtk::TreeModel::Row row;

    // 座標と選択中のパス
    xml << "<pos";
    Gtk::Adjustment* adjust = m_treeview.get_vadjustment();
    if( adjust ) xml << " y=\"" << (int) adjust->get_value() << "\"";
    Gtk::TreeModel::Path focused_path = m_treeview.get_current_path();
    if( !focused_path.empty() ) xml << " path=\"" << focused_path.to_string() << "\"";
    xml << "/>\n";

    while( 1 ){

        if( ( row = m_treeview.get_row( path ) ) ){

            Glib::ustring url = row[ m_columns.m_col_url ];
            Glib::ustring name = row[ m_columns.m_col_name ];
            int type = row[ m_columns.m_type ];

            switch( type ){

            case TYPE_DIR: // サブディレクトリ
                XML_MAKE_DIR(name);
                path.down();
                break;

            case TYPE_BOARD: // 板
                XML_MAKE_BOARD(url,name);
                path.next();
                break;
                
            case TYPE_THREAD: // スレ
                XML_MAKE_THREAD(url,name);
                path.next();
                break;

            case TYPE_IMAGE: // 画像
                XML_MAKE_IMAGE(url,name);
                path.next();
                break;

            case TYPE_COMMENT: // コメント
                XML_MAKE_COMMENT(name);
                path.next();
                break;

            case TYPE_LINK: // リンク
                XML_MAKE_LINK(url,name);
                path.next();
                break;
            }
        }

        // サブディレクトリ内ならupする
        else{

            if( path.get_depth() >= 2 ){

                path.up();

                // サブディレクトリが開いてるか開いてないか
                xml << "</subdir open=\"";
                if( m_treeview.row_expanded( path ) ) xml << "1\" >\n";
                else xml << "0\" >\n";

                path.next();
            }
            else break;
        }
    }

#ifdef _DEBUG_XML    
    std::cout << xml.str() << std::endl;
#endif 

    return xml.str();
}


//
// XML -> tree 変換
//
void BBSListViewBase::xml2tree( const std::string& xml )
{
    JDLIB::Regex regex;

    m_ready_tree = false;
    m_treestore->clear();
    
    std::list< std::string > lines = MISC::get_lines( xml );
    if( lines.empty() ) return;

    // XML を解析してツリーを作る
    Gtk::TreeModel::Row row;
    int y = -1;
    std::string focused_path;
    int level = 0, type;
    std::string url, name;
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){

        std::string& line = *( it );

        // 座標
        if( regex.exec( "< *pos +y=\"(.*)\" +path=\"(.*)\" */>", line, 0, true ) ){

            y = atoi( regex.str( 1 ).c_str() );
            focused_path = regex.str( 2 );
        }
        
        // タイプ別
        else if( ( type = XML::get_type( line, url, name ) ) != TYPE_UNKNOWN ){

            // 板やスレ
            if( type != TYPE_DIR ){
                Gtk::TreeModel::Row row_tmp = *( m_treestore->append( row.children() ) );
                setup_row( row_tmp, url, name, type );
            }

            // サブディレクトリ
            else{

                // レベル0ならルートに追加
                if( level == 0 ) row = *( m_treestore->append() );

                // サブディレクトリの中にサブディレクトリを作る
                else row = *( m_treestore->append( row.children() ) );

                setup_row( row, std::string(), name,  type );

                ++level;
            }
        }
        
        // サブディレクトリ終わり
        else if( regex.exec( "</subdir +open=\"(.*)\" *>", line, 0, true ) ){

            // expand
            if( regex.str( 1 ) == "1" ){

                Gtk::TreePath path = GET_PATH( row );
                m_treeview.expand_parents( path );
                m_treeview.expand_row( path, false );
            }

            // 終わり
            if( level == 0 ) break;

            row = *( row.parent() );
            --level;
        }
    }

    // 前回閉じた位置まで移動
    if( !focused_path.empty() ){
        Gtk::TreePath path = Gtk::TreePath( focused_path );
        if( m_treeview.get_row( path ) ){
            m_treeview.set_cursor( path );
        }
    }

    if( y != -1 ){
        // この段階ではまだスクロールバーが表示されてない時があるのでclock_in()で移動する
        m_jump_y = y;
    }

    m_ready_tree = true;
}



//
// 検索
//
// m_search_invert = true なら前方検索
//
void BBSListViewBase::search()
{
    JDLIB::Regex regex;

    focus_view();
    std::string query = m_toolbar.m_entry_search.get_text();
    if( query.empty() ) return;
    
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
    } 

    Gtk::TreePath path_start = path;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::search() path = " << path.to_string() << " query = " << query << std::endl;
#endif
    
    for(;;){

        // 後方
        if( !m_search_invert ){
            path = m_treeview.next_path( path, false );
            if( ! m_treeview.get_row( path ) ) path =  GET_PATH( *( m_treestore->children().begin() ) );
        }

        // 前方
        else{
            if( path == GET_PATH( *( m_treestore->children().begin() ) ) ) path = GET_PATH( *( m_treestore->children().rbegin() ) );
            else path = m_treeview.prev_path( path, false );
        }

        if( path == path_start ) break;
        
        Glib::ustring name = path2name( path );
        Glib::ustring url = path2url( path );

        if( regex.exec( query, name, 0, true, true, true ) || regex.exec( query, url, 0, true ) ){
            m_treeview.expand_parents( path );
            m_treeview.scroll_to_row( path, 0 );
            m_treeview.set_cursor( path );
            show_status();
            return;
        }
    }
}


// 前検索
void BBSListViewBase::slot_push_up_search()
{
    m_search_invert = true;
    search();
}



// 後検索
void BBSListViewBase::slot_push_down_search()
{
    m_search_invert = false;
    search();
}


//
// 検索entryの操作
//
void BBSListViewBase::slot_entry_operate( int controlid )
{
    if( controlid == CONTROL::Cancel ) focus_view();
}
