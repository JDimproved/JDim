// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "boardview.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "config/globalconf.h"

#include "command.h"
#include "session.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "global.h"
#include "controlid.h"
#include "prefdiagfactory.h"

#include "icons/iconmanager.h"

#include <gtk/gtk.h> // m_liststore->gobj()->sort_column_id = -2
#include <sstream>

using namespace BOARD;

#define COLUMN_TITLE_ID    "ID"
#define COLUMN_TITLE_NAME  "タイトル"
#define COLUMN_TITLE_RES   "レス"
#define COLUMN_TITLE_LOAD  "取得"
#define COLUMN_TITLE_NEW   "新着"
#define COLUMN_TITLE_SINCE "since"
#define COLUMN_TITLE_WRITE "最終書込"
#define COLUMN_TITLE_SPEED "速度"


// row -> path
#define GET_PATH( row ) m_liststore->get_path( row )



// 自動ソート抑制
// -2 = DEFAULT_UNSORTED_COLUMN_ID
//
// 列に値をセットする前にUNSORTED_COLUMN()しておかないと
// いちいちソートをかけるので極端に遅くなる
#define UNSORTED_COLUMN() do{ m_liststore->gobj()->sort_column_id = -2; } while(0)


#define DEFAULT_COLMUN_WIDTH 50

enum{
    COL_MARKVAL_UPDATED = 0,   
    COL_MARKVAL_CACHED, 
    COL_MARKVAL_NORMAL,
    COL_MARKVAL_OLD
};



// set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) を指定して append_columnする
#define APPEND_COLUMN(title,model) do{ \
Gtk::TreeView::Column* col = Gtk::manage( new Gtk::TreeViewColumn( title, model ) ); \
col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ); \
m_treeview.append_column( *col ); \
}while(0)



BoardView::BoardView( const std::string& url,const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_treeview( CONFIG::get_fontname_tree_board(), CONFIG::get_color_back_tree_board() )
{
    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );

    m_toolbar.m_entry_search.signal_activate().connect( sigc::mem_fun( *this, &BoardView::search ) );
    m_toolbar.m_button_close.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::close_view ) );
    m_toolbar.m_button_reload.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::reload ) );
    m_toolbar.m_button_stop.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::stop ) );
    m_toolbar.m_button_new_article.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_new_article ) );
    m_toolbar.m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_push_delete ) );
    m_toolbar.m_button_favorite.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_push_favorite ) );
    m_toolbar.m_button_up_search.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_push_up_search ) );
    m_toolbar.m_button_down_search.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_push_down_search ) );
    m_toolbar.m_button_preferences.signal_clicked().connect( sigc::mem_fun(*this, &BoardView::slot_push_preferences ) );
    m_toolbar.m_entry_search.signal_operate().connect( sigc::mem_fun( *this, &BoardView::slot_entry_operate ) );

    pack_start( m_toolbar, Gtk::PACK_SHRINK );    
    pack_start( m_scrwin );
    show_all_children();    

    // ツリービュー設定
    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );

#ifndef USE_GTKMM24
    
    // セルを固定の高さにする
    // append_column する前に columnに対して set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) すること
    m_treeview.set_fixed_height_mode( true );

#ifdef _DEBUG
    std::cout << "BoardView::BoardView : m_treeview.set_fixed_height_mode\n";
#endif

#endif

    // columnのappend
    APPEND_COLUMN( " ", m_columns.m_col_mark );
    APPEND_COLUMN( COLUMN_TITLE_ID, m_columns.m_col_id );
    APPEND_COLUMN( COLUMN_TITLE_NAME, m_columns.m_col_subject );
    APPEND_COLUMN( COLUMN_TITLE_RES, m_columns.m_col_res );
    APPEND_COLUMN( COLUMN_TITLE_LOAD, m_columns.m_col_str_load );
    APPEND_COLUMN( COLUMN_TITLE_NEW, m_columns.m_col_str_new );
    APPEND_COLUMN( COLUMN_TITLE_SINCE, m_columns.m_col_since );
    APPEND_COLUMN( COLUMN_TITLE_WRITE, m_columns.m_col_write );
    APPEND_COLUMN( COLUMN_TITLE_SPEED, m_columns.m_col_speed );
    m_treeview.set_column_for_height( 2 );

    // サイズを調整しつつソートの設定
    for( guint i = 0; i < COL_MARK_VAL; i++ ){
        
        Gtk::TreeView::Column* column = m_treeview.get_column( i );

        int width = 0;

        switch( i ){

        case COL_MARK:
            width = SESSION::col_mark();
            break;

        case COL_ID:
            width = SESSION::col_id();
            break;

        case COL_SUBJECT:
            width = SESSION::col_subject();
            break;

        case COL_RES:
            width = SESSION::col_number();
            break;

        case COL_STR_LOAD:
            width = SESSION::col_load();
            break;

        case COL_STR_NEW:
            width = SESSION::col_new();
            break;

        case COL_SINCE:
            width = SESSION::col_since();
            break;

        case COL_WRITE:
            width = SESSION::col_write();
            break;

        case COL_SPEED:
            width = 1;//SESSION::col_speed(); ぴったり収まるように調整する
            break;
        }

        if( ! width ) width = DEFAULT_COLMUN_WIDTH;

        column->set_fixed_width( width );
        column->set_resizable( true );
        column->set_clickable( true );        

        // ヘッダをクリックしたときに呼ぶslotの設定
        if( i == COL_MARK ) column->signal_clicked().connect( sigc::mem_fun(*this, &BoardView::slot_mark_clicked ) );        
        else if( i == COL_ID ) column->signal_clicked().connect( sigc::mem_fun(*this, &BoardView::slot_id_clicked ) );
        else column->signal_clicked().connect( sigc::bind< int >( sigc::mem_fun( *this, &BoardView::slot_col_clicked ), i ) );

        // subjectの背景色設定
        if( i == COL_SUBJECT ){
            Gtk::CellRendererText* rentext = dynamic_cast< Gtk::CellRendererText* >( column->get_first_cell_renderer() );
            if( rentext ){
                rentext->property_cell_background().set_value( "yellow" );
                column->add_attribute( *rentext, "cell_background_set", COL_DRAWBG ); 
            }
        }
    }

    // ソート関数セット
    m_liststore->set_sort_func( COL_MARK_VAL, sigc::mem_fun( *this, &BoardView::slot_compare_mark_val ) );    
    m_liststore->set_sort_func( COL_ID, sigc::mem_fun( *this, &BoardView::slot_compare_num_id ) );
    m_liststore->set_sort_func( COL_SUBJECT, sigc::mem_fun( *this, &BoardView::slot_compare_subject ) );
    m_liststore->set_sort_func( COL_RES, sigc::mem_fun( *this, &BoardView::slot_compare_num_res ) );
    m_liststore->set_sort_func( COL_LOAD, sigc::mem_fun( *this, &BoardView::slot_compare_num_load ) );
    m_liststore->set_sort_func( COL_NEW, sigc::mem_fun( *this, &BoardView::slot_compare_new ) );
    m_liststore->set_sort_func( COL_SINCE_T, sigc::mem_fun( *this, &BoardView::slot_compare_since_t ) );
    m_liststore->set_sort_func( COL_WRITE_T, sigc::mem_fun( *this, &BoardView::slot_compare_write_t) );
    m_liststore->set_sort_func( COL_SPEED, sigc::mem_fun( *this, &BoardView::slot_compare_speed ) );
    
    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &BoardView::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &BoardView::slot_button_release ) );
    m_treeview.sig_motion().connect( sigc::mem_fun(*this, &BoardView::slot_motion ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BoardView::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BoardView::slot_key_release ) );

    // D&D設定
    m_treeview.set_reorderable_view( true );
    m_treeview.sig_drag_begin().connect( sigc::mem_fun(*this, &BoardView::slot_drag_begin ) );
    m_treeview.sig_drag_end().connect( sigc::mem_fun(*this, &BoardView::slot_drag_end ) );


    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "OpenTab", "タブで開く"), sigc::mem_fun( *this, &BoardView::slot_open_tab ) );
    action_group()->add( Gtk::Action::create( "Favorite_Article", "スレをお気に入りに追加"), sigc::mem_fun( *this, &BoardView::slot_favorite_thread ) );
    action_group()->add( Gtk::Action::create( "Favorite_Board", "板をお気に入りに登録"), sigc::mem_fun( *this, &BoardView::slot_favorite_board ) );
    action_group()->add( Gtk::Action::create( "GotoTop", "一番上に移動"), sigc::mem_fun( *this, &BoardView::goto_top ) );
    action_group()->add( Gtk::Action::create( "GotoBottom", "一番下に移動"), sigc::mem_fun( *this, &BoardView::goto_bottom ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "Delete", "選択した行のログを削除する"), sigc::mem_fun( *this, &BoardView::delete_view ) );
    action_group()->add( Gtk::Action::create( "OpenRows", "選択した行を開く"), sigc::mem_fun( *this, &BoardView::open_selected_rows ) );
    action_group()->add( Gtk::Action::create( "Unselect", "選択解除"), sigc::mem_fun( *this, &BoardView::slot_unselect_all ) );
    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー"), sigc::mem_fun( *this, &BoardView::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", "タイトルとURLをコピー"), sigc::mem_fun( *this, &BoardView::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く"), sigc::mem_fun( *this, &BoardView::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "AboneThread", "スレをあぼ〜んする"), sigc::mem_fun( *this, &BoardView::slot_abone_thread ) );
    action_group()->add( Gtk::Action::create( "PreferenceArticle", "スレのプロパティ"), sigc::mem_fun( *this, &BoardView::slot_preferences_article ) );
    action_group()->add( Gtk::Action::create( "Preference", "板のプロパティ"), sigc::mem_fun( *this, &BoardView::slot_push_preferences ) );


    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

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
    "<menuitem action='Favorite_Article'/>"
    "<separator/>"
    "<menuitem action='AboneThread'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='PreferenceArticle'/>"
    "<menuitem action='Preference'/>"
    "</popup>"


    // 通常 + 複数
    "<popup name='popup_menu_mul'>"
    "<menuitem action='OpenRows'/>"
    "<separator/>"
    "<menuitem action='Unselect'/>"
    "<separator/>"
    "<menuitem action='Favorite_Article'/>"
    "<separator/>"
    "<menuitem action='AboneThread'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"


    // お気に入りボタン押した時のメニュー
    "<popup name='popup_menu_favorite'>"
    "<menuitem action='Favorite_Article'/>"
    "<menuitem action='Favorite_Board'/>"
    "</popup>"


    // 削除ボタン押した時のメニュー
    "<popup name='popup_menu_delete'>"
    "<menuitem action='Delete'/>"
    "</popup>"


    "</ui>";

    ui_manager()->add_ui_from_string( str_ui );

    // ポップアップメニューにキーアクセレータを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );
    CONTROL::set_menu_motion( popupmenu );

    // マウスジェスチォ可能
    SKELETON::View::set_enable_mg( true );

    // コントロールモード設定
    SKELETON::View::get_control().set_mode( CONTROL::MODE_BOARD );
}


BoardView::~BoardView()
{
#ifdef _DEBUG    
    std::cout << "BoardView::~BoardView : " << get_url() << std::endl;
#endif
    DBTREE::board_save_info( get_url() );
    save_column_width();
}



//
// コピー用URL(メインウィンドウのURLバーなどに表示する)
//
const std::string BoardView::url_for_copy()
{
    return DBTREE::url_boardbase( get_url() );
}



//
// 列の幅の大きさを取得してセッションデータベース更新
//
void BoardView::save_column_width()
{
#ifdef _DEBUG
    std::cout << "save_column_width " << get_url() << std::endl;
#endif    

    for( guint i = 0; i < COL_MARK_VAL; i++ ){
        
        Gtk::TreeView::Column* column = m_treeview.get_column( i );

        int width = 0;
        if( column ) width = column->get_width();
        if( !width ) continue;

        switch( i ){

        case COL_MARK:
            SESSION::set_col_mark( width );
            break;

        case COL_ID:
            SESSION::set_col_id( width );
            break;

        case COL_SUBJECT:
            SESSION::set_col_subject( width );
            break;

        case COL_RES:
            SESSION::set_col_number( width );
            break;

        case COL_STR_LOAD:
            SESSION::set_col_load( width );
            break;

        case COL_STR_NEW:
            SESSION::set_col_new( width );
            break;

        case COL_SINCE:
            SESSION::set_col_since( width );
            break;

        case COL_WRITE:
            SESSION::set_col_write( width );
            break;

        case COL_SPEED:
            SESSION::set_col_speed( width );
            break;
        }
    }
} 
   


//
// ヘッダをクリックしたときのslots
//

// mark をクリック
void BoardView::slot_mark_clicked()
{
    // 前回と違う列をソートした後にクリックしたら ASCENDING
    if( m_previous_col != COL_MARK_VAL ){
        m_previous_col = COL_MARK_VAL;
        m_ascend = false;
    }

    slot_col_clicked( COL_MARK_VAL );
}

// id をクリック
void BoardView::slot_id_clicked()
{
    // 前回違う列をソートした後にクリックしたら ASCENDING
    if( m_previous_col != COL_ID ){
        m_previous_col = COL_ID;
        m_ascend = false;
    }

    slot_col_clicked( COL_ID );
}

// その他のヘッダをクリック
void BoardView::slot_col_clicked( int col )
{
    if( col == COL_STR_LOAD ) col = COL_LOAD;
    else if( col == COL_STR_NEW ) col = COL_NEW;
    else if( col == COL_SINCE ) col = COL_SINCE_T;
    else if( col == COL_WRITE ) col = COL_WRITE_T;
    
    DBTREE::board_set_view_sort_column( get_url(), col );

    if( m_previous_col == col && !m_ascend ){
        m_ascend = true;
        m_liststore->set_sort_column( col, Gtk::SORT_ASCENDING );
        DBTREE::board_set_view_sort_ascend( get_url(), true );
    }
    else{
        m_ascend = false;
        m_liststore->set_sort_column( col, Gtk::SORT_DESCENDING );
        DBTREE::board_set_view_sort_ascend( get_url(), false );
    }

    m_previous_col = col;
}


//
// mark 列のソート関数
//
int BoardView::slot_compare_mark_val( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{
    Gtk::TreeModel::Row row_a = *( a );
    Gtk::TreeModel::Row row_b = *( b );

    int num_a = row_a[ m_columns.m_col_mark_val ];
    int num_b = row_b[ m_columns.m_col_mark_val ];

    // 抽出状態優先
    int ret = compare_drawbg( row_a, row_b );
    if( ret ) return ret;

    // 両方ともマーク有り
    if( num_a < COL_MARKVAL_NORMAL && num_b < COL_MARKVAL_NORMAL ){

        if( num_a > num_b ) return 1;
        else if( num_a < num_b )return -1;

        // 同じマークならIDで決める
        return compare_id( row_a, row_b );
    }

    //　マークが付いている方を優先
    if( num_a < COL_MARKVAL_NORMAL ) return ( m_ascend ? -1 : 1 );
    if( num_b < COL_MARKVAL_NORMAL ) return ( m_ascend ? 1 : -1 );

    // どちらもマークが付いていないならIDの小さい方を優先
    return compare_id( row_a, row_b );
}


// name 列のソート関数
int BoardView::slot_compare_subject( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{
    Gtk::TreeModel::Row row_a = *( a ); 
    Gtk::TreeModel::Row row_b = *( b ); 

    Glib::ustring name_a = row_a[ m_columns.m_col_subject ]; 
    Glib::ustring name_b = row_b[ m_columns.m_col_subject ]; 

    // 抽出状態優先
    int ret = compare_drawbg( row_a, row_b );
    if( ret ) return ret;

    if( name_a > name_b ) return 1;
    else if( name_a < name_b )return -1;

    // 同じ値ならIDで決める
    return compare_id( row_a, row_b );
}


#define SLOT_COMPARE_ROW( target ) do{ \
Gtk::TreeModel::Row row_a = *( a ); \
Gtk::TreeModel::Row row_b = *( b ); \
int num_a = row_a[ target ]; \
int num_b = row_b[ target ]; \
return compare_row( num_a, num_b, row_a, row_b ); \
}while(0)

// ID 列のソート関数
int BoardView::slot_compare_num_id( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_id ); }

// res 列のソート関数
int BoardView::slot_compare_num_res( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_res ); }

// load 列のソート関数
int BoardView::slot_compare_num_load( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_load ); }

// new 列のソート関数
int BoardView::slot_compare_new( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_new ); }

// since 列のソート関数
int BoardView::slot_compare_since_t( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_since_t ); }

// write 列のソート関数
int BoardView::slot_compare_write_t( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{ SLOT_COMPARE_ROW( m_columns.m_col_write_t ); }

#undef COMPARE_ROW

// speed 列のソート関数
int BoardView::slot_compare_speed( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{
    Gtk::TreeModel::Row row_a = *( a ); 
    Gtk::TreeModel::Row row_b = *( b ); 
    int num_a = row_a[ m_columns.m_col_speed ]; 
    int num_b = row_b[ m_columns.m_col_speed ]; 

    // 抽出状態優先
    int ret = compare_drawbg( row_a, row_b );
    if( ret ) return ret;

    if( num_a > num_b ) return 1;
    else if( num_a < num_b )return -1;

    // 同じ値ならIDで決める
    return compare_id( row_a, row_b );
}



//
// 抽出状態で比較
//
// 抽出状態にあるものを上にする。同じなら0
//
int BoardView::compare_drawbg( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    bool draw_a = row_a[ m_columns.m_col_drawbg ];
    bool draw_b = row_b[ m_columns.m_col_drawbg ];
    if( draw_a && !draw_b ) return ( m_ascend ? -1 : 1 );
    if( draw_b && !draw_a ) return ( m_ascend ? 1 : -1 );
    return 0;
}



//
// markで比較
//
// マークが付いているか、値の小さい方を上にする
//
int BoardView::compare_mark( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    int num_a = row_a[ m_columns.m_col_mark_val ];
    int num_b = row_b[ m_columns.m_col_mark_val ];

    if( num_a < COL_MARKVAL_NORMAL && num_b < COL_MARKVAL_NORMAL ){

        if( num_a > num_b ) return ( m_ascend ? 1 : -1 );
        else if( num_a < num_b ) return ( m_ascend ? -1 : 1 );

        // 同じマークならIDで決める
        return compare_id( row_a, row_b );
    }

    //　マークが付いている方を優先
    if( num_a < COL_MARKVAL_NORMAL ) return ( m_ascend ? -1 : 1 );
    if( num_b < COL_MARKVAL_NORMAL ) return ( m_ascend ? 1 : -1 );

    return 0;
}


//
// IDで比較
//
// IDの小さい方を上にする。両方同じなら 0
//
int BoardView::compare_id( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    int num_a = row_a[ m_columns.m_col_id ];
    int num_b = row_b[ m_columns.m_col_id ];
    if( num_a > num_b ) return ( m_ascend ? 1 : -1 );
    else if( num_a < num_b ) return ( m_ascend ? -1 : 1 );
    return 0;
}


//
// ソート関数の本体
//
int BoardView::compare_row( int& num_a, int& num_b, Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    // 抽出状態優先
    int ret = compare_drawbg( row_a, row_b );
    if( ret ) return ret;

    // 両方とも 0 より大きい
    if( num_a > 0 && num_b > 0 ){

        // 普通にcompare
        if( num_a > num_b ) return 1;
        else if( num_a < num_b )return -1;

        // 同じ値ならIDで決める
        return compare_id( row_a, row_b );
    }

    // 0より大きい方を優先
    if( num_a > 0 ) return ( m_ascend ? -1 : 1 );
    if( num_b > 0 ) return ( m_ascend ? 1 : -1 );

    // マークの付いている方を優先
    ret = compare_mark( row_a, row_b );
    if( ret ) return ret;

    // 両方0以下ならIDで決める
    return compare_id( row_a, row_b );
}


//
// クロック入力
//
void BoardView::clock_in()
{
    View::clock_in();

    m_treeview.clock_in();
}



//
// リロード
//
void BoardView::reload()
{
    show_view();
    CORE::core_set_command( "set_history_board", get_url() );
}


//
// ロード停止
//
void BoardView::stop()
{
    DBTREE::board_stop_load( get_url() );
}



//
// ビュー表示
//
void BoardView::show_view()
{
#ifdef _DEBUG
    std::cout << "BoardView::show_view " << get_url() << std::endl;
#endif

    // DBに登録されてない
    if( get_url().empty() ){
        set_status( "invalid URL" );
        CORE::core_set_command( "set_status","", get_status() );
        return;
    }

    // タブにアイコンを表示
    BOARD::get_admin()->set_command( "set_tabicon", get_url(), "loading" );

    // タブに名前をセット
    BOARD::get_admin()->set_command( "set_tablabel", get_url(), DBTREE::board_name( get_url() ) );

    m_liststore->clear();
    m_pre_query = std::string();
    
    // download 開始
    // 終わったら update_view() が呼ばれる
    DBTREE::board_download_subject( get_url() );
    set_status( "loading..." );
    CORE::core_set_command( "set_status","", get_status() );
}



//
// 再描画(画面初期化)
//
void BoardView::redraw_view()
{
#ifdef _DEBUG
    std::cout << "BoardView::redraw_view" << get_url() << std::endl;
#endif  

    m_search_invert = false;

    // ソート状態回復
    int col = DBTREE::board_view_sort_column( get_url() );
    m_previous_col = col;
    m_ascend = DBTREE::board_view_sort_ascend( get_url() );
    if( col <= 0 ){
        m_previous_col = COL_NUM_COL;
        m_ascend = false;
        slot_mark_clicked();
    }
    else if( m_ascend ) m_liststore->set_sort_column( col, Gtk::SORT_ASCENDING );
    else m_liststore->set_sort_column( col, Gtk::SORT_DESCENDING );

    goto_top();
}



//
// 色、フォントの更新
//
void BoardView::relayout()
{
    m_treeview.init_color( CONFIG::get_color_back_tree_board() );
    m_treeview.init_font( CONFIG::get_fontname_tree_board() );
}



//
// view更新
//
// subject.txtのロードが終わったら呼ばれる
//
void BoardView::update_view()
{
#ifdef _DEBUG
    std::cout << "BoardView::update_view " << get_url() << std::endl;
#endif    

    m_liststore->clear();

    // 高速化のためデータベースに直接アクセス
    std::list< DBTREE::ArticleBase* >& list_subject = DBTREE::board_list_subject( get_url() );

    time_t current_t = time( NULL );
    
    // 自動ソート抑制
    UNSORTED_COLUMN();

    int id = 1;
    if( list_subject.size() ){

        std::list< DBTREE::ArticleBase* >::iterator it;
        for( it = list_subject.begin(); it != list_subject.end(); ++it, ++id ){

            DBTREE::ArticleBase* art = *( it );
    
            // 行を作って内容をセット
            Gtk::TreeModel::Row row = *( m_liststore->prepend() ); // append より prepend の方が速いらしい

            row[ m_columns.m_col_id ]  = id;
            row[ m_columns.m_col_since ] = art->get_since_date();

            if( art->is_current() )
                row[ m_columns.m_col_speed ] = art->get_number() / MAX( 1, ( current_t - art->get_since_time()) / ( 60 * 60 * 24 ) + 1 );
        
            row[ m_columns.m_col_since_t ] = art->get_since_time();
            row[ m_columns.m_col_id_dat ] = art->get_id();

            update_row_common( art, row, id );
        }

        redraw_view();
    }

    // ステータスバー更新
    std::ostringstream ss_tmp;
    ss_tmp << DBTREE::board_str_code( get_url() ) << " [ 全 " << ( id -1 ) << " ] ";
    set_status( ss_tmp.str() );
    CORE::core_set_command( "set_status","", get_status() );

    // タブのアイコン状態を更新
    BOARD::get_admin()->set_command( "set_tabicon", get_url(), "default" );

    focus_view();
}

void BoardView::focus_view()
{
    m_treeview.grab_focus();
}


void BoardView::focus_out()
{
    SKELETON::View::focus_out();

    save_column_width();
    m_treeview.hide_tooltip();
}


void BoardView::close_view()
{
    BOARD::get_admin()->set_command( "close_currentview" );
}


//
// 選択した行のログをまとめて削除
//
void BoardView::delete_view()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){
        Gtk::TreeModel::Row row = *( *it );
        std::string url = DBTREE::url_datbase( get_url() ) + row[ m_columns.m_col_id_dat ];
#ifdef _DEBUG
        std::cout << url << std::endl;
#endif
        CORE::core_set_command( "delete_article", url );
    }
}




//
// viewの操作
//
void BoardView::operate_view( const int& control )
{
    bool open_tab = false;

    Gtk::TreePath path = m_treeview.get_current_path();;
    if( path.empty() ) return;

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
    
            // 選択
        case CONTROL::OpenArticleTab:
            open_tab = true;
        case CONTROL::OpenArticle:
            open_row( path, open_tab );
            break;

            // Listに戻る
        case CONTROL::Left:
            CORE::core_set_command( "switch_bbslist" );
            break;

            // 現在の記事を表示
        case CONTROL::Right:
            CORE::core_set_command( "switch_article" );
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

        case CONTROL::TabLeft:
            BOARD::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            BOARD::get_admin()->set_command( "tab_right" );
            break;

        case CONTROL::Quit:
            close_view();
            break;

        case CONTROL::Reload:
            reload();
            break;

        case CONTROL::StopLoading:
            stop();
            break;

        case CONTROL::NewArticle:
            slot_new_article();
            break;

        case CONTROL::Delete:
        {
            Gtk::MessageDialog mdiag( "選択した行のログを削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() != Gtk::RESPONSE_OK ) return;
            delete_view();
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
// 先頭に戻る
//
void BoardView::goto_top()
{
    m_treeview.goto_top();
}


//
// 一番最後へ
//
void BoardView::goto_bottom()
{
    m_treeview.goto_bottom();
}



//
// 上へ移動
//
void BoardView::row_up()
{
    m_treeview.row_up();
}    


//
// 下へ移動
//
void BoardView::row_down()
{
    m_treeview.row_down();
} 
   

//
// page up
//
void BoardView::page_up()
{
    m_treeview.page_up();
}    


//
// page down
//
void BoardView::page_down()
{
    m_treeview.page_down();
} 
   

//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* BoardView::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = NULL;

    // 削除サブメニュー
    if( url == "popup_menu_delete" ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_delete" ) );
    }

    // お気に入りサブメニュー
    else if( url == "popup_menu_favorite" ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite" ) );
    }

    // 通常メニュー
    else if( m_treeview.get_selection()->get_selected_rows().size() == 1 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    }

    // 複数選択メニュー
    else{ 
        m_path_selected = Gtk::TreeModel::Path();
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );
    }

    return popupmenu;
}



//
// 特定の行だけの更新
//
void BoardView::update_item( const std::string& id_dat )
{
#ifdef _DEBUG
    std::cout << "BoardView::update_item " << id_dat << std::endl;
#endif    

    Gtk::TreeModel::Children child = m_liststore->children();
    Gtk::TreeModel::Children::iterator it;

    // 自動ソート抑制
    UNSORTED_COLUMN();
    
    for( it = child.begin() ; it != child.end() ; ++it ){
        Gtk::TreeModel::Row row = *( it );
        
        // 対象の行なら内容更新
        if( row[ m_columns.m_col_id_dat ] == id_dat ){

            std::string url = DBTREE::url_datbase( get_url() ) + row[ m_columns.m_col_id_dat ];
            DBTREE::ArticleBase* art = DBTREE::get_article( url );
            int id = row[ m_columns.m_col_id ];
            update_row_common( art, row, id );
        }
    }
}



//
// update_view() と update_item() で共通に更新する列
//
void BoardView::update_row_common( DBTREE::ArticleBase* art, Gtk::TreeModel::Row& row, int& id )
{
    if( art->empty() ) return;

    // タイトル、レス数、抽出
    row[ m_columns.m_col_subject ] = art->get_subject();
    row[ m_columns.m_col_res ] = art->get_number();
    row[ m_columns.m_col_drawbg ] = false;

    // 読み込み数

    if( art->get_number_load() ){
        const int tmpsize = 32;
        char tmp[ tmpsize ];
        snprintf( tmp, tmpsize, "%d", art->get_number_load() );
        row[ m_columns.m_col_str_load ] = tmp;
        snprintf( tmp, tmpsize, "%d", art->get_number() - art->get_number_load() );
        row[ m_columns.m_col_str_new ] = tmp;

        row[ m_columns.m_col_load ] = art->get_number_load();
        row[ m_columns.m_col_new ] = art->get_number() - art->get_number_load();
    }
    else{
        row[ m_columns.m_col_str_load ] = "";
        row[ m_columns.m_col_str_new ] = "";

        row[ m_columns.m_col_load ] = -1;
        row[ m_columns.m_col_new ] = -1;
    }


    //
    // マーク

    // dat落ち
    int mark_val;
    if( ! art->is_current() ){
        mark_val = COL_MARKVAL_OLD;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::DOWN );
    }
    // キャッシュあり、新着あり
    else if( art->get_number_load() && art->get_number() > art->get_number_load() ){
        mark_val = COL_MARKVAL_UPDATED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::UPDATE );
    }
    // キャッシュあり、新着無し
    else if( art->get_number_load() ){
        mark_val = COL_MARKVAL_CACHED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::CHECK );
    }
    //キャッシュ無し
    else{
        mark_val = COL_MARKVAL_NORMAL;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::TRANSPARENT );
    }
    row[ m_columns.m_col_mark_val ] = mark_val;


    // 書き込み時間
    if( art->get_write_time() ){
        row[ m_columns.m_col_write ] = art->get_write_date();
        row[ m_columns.m_col_write_t ] = art->get_write_time();
    }
    else{
        row[ m_columns.m_col_write ] = std::string();
        if( mark_val < COL_MARKVAL_NORMAL ) row[ m_columns.m_col_write_t ] = 0;
        else row[ m_columns.m_col_write_t ] = -1;
    }
}


//
// マウスボタン押した
//
bool BoardView::slot_button_press( GdkEventButton* event )
{
    // マウスジェスチャ
    SKELETON::View::get_control().MG_start( event );

    // ダブルクリック
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

    return true;
}



//
// マウスボタン離した
//
bool BoardView::slot_button_release( GdkEventButton* event )
{
    /// マウスジェスチャ
    int mg = SKELETON::View::get_control().MG_end( event );
    if( mg != CONTROL::None && enable_mg() ){
        operate_view( mg );
        return true;
    }

    int x = (int)event->x;
    int y = (int)event->y;
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int cell_x;
    int cell_y;
   
    // 座標からpath取得
    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) ){

        m_path_selected = path;

        // リサイズするときにラベルをクリックすると一番上のアイテムが開く問題の対処
        // かなりその場しのぎな方法なのでGTKのバージョンが上がったら誤動作するかも
        if( path.to_string() == "0" && x == cell_x && y == cell_y ) return true;
        
#ifdef _DEBUG        
        std::cout << "BoardView::slot_button_press : " << path.to_string() << " "
                  << x << " " << y << " " << cell_x << " " << cell_y << std::endl;
#endif

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

        // スレを開く
        bool openarticle = SKELETON::View::get_control().button_alloted( event, CONTROL::OpenArticleButton );
        bool openarticletab = SKELETON::View::get_control().button_alloted( event, CONTROL::OpenArticleTabButton );
        if( openarticle || openarticletab ) open_row( path, openarticletab );

        // ポップアップメニューボタン
        else if( SKELETON::View::get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){

            SKELETON::View::show_popupmenu( "", false );
        }

        event->type = type_copy;
    }

    return true;
}



//
// マウス動かした
//
bool BoardView::slot_motion( GdkEventMotion* event )
{
    /// マウスジェスチャ
    SKELETON::View::get_control().MG_motion( event );

    int x = (int)event->x;
    int y = (int)event->y;
    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* column;
    int cell_x;
    int cell_y;

    // ツールチップに文字列をセットする
    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) ){

        m_treeview.set_tooltip_min_width( column->get_width() );
        if( column->get_title() == COLUMN_TITLE_NAME ) m_treeview.set_str_tooltip( get_name_of_cell( path, m_columns.m_col_subject ) );
        else if( column->get_title() == COLUMN_TITLE_SINCE ) m_treeview.set_str_tooltip( get_name_of_cell( path, m_columns.m_col_since ) );
        else if( column->get_title() == COLUMN_TITLE_WRITE ) m_treeview.set_str_tooltip( get_name_of_cell( path, m_columns.m_col_write ) );
        else m_treeview.set_str_tooltip( std::string() );
    }

    return true;
}




//
// キー入力
//
bool BoardView::slot_key_press( GdkEventKey* event )
{
    int key = SKELETON::View::get_control().key_press( event );

    // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
    // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
    // OpenArticle(Tab)は slot_key_release() で処理する
    if( key == CONTROL::OpenArticle ) return true;
    if( key == CONTROL::OpenArticleTab ) return true;

    operate_view( key );

    return true;
}


//
// キーリリースイベント
//
bool BoardView::slot_key_release( GdkEventKey* event )
{
    int key = SKELETON::View::get_control().key_press( event );

    // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
    // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
    // OpenArticle(Tab)はここで処理する
    if( key == CONTROL::OpenArticle ) operate_view( key );
    if( key == CONTROL::OpenArticleTab ) operate_view( key );
   
    return true;
}



//
// このビューからD&Dを開始したときにtreeviewから呼ばれる
//
void BoardView::slot_drag_begin()
{
#ifdef _DEBUG    
    std::cout << "BoardView::slot_drag_begin\n";
#endif

    CORE::DND_Begin( get_url() );
    set_article_to_buffer();
}



//
// このビューからD&Dを開始した後にD&Dを終了するとtreeviewから呼ばれる
//
void BoardView::slot_drag_end()
{
#ifdef _DEBUG    
    std::cout << "BoardView::slot_drag_end\n";
#endif

    CORE::DND_End();
}





//
// popupmenu でタブで開くを選択
//
void BoardView::slot_open_tab()
{
    if( ! m_path_selected.empty() ) open_row( m_path_selected, true );
}


//
// スレをお気に入りに登録
//
// ポップアップメニューのslot
//
void BoardView::slot_favorite_thread()
{
    // 共有バッファにデータをセットしてから append_favorite コマンド実行
    set_article_to_buffer();
    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}




//
// 板をお気に入りに追加
//
void BoardView::slot_favorite_board()
{
    // 共有バッファにデータをセットしてから append_favorite コマンド実行
    set_board_to_buffer();
    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}


//
// 新スレをたてる
//
void BoardView::slot_new_article()
{
    CORE::core_set_command( "create_new_thread", get_url() );
}


//
// ツールバーの削除ボタン
//
void BoardView::slot_push_delete()
{
    SKELETON::View::show_popupmenu( "popup_menu_delete", false );
}


//
// ツールバーのお気に入りボタン
//
void BoardView::slot_push_favorite()
{
    SKELETON::View::show_popupmenu( "popup_menu_favorite", false );
}


//
// 選択解除
//
void BoardView::slot_unselect_all()
{
    m_treeview.get_selection()->unselect_all();
}


//
// スレのURLをコピー
//
void BoardView::slot_copy_url()
{
    if( m_path_selected.empty() ) return;

    std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    COPYCLIP( url );
}


// スレの名前とURLをコピー
//
void BoardView::slot_copy_title_url()
{
    if( m_path_selected.empty() ) return;

    std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    std::string name = DBTREE::article_subject( url );
    std::stringstream ss;
    ss << name << std::endl
       << url << std::endl;

    COPYCLIP( ss.str() );
}


//
// ポップアップメニューでブラウザで開くを選択
//
void BoardView::slot_open_browser()
{
    std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    CORE::core_set_command( "open_url_browser", url );
}



//
// 記事を開く 
//
bool BoardView::open_row( Gtk::TreePath& path, bool tab )
{
    std::string str_tab = "false";
    if( tab ) str_tab = "true";

    std::string url_target = path2daturl( path );

#ifdef _DEBUG
    std::cout << "BoardView::open_row " << url_target << std::endl;
#endif

    if( url_target.empty() ) return false;

    CORE::core_set_command( "open_article", url_target , str_tab, "" );
    return true;
}



//
// 選択した行をまとめて開く
//
void BoardView::open_selected_rows()
{
    std::string list_url;
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){
        Gtk::TreeModel::Row row = *( *it );
        std::string url = DBTREE::url_datbase( get_url() ) + row[ m_columns.m_col_id_dat ];

        if( !list_url.empty() ) list_url += " ";
        list_url += url;
    }

    CORE::core_set_command( "open_article_list", std::string(), list_url );
}


//
// path -> スレッドの(dat型)URL変換
// 
std::string BoardView::path2daturl( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return std::string();

    std::string url = DBTREE::url_datbase( get_url() ) + row[ m_columns.m_col_id_dat ];
    return url;
}




//
// 抽出
//
bool BoardView::drawout()
{
    bool find = false;
    bool reset = false;

    focus_view();
    std::string query = m_toolbar.m_entry_search.get_text();

    // 空の時はリセット
    if( query.empty() ){
        find = true;
        reset = true;
    }

    if( m_pre_query == query ) return false;
    m_pre_query = query;
    
#ifdef _DEBUG
    std::cout << "BoardView::drawout query = " <<  query << std::endl;
#endif

    // 自動ソート抑制
    UNSORTED_COLUMN();

    JDLIB::Regex regex;
    Gtk::TreeModel::Children child = m_liststore->children();
    Gtk::TreeModel::Children::iterator it = child.begin();
    for( ; it != child.end() ; ++it ){

        Gtk::TreeModel::Row row = *( it );
        Glib::ustring subject = row[ m_columns.m_col_subject ];

        if( reset ) row[ m_columns.m_col_drawbg ] = false;
        else if( regex.exec( query, subject, 0, true ) ){
            row[ m_columns.m_col_drawbg ] = true;
            find = true;

#ifdef _DEBUG
            std::cout << subject << " " << row[ m_columns.m_col_mark_val ] << std::endl;
#endif

        }
        else row[ m_columns.m_col_drawbg ] = false;
    }

    if( find ) redraw_view();

    return true;
}



//
// 検索移動
//
void BoardView::search()
{
    if( drawout() ) return;

    focus_view();
    std::string query = m_toolbar.m_entry_search.get_text();
    if( query.empty() ) return;
   
    Gtk::TreePath path = m_treeview.get_current_path();;
    if( path.empty() ){
        if( m_search_invert ) path = GET_PATH( *( m_liststore->children().begin() ) );
        else GET_PATH( *( m_liststore->children().rbegin() ) );
    }

    Gtk::TreePath path_start = path;
    JDLIB::Regex regex; 

#ifdef _DEBUG
    std::cout << "BoardView::search start = " << path_start.to_string() << " query = " <<  query << std::endl;
#endif
    
    for(;;){

        if( !m_search_invert ){
            // 次へ
            path.next();
            // 先頭に戻る
            if( ! m_treeview.get_row( path ) ) path =  GET_PATH( *( m_liststore->children().begin() ) );
        }
        else{
            // 前へ
            if( ! path.prev() ){
                // 一番後へ
                path =  GET_PATH( *( m_liststore->children().rbegin() ) );
            }
        }

        
        if( path == path_start ) break;
        
        Glib::ustring subject = get_name_of_cell( path, m_columns.m_col_subject );
        if( regex.exec( query, subject, 0, true ) ){
            m_treeview.scroll_to_row( path, 0 );
            m_treeview.set_cursor( path );
            return;
        }
    }
}




// 前検索
void BoardView::slot_push_up_search()
{
    m_search_invert = true;
    search();
}



// 後検索
void BoardView::slot_push_down_search()
{
    m_search_invert = false;
    search();
}


//
// 検索entryの操作
//
void BoardView::slot_entry_operate( int controlid )
{
    if( controlid == CONTROL::Cancel ) focus_view();
}



//
// 板プロパティ表示
//
void BoardView::slot_push_preferences()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_BOARD, get_url() );
    pref->run();
    delete pref;
}


//
// スレプロパティ表示
//
void BoardView::slot_preferences_article()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2daturl( m_path_selected );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( CORE::PREFDIAG_ARTICLE, url );
    pref->run();
    delete pref;
}


//
// 選択したスレをあぼーん
//
void BoardView::slot_abone_thread()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    if( ! list_it.size() ) return;

    std::list< std::string > threads = DBTREE::get_abone_list_thread( get_url() );

    for( ; it != list_it.end(); ++it ){
        Gtk::TreeModel::Row row = *( *it );
        Glib::ustring subject = row[ m_columns.m_col_subject ];
        threads.push_back( subject );
    }

    // あぼーん情報更新
    // 板の再描画も行われる
    std::list< std::string > words = DBTREE::get_abone_list_word_thread( get_url() );
    std::list< std::string > regexs = DBTREE::get_abone_list_regex_thread( get_url() );
    DBTREE::reset_abone_thread( get_url(), threads, words, regexs );
}



//
// path と column からそのセルの内容を取得
//
template < typename ColumnType >
std::string BoardView::get_name_of_cell( Gtk::TreePath& path, const Gtk::TreeModelColumn< ColumnType >& column )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return std::string();

    Glib::ustring name = row[ column ];
    return name;
}



//
// 共有バッファに選択中の行を登録する
//
void BoardView::set_article_to_buffer()
{
    CORE::SBUF_clear_info();

    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    if( list_it.size() ){
        std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
        for( ; it != list_it.end(); ++it ){

            Gtk::TreeModel::Row row = *( *it );
            Glib::ustring name = row[ m_columns.m_col_subject ];

            CORE::DATA_INFO info;
            info.type = TYPE_THREAD;
            info.url = DBTREE::url_datbase( get_url() ) + row[ m_columns.m_col_id_dat ];
            info.name = name.raw();

            CORE::SBUF_append( info );
#ifdef _DEBUG    
            std::cout << "append " << info.name << std::endl;
#endif
        }
    }
}



//
// 共有バッファに板を登録する
//
void BoardView::set_board_to_buffer()
{
    CORE::DATA_INFO info;
    info.type = TYPE_BOARD;
    info.url = get_url();
    info.name = DBTREE::board_name( get_url() );

    CORE::SBUF_clear_info();
    CORE::SBUF_append( info );
}
