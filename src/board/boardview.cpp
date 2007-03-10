// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "boardview.h"

#include "skeleton/msgdiag.h"

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
#include "httpcode.h"
#include "colorid.h"
#include "fontid.h"

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
    COL_MARKVAL_OLD = -2,        // dat 落ち
    COL_MARKVAL_FINISHED = -1,   // キャッシュあり、新着無し、規定スレ数を越えている
    COL_MARKVAL_NORMAL = 0,      // 通常状態
    COL_MARKVAL_NEWTHREAD,       // 新スレ
    COL_MARKVAL_CACHED,          // キャッシュあり、新着無し
    COL_MARKVAL_UPDATED,         // キャッシュあり、新着有り
    COL_MARKVAL_BKMARKED,        // ブックマークされている、新着無し
    COL_MARKVAL_BKMARKED_UPDATED // ブックマークされている、新着有り
};


enum
{
    SORTMODE_ASCEND = 0,
    SORTMODE_DESCEND,

    SORTMODE_MARK1, // 通常
    SORTMODE_MARK2, // 新着を一番上に
    SORTMODE_MARK3  // 反転
};


// 昇順で上か下か
enum{
    COL_A_UP = -1,
    COL_B_UP = 1,
};


enum
{
    BOOKMARK_AUTO = -1,
    BOOKMARK_UNSET = 0,
    BOOKMARK_SET = 1
};


// set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) を指定して append_columnする
#define APPEND_COLUMN(title,model) do{ \
Gtk::TreeView::Column* col = Gtk::manage( new Gtk::TreeViewColumn( title, model ) ); \
col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ); \
m_treeview.append_column( *col ); \
}while(0)



BoardView::BoardView( const std::string& url,const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_treeview( CONFIG::get_fontname( FONT_BOARD ), COLOR_BACK_BOARD ),
      m_col( COL_NUM_COL ),
      m_previous_col( COL_NUM_COL ),
      m_sortmode( SORTMODE_ASCEND ),
      m_previous_sortmode( false )
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
    m_toolbar.m_button_search_cache.signal_clicked().connect( sigc::mem_fun( *this, &BoardView::slot_search_cache ) );
    m_toolbar.m_button_preferences.signal_clicked().connect( sigc::mem_fun(*this, &BoardView::slot_push_preferences ) );
    m_toolbar.m_entry_search.signal_operate().connect( sigc::mem_fun( *this, &BoardView::slot_entry_operate ) );

    pack_start( m_toolbar, Gtk::PACK_SHRINK );    
    pack_start( m_scrwin );
    show_all_children();    

    // ツリービュー設定
    m_liststore = Gtk::ListStore::create( m_columns );

#if GTKMMVER <= 260
    // gtkmm26以下にはunset_model()が無いのでここでset_model()しておく
    m_treeview.set_model( m_liststore );
#endif

#if GTKMMVER >= 260

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
    for( guint i = 0; i < COL_VISIBLE_END; i++ ){
        
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
            width = SESSION::col_speed(); 
            break;
        }

        if( ! width ) width = DEFAULT_COLMUN_WIDTH;

        column->set_fixed_width( width );
        column->set_resizable( true );
        column->set_clickable( true );        

        // ヘッダをクリックしたときに呼ぶslot
        column->signal_clicked().connect( sigc::bind< int >( sigc::mem_fun( *this, &BoardView::slot_col_clicked ), i ) );

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
    m_liststore->set_sort_func( COL_MARK, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );    
    m_liststore->set_sort_func( COL_ID, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SUBJECT, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_RES, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_STR_LOAD, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_STR_NEW, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SINCE, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_WRITE, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SPEED, sigc::mem_fun( *this, &BoardView::slot_compare_row ) );
    
    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &BoardView::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &BoardView::slot_button_release ) );
    m_treeview.sig_motion_notify().connect( sigc::mem_fun(*this, &BoardView::slot_motion_notify ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BoardView::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BoardView::slot_key_release ) );
    m_treeview.sig_scroll_event().connect( sigc::mem_fun(*this, &BoardView::slot_scroll_event ) );

    // D&D設定
    m_treeview.set_reorderable_view( true );
    m_treeview.sig_drag_begin().connect( sigc::mem_fun(*this, &BoardView::slot_drag_begin ) );
    m_treeview.sig_drag_end().connect( sigc::mem_fun(*this, &BoardView::slot_drag_end ) );


    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "BookMark", "ブックマーク設定/解除"),
                         sigc::bind< int >( sigc::mem_fun( *this, &BoardView::slot_bookmark ), BOOKMARK_AUTO ) );
    action_group()->add( Gtk::Action::create( "SetBookMark", "ブックマーク設定"),
                         sigc::bind< int >( sigc::mem_fun( *this, &BoardView::slot_bookmark ), BOOKMARK_SET ) );
    action_group()->add( Gtk::Action::create( "UnsetBookMark", "ブックマーク解除"),
                         sigc::bind< int >( sigc::mem_fun( *this, &BoardView::slot_bookmark ), BOOKMARK_UNSET ) );
    action_group()->add( Gtk::Action::create( "OpenTab", "タブで開く"), sigc::mem_fun( *this, &BoardView::slot_open_tab ) );
    action_group()->add( Gtk::Action::create( "Favorite_Article", "スレをお気に入りに追加"), sigc::mem_fun( *this, &BoardView::slot_favorite_thread ) );
    action_group()->add( Gtk::Action::create( "Favorite_Board", "板をお気に入りに登録"), sigc::mem_fun( *this, &BoardView::slot_favorite_board ) );
    action_group()->add( Gtk::Action::create( "GotoTop", "一番上に移動"), sigc::mem_fun( *this, &BoardView::goto_top ) );
    action_group()->add( Gtk::Action::create( "GotoBottom", "一番下に移動"), sigc::mem_fun( *this, &BoardView::goto_bottom ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "Delete", "選択した行のログを削除する"), sigc::mem_fun( *this, &BoardView::delete_view ) );
    action_group()->add( Gtk::Action::create( "OpenRows", "選択した行を開く"), sigc::mem_fun( *this, &BoardView::open_selected_rows ) );
    action_group()->add( Gtk::Action::create( "CopyURL", "URLをコピー"), sigc::mem_fun( *this, &BoardView::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", "タイトルとURLをコピー"), sigc::mem_fun( *this, &BoardView::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", "ブラウザで開く"), sigc::mem_fun( *this, &BoardView::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "AboneThread", "スレをあぼ〜んする"), sigc::mem_fun( *this, &BoardView::slot_abone_thread ) );
    action_group()->add( Gtk::Action::create( "PreferenceArticle", "スレのプロパティ"), sigc::mem_fun( *this, &BoardView::slot_preferences_article ) );
    action_group()->add( Gtk::Action::create( "Preference", "板のプロパティ"), sigc::mem_fun( *this, &BoardView::slot_push_preferences ) );
    action_group()->add( Gtk::Action::create( "SaveDat", "datファイルを保存"), sigc::mem_fun( *this, &BoardView::slot_save_dat ) );

    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

    Glib::ustring str_ui = 
    "<ui>"

    // 通常
    "<popup name='popup_menu'>"
    "<menuitem action='BookMark'/>"
    "<separator/>"
    "<menuitem action='OpenTab'/>"
    "<menuitem action='OpenBrowser'/>"
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyTitleURL'/>"
    "<separator/>"
    "<menuitem action='SaveDat'/>"
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
    "<menuitem action='SetBookMark'/>"
    "<menuitem action='UnsetBookMark'/>"
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
    set_enable_mg( true );

    // オートリロード可
    set_enable_autoreload( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_BOARD );
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

    for( guint i = 0; i < COL_VISIBLE_END; i++ ){
        
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
// ソート実行
//
void BoardView::exec_sort()
{
    if( m_col < 0 || m_col >= COL_VISIBLE_END ){
        m_col = COL_ID;
        m_sortmode = SORTMODE_ASCEND;
    }

    m_liststore->set_sort_column( -2, Gtk::SORT_ASCENDING );
    m_liststore->set_sort_column( m_col, Gtk::SORT_ASCENDING );
    CORE::core_set_command( "switch_board" );
}


//
// ヘッダをクリックしたときのslot関数
//
void BoardView::slot_col_clicked( int col )
{
#ifdef _DEBUG
    std::cout << "BoardView::slot_col_clicked col = " << col << std::endl;
#endif

    if( m_col != col ){ // 前回クリックした列と違う列をクリックした

        m_previous_col = m_col;
        m_previous_sortmode = m_sortmode;

        m_col = col;

        if( m_col == COL_MARK ) m_sortmode = SORTMODE_MARK1;
        else if( m_col == COL_ID ) m_sortmode = SORTMODE_ASCEND;
        else if( m_col == COL_SUBJECT ) m_sortmode = SORTMODE_ASCEND;
        else m_sortmode = SORTMODE_DESCEND;
    }
    else if( m_sortmode == SORTMODE_DESCEND ) m_sortmode = SORTMODE_ASCEND;
    else if( m_sortmode == SORTMODE_ASCEND ) m_sortmode = SORTMODE_DESCEND;
    else if( m_sortmode == SORTMODE_MARK1 ) m_sortmode = SORTMODE_MARK2;
    else if( m_sortmode == SORTMODE_MARK2 ) m_sortmode = SORTMODE_MARK3;
    else if( m_sortmode == SORTMODE_MARK3 ) m_sortmode = SORTMODE_MARK1;

    // 旧バージョンとの互換性のため
    if( m_col == COL_MARK ){
        if( m_sortmode == SORTMODE_DESCEND || m_sortmode == SORTMODE_ASCEND ) m_sortmode = SORTMODE_MARK1;
    }

    DBTREE::board_set_view_sort_column( get_url(), m_col );
    DBTREE::board_set_view_sort_mode( get_url(), m_sortmode );
    DBTREE::board_set_view_sort_pre_column( get_url(), m_previous_col );
    DBTREE::board_set_view_sort_pre_mode( get_url(), m_previous_sortmode );

    exec_sort();
}


//
// 抽出状態で比較
//
// row_a が上か　row_b　が上かを返す。同じ状態なら 0
//
int BoardView::compare_drawbg( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    bool draw_a = row_a[ m_columns.m_col_drawbg ];
    bool draw_b = row_b[ m_columns.m_col_drawbg ];

    if( draw_a && ! draw_b ) return COL_A_UP;
    else if( draw_b && ! draw_a ) return COL_B_UP;

    return 0;
}


//
// 列の値によるソート
//
// row_a が上か　row_b　が上かを返す。同じなら 0
//
int BoardView::compare_col( int col, int sortmode, Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b )
{
    int num_a = 0, num_b = 0;
    int ret = 0;

    const int UP = 1;
    const int DOWN = 2;

    switch( col ){

        case COL_MARK:
        {
            num_a = row_a[ m_columns.m_col_mark_val ];
            num_b = row_b[ m_columns.m_col_mark_val ];

            if( sortmode == SORTMODE_MARK2 ){ // 新着を一番上に

                if( num_a == COL_MARKVAL_NEWTHREAD
                    && ( num_b != COL_MARKVAL_NEWTHREAD && num_b != COL_MARKVAL_BKMARKED_UPDATED && num_b != COL_MARKVAL_BKMARKED ) ){
                    num_a = DOWN; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = UP;
                }
                else if( num_b == COL_MARKVAL_NEWTHREAD
                         && ( num_a != COL_MARKVAL_NEWTHREAD && num_a != COL_MARKVAL_BKMARKED_UPDATED && num_a != COL_MARKVAL_BKMARKED ) ){
                    num_a = UP; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = DOWN;
                }
            }

            break;
        }

        case COL_ID:
            num_a = row_a[ m_columns.m_col_id ];
            num_b = row_b[ m_columns.m_col_id ];
            break;

        case COL_SUBJECT:
        {
            // 文字列の大小を数字に変換
            Glib::ustring name_a = row_a[ m_columns.m_col_subject ]; 
            Glib::ustring name_b = row_b[ m_columns.m_col_subject ]; 
            if( name_a < name_b ) { num_a = UP; num_b = DOWN; }
            else if( name_a > name_b ) { num_a = DOWN; num_b = UP; }
            break;
        }

        case COL_RES:
            num_a = row_a[ m_columns.m_col_res ];
            num_b = row_b[ m_columns.m_col_res ];
            break;

        case COL_STR_LOAD:
            num_a = row_a[ m_columns.m_col_load ];
            num_b = row_b[ m_columns.m_col_load ];
            break;

        case COL_STR_NEW:
            num_a = row_a[ m_columns.m_col_new ];
            num_b = row_b[ m_columns.m_col_new ];
            break;

        case COL_SINCE:
            num_a = row_a[ m_columns.m_col_since_t ];
            num_b = row_b[ m_columns.m_col_since_t ];
            break;

        case COL_WRITE:
            num_a = row_a[ m_columns.m_col_write_t ];
            num_b = row_b[ m_columns.m_col_write_t ];
            break;

        case COL_SPEED:
            num_a = row_a[ m_columns.m_col_speed ];
            num_b = row_b[ m_columns.m_col_speed ];
            break;
    }

    // 両方とも 0 より大きいか 0 より小さい場合は普通に比較
    if( ( num_a > 0 && num_b > 0 ) || ( num_a < 0 && num_b < 0 ) ){

        if( num_a < num_b ) ret = COL_A_UP;
        else if( num_a > num_b ) ret = COL_B_UP;

        if( sortmode == SORTMODE_DESCEND ) ret *= -1;
        if( sortmode == SORTMODE_MARK1 || sortmode == SORTMODE_MARK2 ) ret *= -1;
    }

    // 0より大きい方を優先
    else if( num_a > 0 && num_b <= 0 ) ret = COL_A_UP;
    else if( num_b > 0 && num_a <= 0 ) ret = COL_B_UP;

    // 0を優先
    else if( num_a == 0 && num_b < 0 ) ret = COL_A_UP;
    else if( num_b == 0 && num_a < 0 ) ret = COL_B_UP;

    return ret;
}


//
// ソート関数
//
int BoardView::slot_compare_row( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
{
    Gtk::TreeModel::Row row_a = *( a );
    Gtk::TreeModel::Row row_b = *( b );

    // 抽出状態を最優先
    int ret = compare_drawbg( row_a, row_b ); 

    if( ! ret ) ret = compare_col( m_col, m_sortmode, row_a, row_b );

    // マルチキーソート
    if( ! ret ) ret = compare_col( m_previous_col, m_previous_sortmode, row_a, row_b );  

    if( ! ret ) ret = compare_col( COL_MARK, SORTMODE_ASCEND, row_a, row_b );
    if( ! ret ) ret = compare_col( COL_ID, SORTMODE_ASCEND, row_a, row_b );

    return ret;
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
    // オートリロードのカウンタを0にする
    View::reset_autoreload_counter();

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
        BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );
        return;
    }

    // タイトル表示
    set_title( DBTREE::board_name( get_url() ) );
    BOARD::get_admin()->set_command( "set_title", get_url(), get_title() );

    // タブにアイコンを表示
    BOARD::get_admin()->set_command( "set_tabicon", get_url(), "loading" );

    // タブに名前をセット
    BOARD::get_admin()->set_command( "set_tablabel", get_url(), DBTREE::board_name( get_url() ) );

#if GTKMMVER >= 280
    m_treeview.unset_model();
#endif
    m_liststore->clear();
    m_pre_query = std::string();
    
    // download 開始
    // 終わったら update_view() が呼ばれる
    DBTREE::board_download_subject( get_url() );
    set_status( "loading..." );
    BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );
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
    m_col = DBTREE::board_view_sort_column( get_url() );
    m_sortmode = DBTREE::board_view_sort_mode( get_url() );

    m_previous_col = DBTREE::board_view_sort_pre_column( get_url() );
    m_previous_sortmode = DBTREE::board_view_sort_pre_mode( get_url() );

    exec_sort();

    goto_top();
}



//
// 色、フォントの更新
//
void BoardView::relayout()
{
    m_treeview.init_color( COLOR_BACK_BOARD );
    m_treeview.init_font( CONFIG::get_fontname( FONT_BOARD ) );
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

    // 更新されたか
    bool updated = false;

    // 画面消去
#if GTKMMVER >= 280
    m_treeview.unset_model();
#endif
    m_liststore->clear();

    // 高速化のためデータベースに直接アクセス
    std::list< DBTREE::ArticleBase* >& list_subject = DBTREE::board_list_subject( get_url() );

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

            if( art->get_status() & STATUS_NORMAL )
                row[ m_columns.m_col_speed ] = art->get_speed();
        
            row[ m_columns.m_col_since_t ] = art->get_since_time();
            row[ m_columns.m_col_id_dat ] = art->get_id();

            update_row_common( art, row, id );

            // 更新があるかチェック
            if( art->get_number_load() && art->get_number() > art->get_number_load() ) updated = true;
        }

#if GTKMMVER >= 280
        m_treeview.set_model( m_liststore );
#endif
        redraw_view();
    }

    // ステータスバー更新
    std::ostringstream ss_tmp;
    ss_tmp << DBTREE::board_str_code( get_url() ) << " [ 全 " << ( id -1 ) << " ] ";
    set_status( ss_tmp.str() );
    BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );

    // タブのアイコン状態を更新
    int code = DBTREE::board_code( get_url() );

    // オートリロードモードでロード待ち
    if( View::get_autoreload_mode() != AUTORELOAD_NOT ){
        BOARD::get_admin()->set_command( "set_tabicon", get_url(), "loading_stop" );
    }
    // 更新あり   
    else  if( ( code == HTTP_OK || code == HTTP_PARTIAL_CONTENT ) && updated ){
        BOARD::get_admin()->set_command( "set_tabicon", get_url(), "update" );
    }
    // 通常
    else BOARD::get_admin()->set_command( "set_tabicon", get_url(), "default" );
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
            CORE::core_set_command( "switch_leftview" );
            break;

            // 現在の記事を表示
        case CONTROL::Right:
            CORE::core_set_command( "switch_rightview" );
            break;

        case CONTROL::ScrollLeft:
            scroll_left();
            break;

        case CONTROL::ScrollRight:
            scroll_right();
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
            SKELETON::MsgDiag mdiag( NULL, "選択した行のログを削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() != Gtk::RESPONSE_OK ) return;
            delete_view();
            break;
        }

        // ポップアップメニュー表示
        case CONTROL::ShowPopupMenu:
            show_popupmenu( "", true );
            break;

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
// 指定したIDのスレに移動
//
void BoardView::goto_num( int num )
{
    if( ! num ) return;

    focus_view();
   
    Gtk::TreeModel::Children child = m_liststore->children();
    Gtk::TreeModel::Children::iterator it = child.begin();
    for( ; it != child.end() ; ++it ){

        Gtk::TreeModel::Row row = *( it );
        
        if( row[ m_columns.m_col_id ] == num ){

            Gtk::TreePath path = GET_PATH( row );
            m_treeview.scroll_to_row( path );
            m_treeview.set_cursor( path );
            return;
        }
    }
}



//
// 左スクロール
//
void BoardView::scroll_left()
{
    Gtk::Adjustment*  hadjust = m_scrwin.get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value( MAX( 0,  hadjust->get_value() - hadjust->get_step_increment() ) );
}


//
// 右スクロール
//
void BoardView::scroll_right()
{
    Gtk::Adjustment*  hadjust = m_scrwin.get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value(  MIN( hadjust->get_upper() - hadjust->get_page_size(),
                              hadjust->get_value() + hadjust->get_step_increment() ) );
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
        m_path_selected = * (m_treeview.get_selection()->get_selected_rows().begin() );
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

    int mark_val;

    // ブックマーク & 新着あり
    if( art->is_bookmarked_thread() && art->enable_load() ){
        mark_val = COL_MARKVAL_BKMARKED_UPDATED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::BKMARK_UPDATE );
    }

    // ブックマーク
    else if( art->is_bookmarked_thread() ){
        mark_val = COL_MARKVAL_BKMARKED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::BKMARK );
    }

    // dat落ち
    else if( art->get_status() & STATUS_OLD ){
        mark_val = COL_MARKVAL_OLD;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::DOWN );
    }

    // キャッシュはあるが規定のレス数を越えていて全てのレスが既読
    else if( art->is_finished() ){
        mark_val = COL_MARKVAL_FINISHED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::CHECK );
    }

    // 新着あり
    else if( art->is_cached() && art->enable_load() ){
        mark_val = COL_MARKVAL_UPDATED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::UPDATE );
    }
    // キャッシュあり、新着無し
    else if( art->is_cached() ){
        mark_val = COL_MARKVAL_CACHED;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::CHECK );
    }
    // キャッシュ無し、新着
    else if( art->get_hour() < CONFIG::get_newthread_hour() ){
        mark_val = COL_MARKVAL_NEWTHREAD;
        row[ m_columns.m_col_mark ] = ICON::get_icon( ICON::NEWTHREAD );
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
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    // ダブルクリック
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

    CORE::core_set_command( "switch_board" );

    return true;
}



//
// マウスボタン離した
//
bool BoardView::slot_button_release( GdkEventButton* event )
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

    int x = (int)event->x;
    int y = (int)event->y;
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int cell_x;
    int cell_y;
   
    // 座標からpath取得
    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) ){

        m_path_selected = path;

#ifdef _DEBUG        
        std::cout << "BoardView::slot_button_press : path = " << path.to_string()
                  << " x = " << x << " y = " << y
                  << " cellheight = " << m_treeview.get_row_height() 
                  << " cell_x = " << cell_x << " cell_y = " << cell_y << std::endl;
#endif

        // リサイズするときにラベルをクリックすると行を開く問題の対処
        // かなりその場しのぎな方法なのでGTKのバージョンが上がったら誤動作するかも
        if( x == cell_x && y < m_treeview.get_row_height() ) return true;

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

        // スレを開く
        bool openarticle = get_control().button_alloted( event, CONTROL::OpenArticleButton );
        bool openarticletab = get_control().button_alloted( event, CONTROL::OpenArticleTabButton );

        if( openarticle || openarticletab ){

            // 複数行選択中
            if( m_treeview.get_selected_iterators().size() >= 2 ) open_selected_rows();

            else open_row( path, openarticletab );
        }

        // ポップアップメニューボタン
        else if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){

            show_popupmenu( "", false );
        }

        else operate_view( get_control().button_press( event ) );

        event->type = type_copy;
    }

    return true;
}



//
// マウス動かした
//
bool BoardView::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

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
    int key = get_control().key_press( event );

    if( key != CONTROL::None ){

        // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
        // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
        // OpenArticle(Tab)は slot_key_release() で処理する
        if( key == CONTROL::OpenArticle ) return true;
        if( key == CONTROL::OpenArticleTab ) return true;

        operate_view( key );
    }
    else release_keyjump_key( event->keyval );

    return true;
}


//
// キーリリースイベント
//
bool BoardView::slot_key_release( GdkEventKey* event )
{
    int key = get_control().key_press( event );

    // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
    // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
    // OpenArticle(Tab)はここで処理する
    if( key == CONTROL::OpenArticle ) operate_view( key );
    if( key == CONTROL::OpenArticleTab ) operate_view( key );
   
    return true;
}



//
// マウスホイールイベント
//
bool BoardView::slot_scroll_event( GdkEventScroll* event )
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
// ブックマーク設定、解除
//
void BoardView::slot_bookmark( int bookmark )
{
    std::string datbase = DBTREE::url_datbase( get_url() );
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){
        Gtk::TreeModel::Row row = *( *it );
        std::string url = datbase + row[ m_columns.m_col_id_dat ];
        DBTREE::ArticleBase* art = DBTREE::get_article( url );
        if( art ){
            bool set = bookmark;
            if( bookmark == BOOKMARK_AUTO ) set = ! art->is_bookmarked_thread();
#ifdef _DEBUG
            std::cout << "BoardView::slot_bookmark url = " << url << " set = " << set << std::endl;
#endif
            art->set_bookmarked_thread( set );
        }
    }
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
    show_popupmenu( "popup_menu_delete", false );
}


//
// ツールバーのお気に入りボタン
//
void BoardView::slot_push_favorite()
{
    show_popupmenu( "popup_menu_favorite", false );
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
        else if( regex.exec( query, subject, 0, true, true, true ) ){
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
        if( regex.exec( query, subject, 0, true, true, true ) ){
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


// キャッシュ検索
void BoardView::slot_search_cache()
{
    std::string query = m_toolbar.m_entry_search.get_text();
    CORE::core_set_command( "open_article_searchcache", get_url() , query, "false" );
}


//
// 検索entryの操作
//
void BoardView::slot_entry_operate( int controlid )
{
    if( controlid == CONTROL::Cancel ) focus_view();
    else if( controlid == CONTROL::SearchCache ) slot_search_cache();
}


//
// 板プロパティ表示
//
void BoardView::slot_push_preferences()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARD, get_url() );
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLE, url );
    pref->run();
    delete pref;
}


//
// datを保存
//
void BoardView::slot_save_dat()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2daturl( m_path_selected );

    DBTREE::article_save_dat( url, std::string() );
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
