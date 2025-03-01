// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "boardadmin.h"
#include "boardview.h"

#include "skeleton/msgdiag.h"
#include "skeleton/filediag.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "config/globalconf.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "command.h"
#include "cache.h"
#include "session.h"
#include "sharedbuffer.h"
#include "global.h"
#include "type.h"
#include "prefdiagfactory.h"
#include "httpcode.h"
#include "colorid.h"
#include "fontid.h"
#include "compmanager.h"
#include "dndmanager.h"

#include "icons/iconmanager.h"

#include <sstream>

using namespace BOARD;


// row -> path
#define GET_PATH( row ) m_liststore->get_path( row )



enum{
    CANCEL_OPENROW = 500,  // msec  連続クリック防止用カウンタ
    DEFAULT_COLMUN_WIDTH = 50
};


enum{
    COL_MARKVAL_OLD = -2,        // dat 落ち
    COL_MARKVAL_FINISHED = -1,   // キャッシュあり、新着無し、規定スレ数を越えている
    COL_MARKVAL_NORMAL = 0,      // 通常状態、キャッシュ無し、
    COL_MARKVAL_924,             // スレッド924、キャッシュ無し
    COL_MARKVAL_NEWTHREAD_HOUR,  // 新スレ( CONFIG::get_newthread_hour 時間以内 )、キャッシュ無し
    COL_MARKVAL_NEWTHREAD,       // 前回の板一覧読み込み時から新しく出来たスレ、キャッシュ無し
    COL_MARKVAL_CACHED,          // キャッシュあり、新着無し
    COL_MARKVAL_BROKEN_SUBJECT,  // キャッシュあり、新着無しだが subject.txt が壊れている可能性がある
    COL_MARKVAL_UPDATED,         // キャッシュあり、新着有り
    COL_MARKVAL_BKMARKED,        // ブックマークされている、新着無し
    COL_MARKVAL_BKMARKED_BROKEN_SUBJECT, // ブックマークされている、新着無しだが subject.txt が壊れている可能性がある
    COL_MARKVAL_BKMARKED_UPDATED // ブックマークされている、新着有り
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



#define DELETE_COLUMN(col) do{ if( ! col ) { delete col; col = nullptr; } }while(0)

// set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) を指定して append_columnする
#define APPEND_COLUMN(col,title,model) do{                    \
col = Gtk::manage( new Gtk::TreeViewColumn( title, model ) );   \
col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ); \
m_treeview.append_column( *col ); \
}while(0)


BoardViewBase::BoardViewBase( const std::string& url, const bool show_col_board )
    : SKELETON::View( url )
    , m_treeview( url, DNDTARGET_FAVORITE, true, CONFIG::get_fontname( FONT_BOARD ),
                  COLOR_CHAR_BOARD, COLOR_BACK_BOARD, COLOR_BACK_BOARD_EVEN )
    , m_col( COL_NUM_COL )
    , m_previous_col( COL_NUM_COL )
    , m_sortmode( SORTMODE_ASCEND )
    , m_enable_menuslot( true )
    , m_load_subject_txt( true )
    , m_show_col_board( show_col_board )
{
    // 次スレ検索ビューのようにURLの途中に http が入っている場合は取り除く
    size_t pos = url.rfind( "http://" );
    if( pos == std::string::npos || pos == 0 ) pos = url.rfind( "https://" );
    if( pos != std::string::npos && pos != 0 ) m_url_board = DBTREE::url_boardbase( url.substr( 0, pos ) );
    else m_url_board = DBTREE::url_boardbase( url );

    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );

    pack_start( m_scrwin );
    show_all_children();

    // ツリービュー設定
    m_liststore = Gtk::ListStore::create( m_columns );

    // セルを固定の高さにする
    // append_column する前に columnに対して set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED ) すること
    m_treeview.set_fixed_height_mode( true );

#ifdef _DEBUG
    std::cout << "BoardViewBase::BoardViewBase : m_treeview.set_fixed_height_mode\n";
#endif

    // 列のappend
    update_columns();

    // ソート関数セット
    m_liststore->set_sort_func( COL_MARK, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_ID, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_BOARD, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SUBJECT, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_RES, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_STR_LOAD, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_STR_NEW, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SINCE, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_WRITE, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_ACCESS, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_SPEED, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );
    m_liststore->set_sort_func( COL_DIFF, sigc::mem_fun( *this, &BoardViewBase::slot_compare_row ) );

    m_treeview.add_events( Gdk::SMOOTH_SCROLL_MASK );

    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &BoardViewBase::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &BoardViewBase::slot_button_release ) );
    m_treeview.sig_motion_notify().connect( sigc::mem_fun(*this, &BoardViewBase::slot_motion_notify ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BoardViewBase::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BoardViewBase::slot_key_release ) );
    m_treeview.sig_scroll_event().connect( sigc::mem_fun(*this, &BoardViewBase::slot_scroll_event ) );
    m_treeview.set_has_tooltip( true );
    m_treeview.signal_query_tooltip().connect( sigc::mem_fun(*this, &BoardViewBase::slot_query_tooltip) );
    m_treeview.signal_drag_data_get().connect( sigc::mem_fun(*this, &BoardViewBase::slot_drag_data_get ) );
    m_treeview.sig_dropped_uri_list().connect( sigc::mem_fun(*this, &BoardViewBase::slot_dropped_url_list ) );

    // アクション初期化
    setup_action();

    // マウスジェスチャ可能
    set_enable_mg( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_BOARD );
}


BoardViewBase::~BoardViewBase() noexcept
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::~BoardViewBase : " << get_url() << std::endl;
#endif
}


SKELETON::Admin* BoardViewBase::get_admin()
{
    return BOARD::get_admin();
}


//
// url 更新
//
// 移転があったときなどにadminから呼び出される
//
void BoardViewBase::update_url( const std::string& url_old, const std::string& url_new )
{
    if( m_url_board.rfind( url_old, 0 ) == 0 ) m_url_board = url_new + m_url_board.substr( url_old.length() );

    SKELETON::View::update_url( url_old, url_new );
}


// アイコンのID取得
int BoardViewBase::get_icon( const std::string& iconname ) const
{
    int id = ICON::NONE;

    if( iconname == "default" ) id = ICON::BOARD;
    if( iconname == "loading" ) id = ICON::LOADING;
    if( iconname == "loading_stop" ) id = ICON::LOADING_STOP;
    if( iconname == "update" ) id = ICON::BOARD_UPDATE;  // 更新チェックして更新があった場合
    if( iconname == "updated" ) id = ICON::BOARD_UPDATED;

#ifdef _DEBUG
    std::cout << "BoardViewBase::get_icon : " << iconname << " url = " << get_url() << std::endl;
#endif

    return id;
}


//
// コピー用URL(メインウィンドウのURLバーなどに表示する)
//
std::string BoardViewBase::url_for_copy() const
{
    return get_url_board();
}


//
// アクション初期化
//
void BoardViewBase::setup_action()
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::setup_action\n";
#endif

    // アクショングループを作成
    m_action_group = Gio::SimpleActionGroup::create();
    m_action_group->add_action( "BookMark",
                                 sigc::bind( sigc::mem_fun( *this, &BoardViewBase::slot_bookmark ), BOOKMARK_AUTO ) );
    m_action_group->add_action( "SetBookMark",
                                sigc::bind( sigc::mem_fun( *this, &BoardViewBase::slot_bookmark ), BOOKMARK_SET ) );
    m_action_group->add_action( "UnsetBookMark",
                                sigc::bind( sigc::mem_fun( *this, &BoardViewBase::slot_bookmark ), BOOKMARK_UNSET ) );
    m_action_group->add_action( "OpenTab", sigc::mem_fun( *this, &BoardViewBase::slot_open_tab ) );
    m_action_group->add_action( "RegetArticle", sigc::mem_fun( *this, &BoardViewBase::slot_reget_article ) );
    m_action_group->add_action( "FavoriteArticle", sigc::mem_fun( *this, &BoardViewBase::slot_favorite_thread ) );
    m_action_group->add_action( "FavoriteBoard", sigc::mem_fun( *this, &BoardViewBase::slot_favorite_board ) );
    m_action_group->add_action( "GotoTop", sigc::mem_fun( *this, &BoardViewBase::goto_top ) );
    m_action_group->add_action( "GotoBottom", sigc::mem_fun( *this, &BoardViewBase::goto_bottom ) );
    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &BoardViewBase::slot_delete_logs ) );
    m_action_group->add_action( "OpenRows",
                                sigc::bind( sigc::mem_fun( *this, &BoardViewBase::open_selected_rows ), false ) );
    m_action_group->add_action( "RegetRows",
                                sigc::bind( sigc::mem_fun( *this, &BoardViewBase::open_selected_rows ), true ) );
    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &BoardViewBase::slot_copy_url ) );
    m_action_group->add_action( "CopyTitleURL", sigc::mem_fun( *this, &BoardViewBase::slot_copy_title_url ) );
    m_action_group->add_action( "OpenBrowser", sigc::mem_fun( *this, &BoardViewBase::slot_open_browser ) );
    m_action_group->add_action( "AboneThread", sigc::mem_fun( *this, &BoardViewBase::slot_abone_thread ) );
    m_action_group->add_action( "PreferenceArticle", sigc::mem_fun( *this, &BoardViewBase::slot_preferences_article ) );
    m_action_group->add_action( "PreferenceBoard", sigc::mem_fun( *this, &BoardViewBase::show_preference ) );
    m_action_group->add_action( "SaveDat", sigc::mem_fun( *this, &BoardViewBase::slot_save_dat ) );
    m_action_group->add_action( "SearchNextArticle", sigc::mem_fun( *this, &BoardViewBase::slot_search_next ) );

    insert_action_group( "board", m_action_group );

    Glib::ustring menu_ui = R"(
<interface>
  <!-- 複数選択した時のメニュー -->
  <menu id="popup_menu_mul">
    <section>
      <item>
        <attribute name="label" translatable="yes">選択したスレを開く(_O)</attribute>
        <attribute name="action">board.OpenRows</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">スレ情報を消さずにスレを再取得(_R)</attribute>
        <attribute name="action">board.RegetRows</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">しおりを設定(_S)</attribute>
        <attribute name="action">board.SetBookMark</attribute>
      </item>
      <item>
        <attribute name="label" translatable="yes">しおりを解除(_U)</attribute>
        <attribute name="action">board.UnsetBookMark</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_SAVE_DAT R"((_S)...</attribute>
        <attribute name="action">board.SaveDat</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_FAVORITE_ARTICLE R"((_F)...</attribute>
        <attribute name="action">board.FavoriteArticle</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="label" translatable="yes">)" ITEM_NAME_ABONE_ARTICLE R"((_N)...</attribute>
        <attribute name="action">board.AboneThread</attribute>
      </item>
    </section>
    <section>
      <submenu>
        <attribute name="label" translatable="yes">)" ITEM_NAME_DELETE R"((_D)</attribute>
        <item>
          <attribute name="label" translatable="yes">選択した行のログを削除する(_D)</attribute>
          <attribute name="action">board.Delete</attribute>
        </item>
      </submenu>
    </section>
  </menu>
  <!-- お気に入りボタン押した時のメニュー -->
  <menu id="popup_menu_favorite">
    <item>
      <attribute name="label" translatable="yes">)" ITEM_NAME_FAVORITE_ARTICLE R"((_F)...</attribute>
      <attribute name="action">board.FavoriteArticle</attribute>
    </item>
    <item>
      <attribute name="label" translatable="yes">板をお気に入りに追加(_A)</attribute>
      <attribute name="action">board.FavoriteBoard</attribute>
    </item>
  </menu>
  <!-- お気に入りボタン押した時のメニュー( スレのみ ) -->
  <menu id="popup_menu_favorite_article">
    <item>
      <attribute name="label" translatable="yes">)" ITEM_NAME_FAVORITE_ARTICLE R"((_F)...</attribute>
      <attribute name="action">board.FavoriteArticle</attribute>
    </item>
  </menu>
  <!-- 削除ボタン押した時のメニュー -->
  <menu id="popup_menu_delete">
    <item>
      <attribute name="label" translatable="yes">選択した行のログを削除する(_D)</attribute>
      <attribute name="action">board.Delete</attribute>
    </item>
  </menu>
</interface>)";

    auto builder = Gtk::Builder::create_from_string( menu_ui );

    m_popup_menu.bind_model( create_context_menu(), true );
    m_popup_menu.attach_to_widget( *this );

    // アクセラレーターキーやマウスジェスチャーの追加は行わない
    // CONTROL::set_menu_motion() はGTK4で廃止されるGtk::MenuのAPIを使っている
    setup_popupmenu( m_popup_menu_mul, builder->get_object( "popup_menu_mul" ) );
    setup_popupmenu( m_popup_menu_delete, builder->get_object( "popup_menu_delete" ) );
    setup_popupmenu( m_popup_menu_favorite_article, builder->get_object( "popup_menu_favorite_article" ) );
    setup_popupmenu( m_popup_menu_favorite, builder->get_object( "popup_menu_favorite" ) );
}


void BoardViewBase::setup_popupmenu( Gtk::Menu& menu, const Glib::RefPtr<Glib::Object>& model )
{
    menu.bind_model( Glib::RefPtr<Gio::MenuModel>::cast_dynamic( model ), true );
    menu.attach_to_widget( *this );
}


//
// 通常の右クリックメニューの作成
//
Glib::RefPtr<Gio::MenuModel> BoardViewBase::create_context_menu() const
{
    std::list< int > list_menu;

    list_menu.push_back( ITEM_BOOKMARK );
    list_menu.push_back( ITEM_OPENARTICLETAB );
    list_menu.push_back( ITEM_REGETARTICLE );
    list_menu.push_back( ITEM_OPEN_BROWSER );
    list_menu.push_back( ITEM_COPY_URL );
    list_menu.push_back( ITEM_COPY_TITLE_URL_THREAD );
    list_menu.push_back( ITEM_SAVE_DAT );
    list_menu.push_back( ITEM_FAVORITE_ARTICLE );
    list_menu.push_back( ITEM_NEXTARTICLE );
    list_menu.push_back( ITEM_ABONE_ARTICLE );
    list_menu.push_back( ITEM_DELETE );
    list_menu.push_back( ITEM_PREF_THREAD );
    list_menu.push_back( ITEM_PREF_BOARD );

    // メニューに含まれていない項目を抜き出して「その他」に含める
    for( int num = 0; ; ++num ) {

        const int item = SESSION::get_item_board_menu( num );
        if( item == ITEM_END ) break;

        list_menu.remove( item );
    }

    auto menumodel = Gio::Menu::create();
    auto section = Gio::Menu::create();
    for( int num = 0; ; ++num ) {

        const int item = SESSION::get_item_board_menu( num );
        if( item == ITEM_END ) break;

        if( item == ITEM_ETC && list_menu.size() ) {
            auto etc_section = Gio::Menu::create();
            for( int i : list_menu ) {
                add_menu_item( i, etc_section );
            }
            section->append_submenu( ITEM_NAME_ETC "(_O)", etc_section );
        }
        else {
            const bool new_section = add_menu_item( item, section );
            if( new_section ) {
                menumodel->append_section( section );
                section = Gio::Menu::create();
            }
        }
    }

    if( section->get_n_items() > 0 ) {
        menumodel->append_section( section );
    }

    return menumodel;
}


bool BoardViewBase::add_menu_item( const int item, const Glib::RefPtr<Gio::Menu>& section ) const
{
    bool new_section = false;
    Glib::RefPtr<Gio::MenuItem> menuitem;

    switch( item ){

        // しおりを設定/解除
        case ITEM_BOOKMARK:
            section->append( ITEM_NAME_BOOKMARK "(_B)", "board.BookMark" );
            break;

            // タブでスレを開く
        case ITEM_OPENARTICLETAB:
            menuitem = Gio::MenuItem::create( ITEM_NAME_OPENARTICLETAB "(_T)", "board.OpenTab" );
            section->append_item( CONTROL::set_accel_abbrev( menuitem, CONTROL::OpenArticleTab ) );
            break;

            // スレ情報を消さずに再取得"
        case ITEM_REGETARTICLE:
            section->append( ITEM_NAME_REGETARTICLE "(_R)", "board.RegetArticle" );
            break;

            // リンクをブラウザで開く
        case ITEM_OPEN_BROWSER:
            section->append( ITEM_NAME_OPEN_BROWSER "(_W)", "board.OpenBrowser" );
            break;

            // リンクのURLをコピー
        case ITEM_COPY_URL:
            section->append( ITEM_NAME_COPY_URL "(_U)", "board.CopyURL" );
            break;

            // スレのタイトルとURLをコピー
        case ITEM_COPY_TITLE_URL_THREAD:
            section->append( ITEM_NAME_COPY_TITLE_URL "(_L)", "board.CopyTitleURL" );
            break;

            // dat 保存
        case ITEM_SAVE_DAT:
            menuitem = Gio::MenuItem::create( ITEM_NAME_SAVE_DAT "(_S)", "board.SaveDat" );
            section->append_item( CONTROL::set_accel_abbrev( menuitem, CONTROL::SaveDat ) );
            break;

            // スレをお気に入りに追加
        case ITEM_FAVORITE_ARTICLE:
            section->append( ITEM_NAME_FAVORITE_ARTICLE "(_F)", "board.FavoriteArticle" );
            break;

            // 次スレ検索
        case ITEM_NEXTARTICLE:
            section->append( ITEM_NAME_NEXTARTICLE, "board.SearchNextArticle" );
            break;

            // スレをあぼ〜んする"
        case ITEM_ABONE_ARTICLE:
            section->append( ITEM_NAME_ABONE_ARTICLE "(_N)", "board.AboneThread" );
            break;

            // 削除
        case ITEM_DELETE:
            {
                // Gio::Menuはサブメニューの枝にアクセラレーターキーを設定できない
                menuitem = Gio::MenuItem::create( "選択した行のログを削除する(_D)", "board.Delete" );
                auto submenu = Gio::Menu::create();
                submenu->append_item( CONTROL::set_accel_abbrev( menuitem, CONTROL::Delete ) );
                section->append_submenu( ITEM_NAME_DELETE "(_D)", submenu );
            }
            break;

            // スレのプロパティ
        case ITEM_PREF_THREAD:
            section->append( ITEM_NAME_PREF_THREAD "(_P)", "board.PreferenceArticle" );
            break;

            // 板のプロパティ
        case ITEM_PREF_BOARD:
            menuitem = Gio::MenuItem::create( ITEM_NAME_PREF_BOARD "(_O)...", "board.PreferenceBoard" );
            section->append_item( CONTROL::set_accel_abbrev( menuitem, CONTROL::PreferenceBoard ) );
            break;

            // 区切り
        case ITEM_SEPARATOR:
            new_section = true;
            break;
    }
    return new_section;
}


//
// 行数
//
int BoardViewBase::get_row_size() const
{
    return m_treeview.get_row_size();
}


//
// 自動ソート抑制
// -2 = DEFAULT_UNSORTED_COLUMN_ID
//
// 追加や更新などで列に値をセットする前に実行してしておかないと
// いちいちソートをかけるので極端に遅くなる
//
void BoardViewBase::unsorted_column()
{
    m_liststore->set_sort_column( Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID,
                                  Gtk::SortType::SORT_ASCENDING );
}


//
// url から row を取得
//
Gtk::TreeModel::Row BoardViewBase::get_row_from_url( const std::string& url ) const
{
    Gtk::TreeModel::Children child = m_liststore->children();
    for( Gtk::TreeModel::Row row : child ) {
        DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];
        if( url == art->get_url() ) return row;
    }

    return Gtk::TreeModel::Row();
}


//
// 列項目の更新
//
void BoardViewBase::update_columns()
{
    m_treeview.remove_all_columns();

    DELETE_COLUMN( m_col_id );
    DELETE_COLUMN( m_col_board );
    DELETE_COLUMN( m_col_subject );
    DELETE_COLUMN( m_col_res );
    DELETE_COLUMN( m_col_str_load );
    DELETE_COLUMN( m_col_str_new );
    DELETE_COLUMN( m_col_since );
    DELETE_COLUMN( m_col_write );
    DELETE_COLUMN( m_col_speed );
    DELETE_COLUMN( m_col_diff );
    DELETE_COLUMN( m_col_access );

    int num = 0;

    // 先頭に「板」列を追加
    if( m_show_col_board ){

        bool append_board = true;
        for(;;){
            const int item = SESSION::get_item_board_col( num );
            if( item == ITEM_BOARD ) append_board = false;
            if( item == ITEM_END ) break;
            num++;
        }
        if( append_board ) APPEND_COLUMN( m_col_board, ITEM_NAME_BOARD, m_columns.m_col_board );
    }

    m_col_diff_is_shown = false;
    num = 0;
    for(;;){
        const int item = SESSION::get_item_board_col( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_MARK: {
                    auto* col = Gtk::make_managed<Gtk::TreeViewColumn>( ITEM_NAME_MARK );
                    auto* render_pixbuf = Gtk::make_managed<Gtk::CellRendererPixbuf>();
                    col->pack_start( *render_pixbuf, false );
                    col->add_attribute( *render_pixbuf, "gicon", 0 );
                    col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
                    m_treeview.append_column( *col );
                    m_col_mark = col;
                }
                break;
            case ITEM_ID: APPEND_COLUMN( m_col_id, ITEM_NAME_ID, m_columns.m_col_id ); break;
            case ITEM_BOARD: APPEND_COLUMN( m_col_board, ITEM_NAME_BOARD, m_columns.m_col_board ); break;
            case ITEM_NAME: APPEND_COLUMN( m_col_subject, ITEM_NAME_NAME, m_columns.m_col_subject ); break;
            case ITEM_RES: APPEND_COLUMN( m_col_res, ITEM_NAME_RES, m_columns.m_col_res ); break;
            case ITEM_LOAD: APPEND_COLUMN( m_col_str_load, ITEM_NAME_LOAD, m_columns.m_col_str_load ); break;
            case ITEM_NEW: APPEND_COLUMN( m_col_str_new, ITEM_NAME_NEW, m_columns.m_col_str_new ); break;
            case ITEM_SINCE: APPEND_COLUMN( m_col_since, ITEM_NAME_SINCE, m_columns.m_col_since ); break;
            case ITEM_LASTWRITE: APPEND_COLUMN( m_col_write, ITEM_NAME_LASTWRITE, m_columns.m_col_write ); break;
            case ITEM_ACCESS: APPEND_COLUMN( m_col_access, ITEM_NAME_ACCESS, m_columns.m_col_access); break;
            case ITEM_SPEED: APPEND_COLUMN( m_col_speed, ITEM_NAME_SPEED, m_columns.m_col_speed ); break;
            case ITEM_DIFF: APPEND_COLUMN( m_col_diff, ITEM_NAME_DIFF, m_columns.m_col_diff); m_col_diff_is_shown = true; break;
        }
        ++num;
    }

    // 高さを取得するためのcolumn番号をリセット
    m_treeview.set_column_for_height( 0 );

    // サイズを調整しつつソートの設定
    for( guint i = 0; i < COL_VISIBLE_END; i++ ){

        const int id = get_title_id( i );
        if( id < 0 ) continue;

        Gtk::TreeView::Column* column = m_treeview.get_column( i );
        if( ! column ) continue;

        int width = 0;

        switch( id ){

        case COL_MARK:
            width = SESSION::col_mark();
            break;

        case COL_ID:
            width = SESSION::col_id();
            break;

        case COL_BOARD:
            width = SESSION::col_board();
            break;

        case COL_SUBJECT:
            width = SESSION::col_subject();
            m_treeview.set_column_for_height( i );
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

        case COL_ACCESS:
            width = SESSION::col_access();
            break;

        case COL_SPEED:
            width = SESSION::col_speed();
            break;

        case COL_DIFF:
            width = SESSION::col_diff();
            break;
        }

        if( ! width ) width = DEFAULT_COLMUN_WIDTH;

        column->set_fixed_width( width );
        column->set_resizable( true );
        column->set_clickable( true );

        // ヘッダをクリックしたときに呼ぶslot
        column->signal_clicked().connect( sigc::bind< int >( sigc::mem_fun( *this, &BoardViewBase::slot_col_clicked ), id ) );
        // 列の幅が変わったらセッション情報に記憶する
        column->connect_property_changed( "width", sigc::mem_fun( *this, &BoardViewBase::save_column_width ) );

        // ヘッダの位置
        switch( id ){
            case COL_MARK:
                column->set_alignment( 0.5 );
                break;

            case COL_ID:
            case COL_RES:
            case COL_STR_LOAD:
            case COL_STR_NEW:
            case COL_SPEED:
            case COL_DIFF:
                column->set_alignment( 1.0 );
                break;

            default:
                column->set_alignment( 0.0 );
                break;
        }

        Gtk::CellRenderer *cell = column->get_first_cell();

        // 実際の描画の際に cellrendere のプロパティをセットするスロット関数
        if( cell ) {
            auto slot_func{ id == COL_SUBJECT ? &BoardViewBase::slot_cell_data_markup
                                              : &BoardViewBase::slot_cell_data };
            column->set_cell_data_func( *cell, sigc::mem_fun( *this, slot_func ) );
        }

        Gtk::CellRendererText* rentext = dynamic_cast< Gtk::CellRendererText* >( cell );
        if( rentext ){

            // 列間スペース
            rentext->property_xpad() = 4;

            // 行間スペース
            rentext->property_ypad() = CONFIG::get_tree_ypad();

            // 文字位置
            switch( id ){
                case COL_ID:
                case COL_RES:
                case COL_STR_LOAD:
                case COL_STR_NEW:
                case COL_SPEED:
                case COL_DIFF:
                    rentext->property_xalign() = 1.0;
                    break;

                default:
                    rentext->property_xalign() = 0.0;
            }
        }
    }
}


//
// i列目のIDを取得
//
// 失敗の時は-1を変えす
//
int BoardViewBase::get_title_id( const int col ) const
{
    const Gtk::TreeView::Column* column = m_treeview.get_column( col );
    if( ! column ) return -1;

    const std::string title = column->get_title();
    int id = -1;

    if( title == ITEM_NAME_MARK ) id = COL_MARK;
    else if( title == ITEM_NAME_ID ) id = COL_ID;
    else if( title == ITEM_NAME_BOARD ) id = COL_BOARD;
    else if( title == ITEM_NAME_NAME ) id = COL_SUBJECT;
    else if( title == ITEM_NAME_RES ) id = COL_RES;
    else if( title == ITEM_NAME_LOAD ) id = COL_STR_LOAD;
    else if( title == ITEM_NAME_NEW ) id = COL_STR_NEW;
    else if( title == ITEM_NAME_SINCE ) id = COL_SINCE;
    else if( title == ITEM_NAME_LASTWRITE ) id = COL_WRITE;
    else if( title == ITEM_NAME_ACCESS ) id = COL_ACCESS;
    else if( title == ITEM_NAME_SPEED ) id = COL_SPEED;
    else if( title == ITEM_NAME_DIFF ) id = COL_DIFF;

    return id;
}



//
// ソート列やソートモードの保存
//
void BoardViewBase::save_sort_columns()
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::save_sort_columns : url = " << get_url() << std::endl;
#endif

    DBTREE::board_set_view_sort_column( get_url_board(), m_col );
    DBTREE::board_set_view_sort_mode( get_url_board(), m_sortmode );
    DBTREE::board_set_view_sort_pre_column( get_url_board(), m_previous_col );
    DBTREE::board_set_view_sort_pre_mode( get_url_board(), m_previous_sortmode );
}


//
// 列の幅の保存
//
void BoardViewBase::save_column_width()
{
#ifdef _DEBUG
    std::cout << "save_column_width " << get_url() << std::endl;
#endif

    for( guint i = 0; i < COL_VISIBLE_END; i++ ){

        const int id = get_title_id( i );
        if( id < 0 ) continue;

        Gtk::TreeView::Column* column = m_treeview.get_column( i );

        int width = 0;
        if( column ) width = column->get_width();
        if( ! width ) continue;

        switch( id ){

        case COL_MARK:
            SESSION::set_col_mark( width );
            break;

        case COL_ID:
            SESSION::set_col_id( width );
            break;

        case COL_BOARD:
            SESSION::set_col_board( width );
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

        case COL_ACCESS:
            SESSION::set_col_access( width );
            break;

        case COL_SPEED:
            SESSION::set_col_speed( width );
            break;

        case COL_DIFF:
            SESSION::set_col_diff( width );
            break;
        }
    }
}



//
// Subjectの実際の描画の際に cellrenderer のプロパティをセットするスロット関数
//
void BoardViewBase::slot_cell_data_markup( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;

    Gtk::CellRendererText* rentext = dynamic_cast<Gtk::CellRendererText*>( cell );

    // ハイライト色 ( 抽出状態 )
    if( row[ m_columns.m_col_drawbg ] ){
        rentext->property_foreground() = CONFIG::get_color( COLOR_CHAR_HIGHLIGHT_TREE );
        rentext->property_foreground_set() = true;
        rentext->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        rentext->property_cell_background_set() = true;
    }

    else {
        rentext->property_foreground_set() = false;
        m_treeview.slot_cell_data( cell, it );
    }

    rentext->property_text() = "";
    rentext->property_markup() = row[ m_columns.m_col_subject ];
}



//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void BoardViewBase::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    // scanbuild-19 のレポートを抑制するためnullチェックする
    // warning: Called C++ object pointer is null [core.CallAndMessage]
    if( ! cell ) return;

    Gtk::TreeModel::Row row = *it;
    // Gio::Icon を表示するcell (ITEM_MARK)は CellRendererText にキャストできない、nullptr が返る
    Gtk::CellRendererText* rentext = dynamic_cast<Gtk::CellRendererText*>( cell );

    // ハイライト色 ( 抽出状態 )
    if( row[ m_columns.m_col_drawbg ] ){
        if( rentext ) {
            rentext->property_foreground() = CONFIG::get_color( COLOR_CHAR_HIGHLIGHT_TREE );
            rentext->property_foreground_set() = true;
        }
        cell->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        cell->property_cell_background_set() = true;
    }

    else {
        if( rentext ) {
            rentext->property_foreground_set() = false;
        }
        m_treeview.slot_cell_data( cell, it );
    }
}



//
// ソート実行
//
void BoardViewBase::exec_sort()
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::exec_sort url = " << get_url() << std::endl;
#endif

    if( m_col < 0 || m_col >= COL_VISIBLE_END ){
        m_col = COL_ID;
        m_sortmode = SORTMODE_ASCEND;
    }

    m_liststore->set_sort_column( -2, Gtk::SORT_ASCENDING );
    m_liststore->set_sort_column( m_col, Gtk::SORT_ASCENDING );
}


//
// ソート状態回復
//
void BoardViewBase::restore_sort()
{
    m_search_invert = false;

    m_col = get_default_sort_column();
    m_sortmode = get_default_view_sort_mode();

    m_previous_col = get_default_view_sort_pre_column();
    m_previous_sortmode = get_default_view_sort_pre_mode();

    if( get_row_size() ){
        exec_sort();
        goto_top();
    }
}


//
// デフォルトのソート状態
//
int BoardViewBase::get_default_sort_column() const
{
    return DBTREE::board_view_sort_column( get_url_board() );
}

int BoardViewBase::get_default_view_sort_mode() const
{
    return DBTREE::board_view_sort_mode( get_url_board() );
}

int BoardViewBase::get_default_view_sort_pre_column() const
{
    return DBTREE::board_view_sort_pre_column( get_url_board() );
}

int BoardViewBase::get_default_view_sort_pre_mode() const
{
    return DBTREE::board_view_sort_pre_mode( get_url_board() );
}


//
// ヘッダをクリックしたときのslot関数
//
void BoardViewBase::slot_col_clicked( const int col )
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::slot_col_clicked col = " << col << std::endl;
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
    else if( m_sortmode == SORTMODE_MARK3 ) m_sortmode = SORTMODE_MARK4;
    else if( m_sortmode == SORTMODE_MARK4 ) m_sortmode = SORTMODE_MARK1;

    if( m_col == COL_MARK ){
        std::string info;
        if( m_sortmode == SORTMODE_MARK1 ) info = "モード 1";
        if( m_sortmode == SORTMODE_MARK2 ) info = "モード 2";
        if( m_sortmode == SORTMODE_MARK3 ) info = "モード 3";
        if( m_sortmode == SORTMODE_MARK4 ) info = "モード 4";
        CORE::core_set_command( "set_info", "", info );
    }

/*  そろそろ消しても良い?  問題があれば戻す

    // 旧バージョンとの互換性のため
    if( m_col == COL_MARK ){
        if( m_sortmode == SORTMODE_DESCEND || m_sortmode == SORTMODE_ASCEND ) m_sortmode = SORTMODE_MARK1;
    }
*/
    save_sort_columns();
    exec_sort();
    focus_view();
}


//
// 抽出状態で比較
//
// row_a が上か　row_b　が上かを返す。同じ状態なら 0
//
int BoardViewBase::compare_drawbg( const Gtk::TreeModel::Row& row_a, const Gtk::TreeModel::Row& row_b ) const
{
    const bool draw_a = row_a[ m_columns.m_col_drawbg ];
    const bool draw_b = row_b[ m_columns.m_col_drawbg ];

    if( draw_a && ! draw_b ) return COL_A_UP;
    else if( draw_b && ! draw_a ) return COL_B_UP;

    return 0;
}


//
// 列の値によるソート
//
// row_a が上か　row_b　が上かを返す。同じなら 0
//
int BoardViewBase::compare_col( const int col, const int sortmode,
                                const Gtk::TreeModel::Row& row_a, const Gtk::TreeModel::Row& row_b ) const
{
    int num_a = 0, num_b = 0;
    int ret = 0;
    DBTREE::ArticleBase *arta, *artb;

    const int UP = 1;
    const int DOWN = 2;

    switch( col ){

        case COL_MARK:
        {
            num_a = row_a[ m_columns.m_col_mark_val ];
            num_b = row_b[ m_columns.m_col_mark_val ];

            if( sortmode == SORTMODE_MARK2 ){ // 新着をキャッシュの上に

                if( num_a == COL_MARKVAL_NEWTHREAD
                    && ( num_b != COL_MARKVAL_NEWTHREAD && num_b <= COL_MARKVAL_UPDATED )
                    ){

                    num_a = DOWN; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = UP;
                }
                else if( num_b == COL_MARKVAL_NEWTHREAD
                         && ( num_a != COL_MARKVAL_NEWTHREAD && num_a <= COL_MARKVAL_UPDATED )
                    ){

                    num_a = UP; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = DOWN;
                }
            }
            else if( sortmode == SORTMODE_MARK3 ){ // 新着を一番上に

                if( num_a == COL_MARKVAL_NEWTHREAD && num_b != COL_MARKVAL_NEWTHREAD ){
                    num_a = DOWN; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = UP;
                }
                else if( num_b == COL_MARKVAL_NEWTHREAD && num_a != COL_MARKVAL_NEWTHREAD  ){
                    num_a = UP; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = DOWN;
                }
                else if( num_a == COL_MARKVAL_NEWTHREAD_HOUR && num_b != COL_MARKVAL_NEWTHREAD_HOUR ){
                    num_a = DOWN; // 下で ret *= -1 しているので UP と DOWNを逆にする
                    num_b = UP;
                }
                else if( num_b == COL_MARKVAL_NEWTHREAD_HOUR && num_a != COL_MARKVAL_NEWTHREAD_HOUR ){
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
            const Glib::ustring name_a = row_a[ m_columns.m_col_subject ];
            const Glib::ustring name_b = row_b[ m_columns.m_col_subject ];
            if( name_a < name_b ) { num_a = UP; num_b = DOWN; }
            else if( name_a > name_b ) { num_a = DOWN; num_b = UP; }
            break;
        }

        case COL_RES:
            num_a = row_a[ m_columns.m_col_res ];
            num_b = row_b[ m_columns.m_col_res ];
            break;

        case COL_STR_LOAD:
            arta = row_a[ m_columns.m_col_article ];
            artb = row_b[ m_columns.m_col_article ];
            num_a = arta->get_number_load();
            num_b = artb->get_number_load();
            break;

        case COL_STR_NEW:
            num_a = row_a[ m_columns.m_col_new ];
            num_b = row_b[ m_columns.m_col_new ];
            break;

        case COL_SINCE:
            arta = row_a[ m_columns.m_col_article ];
            artb = row_b[ m_columns.m_col_article ];
            num_a = arta->get_since_time();
            num_b = artb->get_since_time();
            break;

        case COL_WRITE:
            num_a = row_a[ m_columns.m_col_write_t ];
            num_b = row_b[ m_columns.m_col_write_t ];
            break;

        case COL_ACCESS:
            num_a = row_a[ m_columns.m_col_access_t ];
            num_b = row_b[ m_columns.m_col_access_t ];
            break;

        case COL_SPEED:
            num_a = row_a[ m_columns.m_col_speed ];
            num_b = row_b[ m_columns.m_col_speed ];
            break;

        case COL_DIFF:
            num_a = row_a[ m_columns.m_col_diff ];
            num_b = row_b[ m_columns.m_col_diff ];
            break;

        case COL_BOARD:
        {
            // 文字列の大小を数字に変換
            Glib::ustring name_a = row_a[ m_columns.m_col_board ];
            Glib::ustring name_b = row_b[ m_columns.m_col_board ];
            if( name_a < name_b ) { num_a = UP; num_b = DOWN; }
            else if( name_a > name_b ) { num_a = DOWN; num_b = UP; }
            break;
        }
    }

    // 両方とも 0 より大きいか 0 より小さい場合は普通に比較
    if( ( num_a > 0 && num_b > 0 ) || ( num_a < 0 && num_b < 0 ) ){

        if( num_a < num_b ) ret = COL_A_UP;
        else if( num_a > num_b ) ret = COL_B_UP;

        if( sortmode == SORTMODE_DESCEND ) ret *= -1;
        if( sortmode == SORTMODE_MARK1 || sortmode == SORTMODE_MARK2 || sortmode == SORTMODE_MARK3 ) ret *= -1;
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
int BoardViewBase::slot_compare_row( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b )
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
// コマンド
//
bool BoardViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    if( command == "update_columns" ) update_columns();

    else if( command == "draw_bg_articles" ) draw_bg_articles();
    else if( command == "clear_highlight" ) clear_highlight();

    else if( command == "select_item" ) select_item( arg1 );

    return true;
}


//
// クロック入力
//
void BoardViewBase::clock_in()
{
    View::clock_in();

    m_treeview.clock_in();

    if( m_cancel_openrow_counter ) --m_cancel_openrow_counter;
}


//
// ロード停止
//
void BoardViewBase::stop()
{
    DBTREE::board_stop_load( get_url_board() );
}



//
// ビュー表示
//
void BoardViewBase::show_view()
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::show_view " << get_url() << std::endl;
#endif

    if( is_loading() ) return;

    update_boardname();

    m_pre_query = std::string();
    m_last_access_time = DBTREE::board_last_access_time( get_url_board() );

    // オートリロードのカウンタを0にする
    reset_autoreload_counter();

    if( ! m_load_subject_txt ){

        update_view();
        return;
    }

    if( m_col_diff_is_shown ) DBTREE::board_read_subject_from_cache( get_url_board() );

    // download 開始
    // 終わったら update_view() が呼ばれる

    // DBに登録されてない時はロードしない
    if( get_url_board().empty() ){
        set_status( "invalid URL" );
        BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );
        return;
    }

    m_loading = true;
    DBTREE::board_download_subject( get_url_board(), get_url() );
    set_status( "loading..." );
    BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );

    // タブにアイコンを表示
    BOARD::get_admin()->set_command( "toggle_icon", get_url() );
}


//
// 色、フォント、表示内容の更新
//
void BoardViewBase::relayout( const bool completely )
{
    m_treeview.init_color( COLOR_CHAR_BOARD, COLOR_BACK_BOARD, COLOR_BACK_BOARD_EVEN );
    m_treeview.init_font( CONFIG::get_fontname( FONT_BOARD ) );
    update_item_all();
}



//
// view更新
//
// loading_fin : ロードが完了したら true をセットして呼び出す
//
void BoardViewBase::update_view_impl( const std::vector< DBTREE::ArticleBase* >& list_article, const bool loading_fin )
{
#ifdef _DEBUG
    const int code = DBTREE::board_code( get_url_board() );
    std::cout << "BoardViewBase::update_view_impl " << get_url()
              << " code = " << code
              << " size = " << list_article.size()
              << std::endl;
#endif

    // 画面消去
    m_treeview.unset_model();

    if( list_article.size() ){

        m_liststore->clear();

        unsorted_column();

        // 行の追加
        for( int i = list_article.size()-1; i >= 0;  --i ){

            DBTREE::ArticleBase* art = list_article[ i ];
            prepend_row( art, i + 1 );
        }

        m_treeview.set_model( m_liststore );

        if( loading_fin ){
            if( m_list_draw_bg_articles.size() ) draw_bg_articles();
            else restore_sort();
        }
    }

    if( loading_fin ){

        m_loading = false;

        update_status();

        // タブのアイコン状態を更新
        BOARD::get_admin()->set_command( "toggle_icon", get_url() );
    }
}


//
// ステータスバー更新
//
void BoardViewBase::update_status()
{
        std::ostringstream ss_tmp;
        if( m_load_subject_txt ) ss_tmp << DBTREE::board_str_code( get_url_board() ) << " ";
        ss_tmp << "[ 全 " << get_row_size() << " ] ";

        set_status( ss_tmp.str() );
        BOARD::get_admin()->set_command( "set_status", get_url(), get_status() );
}


//
// URLを選択
//
void BoardViewBase::select_item( const std::string& url )
{
    if( m_url_board != DBTREE::url_boardbase( url ) ) return;

    const Gtk::TreeModel::Row row = get_row_from_url( url );
    if( row ){
        Gtk::TreePath path = GET_PATH( row );

        m_treeview.get_selection()->unselect_all();
        m_treeview.set_cursor( path );
    }
}



void BoardViewBase::focus_view()
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::focus_view\n";
#endif

    m_treeview.grab_focus();
}


void BoardViewBase::focus_out()
{
    SKELETON::View::focus_out();
}



//
// 閉じる
//
void BoardViewBase::close_view()
{
    BOARD::get_admin()->set_command( "close_currentview" );
}


//
// 選択した行のログをまとめて削除
//
void BoardViewBase::slot_delete_logs()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    for( const Gtk::TreeModel::iterator& iter : list_it ) {

        Gtk::TreeModel::Row row = *iter;
        DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];
        CORE::core_set_command( "delete_article", art->get_url() );
    }
}




//
// viewの操作
//
bool BoardViewBase::operate_view( const int control )
{
    if( CONTROL::operate_common( control, get_url(), BOARD::get_admin() ) ) return true;

    bool open_tab = false;

    Gtk::TreePath path = m_treeview.get_current_path();

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

            // お気に入りに追加
        case CONTROL::AppendFavorite:
        {
            SKELETON::MsgDiag mdiag( get_parent_win(), "板と選択中のスレのどちらを登録しますか？", false,
                                     Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_CANCEL );

            mdiag.add_button( "板を登録(_B)", 0xB0A );
            mdiag.add_button( "スレを登録(_T)", 0xEAD );
            mdiag.set_default_response( 0xEAD );
            mdiag.set_title( "お気に入りに追加" );
            if( const int ret = mdiag.run(); ret == 0xB0A ) {
                slot_favorite_board();
            }
            else if( ret == 0xEAD ) {
                slot_favorite_thread();
            }
        }
        break;

            // スレを開く
        case CONTROL::OpenArticleTab:
            open_tab = true;
            [[fallthrough]];
        case CONTROL::OpenArticle:
            if( ! path.empty() ) open_row( path, open_tab, false );
            break;

            // Listに戻る
        case CONTROL::Left:
            CORE::core_set_command( "switch_leftview" );
            break;

            // 現在の記事を表示
        case CONTROL::Right:
            CORE::core_set_command( "switch_rightview" );
            break;

        case CONTROL::ScrollLeftBoard:
            scroll_left();
            break;

        case CONTROL::ScrollRightBoard:
            scroll_right();
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

            // 戻る、進む
        case CONTROL::PrevView:
            back_viewhistory( 1 );
            break;

        case CONTROL::NextView:
            forward_viewhistory( 1 );
            break;

            // datを保存
        case CONTROL::Save:
            slot_save_dat();
            break;

            // 閉じる
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
            write();
            break;

        case CONTROL::Delete:
        {

            if( CONFIG::get_show_deldiag() ){

                SKELETON::MsgCheckDiag mdiag( get_parent_win(),
                                              "選択した行のログを削除しますか？",
                                              "今後表示しない(常に削除)(_D)",
                                              Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
                mdiag.set_title( "削除確認" );
                const int ret = mdiag.run();
                if( ret != Gtk::RESPONSE_YES ) return true;
                if( mdiag.get_chkbutton().get_active() ) CONFIG::set_show_deldiag( false );
            }

            slot_delete_logs();
            break;
        }

        // ポップアップメニューをビューの左上に表示
        case CONTROL::ShowPopupMenu:
            show_popupmenu( "", SKELETON::PopupMenuPosition::view_top_left );
            break;

        // 検索
        case CONTROL::Search:
            m_search_invert = false;
            BOARD::get_admin()->set_command( "open_searchbar", get_url() );
            break;

        case CONTROL::SearchInvert:
            m_search_invert = true;
            BOARD::get_admin()->set_command( "open_searchbar", get_url() );
            break;

        case CONTROL::SearchNext:
            down_search();
            break;

        case CONTROL::SearchPrev:
            up_search();
            break;

        // 並び替えモードの切り替え
        case CONTROL::SortColumnMark:
            slot_col_clicked( COL_MARK );
            break;

        case CONTROL::SortColumnID:
            slot_col_clicked( COL_ID );
            break;

        case CONTROL::SortColumnBoard:
            slot_col_clicked( COL_BOARD );
            break;

        case CONTROL::SortColumnSubject:
            slot_col_clicked( COL_SUBJECT );
            break;

        case CONTROL::SortColumnRes:
            slot_col_clicked( COL_RES );
            break;

        case CONTROL::SortColumnStrLoad:
            slot_col_clicked( COL_STR_LOAD );
            break;

        case CONTROL::SortColumnStrNew:
            slot_col_clicked( COL_STR_NEW );
            break;

        case CONTROL::SortColumnSince:
            slot_col_clicked( COL_SINCE );
            break;

        case CONTROL::SortColumnWrite:
            slot_col_clicked( COL_WRITE );
            break;

        case CONTROL::SortColumnAccess:
            slot_col_clicked( COL_ACCESS );
            break;

        case CONTROL::SortColumnSpeed:
            slot_col_clicked( COL_SPEED );
            break;

        case CONTROL::SortColumnDiff:
            slot_col_clicked( COL_DIFF );
            break;

            // 板のプロパティ
        case CONTROL::PreferenceView:
            show_preference();
            break;

        default:
            return false;
    }

    return true;
}




//
// 先頭に戻る
//
void BoardViewBase::goto_top()
{
    m_treeview.goto_top();
}


//
// 一番最後へ
//
void BoardViewBase::goto_bottom()
{
    m_treeview.goto_bottom();
}



//
// 指定したIDのスレに移動
//
void BoardViewBase::goto_num( const int num, const int )
{
    if( ! num ) return;

    focus_view();

    Gtk::TreeModel::Children child = m_liststore->children();
    auto it = std::find_if( child.begin(), child.end(),
                            [this, num]( const Gtk::TreeRow& row ) { return row[ m_columns.m_col_id ] == num; } );
    if( it != child.end() ) {
        Gtk::TreePath path = GET_PATH( *it );
        m_treeview.scroll_to_row( path );
        m_treeview.set_cursor( path );
    }
}



//
// 左スクロール
//
void BoardViewBase::scroll_left()
{
    auto hadjust = m_scrwin.get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value( MAX( 0,  hadjust->get_value() - hadjust->get_step_increment() ) );
}


//
// 右スクロール
//
void BoardViewBase::scroll_right()
{
    auto hadjust = m_scrwin.get_hadjustment();
    if( !hadjust ) return;
    hadjust->set_value(  MIN( hadjust->get_upper() - hadjust->get_page_size(),
                              hadjust->get_value() + hadjust->get_step_increment() ) );
}


//
// 上へ移動
//
void BoardViewBase::row_up()
{
    m_treeview.row_up();
}


//
// 下へ移動
//
void BoardViewBase::row_down()
{
    m_treeview.row_down();
}


//
// page up
//
void BoardViewBase::page_up()
{
    m_treeview.page_up();
}


//
// page down
//
void BoardViewBase::page_down()
{
    m_treeview.page_down();
}


//
// ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
//
// SKELETON::View::show_popupmenu() を参照すること
//
void BoardViewBase::activate_act_before_popupmenu( const std::string& url )
{
    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    Glib::RefPtr<Gio::SimpleAction> act;

    // dat 保存
    act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "SaveDat" ) );
    if( act ){

        act->set_enabled( false );

        std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
        if( list_it.size() ){

            for( const Gtk::TreeModel::iterator& iter : list_it ) {

                Gtk::TreeModel::Row row = *iter;
                DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];
                if( art->is_cached() ) {
                    act->set_enabled( true );
                    break;
                }
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
Gtk::Menu* BoardViewBase::get_popupmenu( const std::string& url )
{
    // 削除サブメニュー
    if( url == "popup_menu_delete" ){
        return &m_popup_menu_delete;
    }

    // お気に入りサブメニュー
    else if( url == "popup_menu_favorite" ){

        const std::string url_board = path2url_board( m_path_selected );

        if( url_board.empty() ) return &m_popup_menu_favorite_article;
        else return &m_popup_menu_favorite;
    }

    // 通常メニュー
    else if( m_treeview.get_selection()->get_selected_rows().size() == 1 ){
        m_path_selected = * (m_treeview.get_selection()->get_selected_rows().begin() );
        return &m_popup_menu;
    }

    // 複数選択メニュー
    else{
        m_path_selected = Gtk::TreeModel::Path();
        return &m_popup_menu_mul;
    }
}


//
// 特定の行だけの表示内容更新
//
// url : boardbase アドレス
// id : DAT の ID(拡張子付き), empty なら全ての行の表示内容を更新する
//
void BoardViewBase::update_item( const std::string& url, const std::string& id )
{
    if( is_loading() ) return;
    if( ! get_row_size() ) return;
    if( get_url_board() != url ) return;

    if( id.empty() ){
        update_item_all();
        return;
    }

    const std::string url_dat = DBTREE::url_datbase( url ) + id;

#ifdef _DEBUG
    std::cout << "BoardViewBase::update_item " << get_url() << std::endl
              << "url = " << url << " id = " << id << " url_dat = " << url_dat << std::endl;
#endif

    unsorted_column();

    const Gtk::TreeModel::Row row = get_row_from_url( url_dat );
    if( row ) update_row_common( row );
}


//
// 全ての行の表示内容更新
//
void BoardViewBase::update_item_all()
{

#ifdef _DEBUG
    std::cout << "BoardViewBase::update_item_all " << get_url() << std::endl;
#endif

    unsorted_column();

    Gtk::TreeModel::Children child = m_liststore->children();
    for( Gtk::TreeModel::Row row : child ) {
        if( ! row ) continue;

        const DBTREE::ArticleBase* art = row[ m_columns.m_col_article ];
        if( ! art ) continue;

        update_row_common( row );
    }
}


//
// 行を作って内容をセット
//
Gtk::TreeModel::Row BoardViewBase::prepend_row( DBTREE::ArticleBase* art, const int id )
{
    Gtk::TreeModel::Row row = *( m_liststore->prepend() ); // append より prepend の方が速いらしい

    row[ m_columns.m_col_id ]  = id;

    row[ m_columns.m_col_diff ] = art->get_number_diff();

    if( m_col_board ) row[ m_columns.m_col_board ] = DBTREE::board_name( art->get_url() );

    row[ m_columns.m_col_article ] = art;

    row[ m_columns.m_col_drawbg ] = false;

    update_row_common( row );

    return row;
}


//
// prepend_row() と update_item() で共通に更新する列
//
void BoardViewBase::update_row_common( const Gtk::TreeModel::Row& row )
{
    DBTREE::ArticleBase* art = row[ m_columns.m_col_article ];
    if( art->empty() ) return;

    const int load = art->get_number_load();
    const int res = art->get_number();

    // タイトル、レス数、抽出
    row[ m_columns.m_col_subject ] = MISC::to_markup( art->get_modified_subject( true ) );
    row[ m_columns.m_col_res ] = res;

    // 読み込み数

    if( load ){
        row[ m_columns.m_col_str_load ] = Glib::ustring::compose( "%1", load );
        const int new_arrival = res - load;
        row[ m_columns.m_col_str_new ] = Glib::ustring::compose( "%1", new_arrival );
        row[ m_columns.m_col_new ] = new_arrival;
    }
    else{
        row[ m_columns.m_col_str_load ] = "";
        row[ m_columns.m_col_str_new ] = "";

        // キャッシュが無い場合はソートの優先度を
        // キャッシュがあって新着0の物より下げる
        row[ m_columns.m_col_new ] = -1;
    }

    // 速度
    if( ( art->get_status() & STATUS_NORMAL ) && ! art->is_924() ) {
        row[ m_columns.m_col_speed ] = art->get_speed();
    }

    //
    // マーク

    int mark_val;
    int icon;

    // ブックマーク
    if( art->is_bookmarked_thread() ){

        //  新着あり
        if( art->enable_load() ){
            mark_val = COL_MARKVAL_BKMARKED_UPDATED;
            icon = ICON::BKMARK_UPDATE;
        }

        // subject.txt が壊れている( subject.txt に示されたレス数よりも実際の取得数の方が多い )
        else if( art->is_cached() && ( art->get_status() & STATUS_BROKEN_SUBJECT ) ){
            mark_val = COL_MARKVAL_BKMARKED_BROKEN_SUBJECT;
            icon = ICON::BKMARK_BROKEN_SUBJECT;
        }

        else{
            mark_val = COL_MARKVAL_BKMARKED;
            icon = ICON::BKMARK;
        }
    }

    // dat落ち
    else if( art->get_status() & STATUS_OLD ){
        mark_val = COL_MARKVAL_OLD;
        icon = ICON::OLD;
    }

    // キャッシュが新着がない状態で存在し、さらに規定の最大レス数を越えている
    else if( art->is_finished() ){
        mark_val = COL_MARKVAL_FINISHED;

        // subject.txt が壊れている( subject.txt に示されたレス数よりも実際の取得数の方が多い )
        if( art->get_status() & STATUS_BROKEN_SUBJECT ) icon = ICON::BROKEN_SUBJECT;
        else icon = ICON::CHECK;
    }

    // 新着あり
    else if( art->enable_load() ){
        mark_val = COL_MARKVAL_UPDATED;
        icon = ICON::UPDATE;
    }

    // キャッシュあり
    else if( art->is_cached() ){

        // subject.txt が壊れている( subject.txt に示されたレス数よりも実際の取得数の方が多い )
        if( art->get_status() & STATUS_BROKEN_SUBJECT ){
            mark_val = COL_MARKVAL_BROKEN_SUBJECT;
            icon = ICON::BROKEN_SUBJECT;
        }

        // 新着無し
        else{
            mark_val = COL_MARKVAL_CACHED;
            icon = ICON::CHECK;
        }
    }

    // スレッド924
    else if( art->is_924() ){

        if( CONFIG::get_show_924() ){
            mark_val = COL_MARKVAL_924;
            icon = ICON::INFO;
        }
        else{
            mark_val = COL_MARKVAL_NORMAL;
            icon = ICON::TRANSPARENT;
        }
    }

    // キャッシュ無し、新着
    else if( ! get_url_board().empty() && art->get_since_time() > m_last_access_time ){
        mark_val = COL_MARKVAL_NEWTHREAD;
        icon = ICON::NEWTHREAD;
    }
    // キャッシュ無し、新着( CONFIG::get_newthread_hour() 時間以内 )
    else if( art->get_hour() < CONFIG::get_newthread_hour() ){
        mark_val = COL_MARKVAL_NEWTHREAD_HOUR;
        icon = ICON::NEWTHREAD_HOUR;
    }
    //キャッシュ無し
    else{
        mark_val = COL_MARKVAL_NORMAL;
        icon = ICON::TRANSPARENT;
    }
    row[ m_columns.m_col_mark_val ] = mark_val;
    row[ m_columns.m_col_mark ] = ICON::get_icon( icon );

    // スレ立て時間
    if( ! art->is_924() ) row[ m_columns.m_col_since ] = art->get_since_date();
    else row[ m_columns.m_col_since ] = std::string();

    // 書き込み時間
    if( art->get_write_time() ){
        row[ m_columns.m_col_write ] = art->get_write_date();
        row[ m_columns.m_col_write_t ] = art->get_write_time();
    }
    else{
        row[ m_columns.m_col_write ] = std::string();

        // DAT落ちしたスレや1000に到達したスレなどは書き込んでいてもソート時の優先度を下げる
        if( mark_val < COL_MARKVAL_NORMAL ) row[ m_columns.m_col_write_t ] = 0;
        else row[ m_columns.m_col_write_t ] = -1;
    }

    // ユーザが最後にロードした時間
    if( art->get_access_time() ){
        row[ m_columns.m_col_access ] = art->get_access_date();
        row[ m_columns.m_col_access_t ] = art->get_access_time();
    }
    else{
        row[ m_columns.m_col_access ] = std::string();

        // DAT落ちしたスレや1000に到達したスレなどは書き込んでいてもソート時の優先度を下げる
        if( mark_val < COL_MARKVAL_NORMAL ) row[ m_columns.m_col_access_t ] = 0;
        else row[ m_columns.m_col_access_t ] = -1;
    }
}


//
// マウスボタン押した
//
bool BoardViewBase::slot_button_press( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::slot_button_press\n";
#endif

    if( m_cancel_openrow_counter ){
#ifdef _DEBUG
        std::cout << "canceled\n";
#endif
        return true;
    }

    m_clicked = true;

    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    // ダブルクリック
    // button_release_eventでは event->type に必ず GDK_BUTTON_RELEASE が入る
    m_dblclicked = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclicked = true;

    BOARD::get_admin()->set_command( "switch_admin" );

    return true;
}



//
// マウスボタン離した
//
bool BoardViewBase::slot_button_release( GdkEventButton* event )
{
    if( ! m_clicked ) return true;
    m_clicked = false;

    /// マウスジェスチャ
    const int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない
    if( get_control().MG_wheel_end( event ) ) return true;

    if( mg != CONTROL::NoOperation && enable_mg() ){
        operate_view( mg );
        return true;
    }

    // リサイズするときにラベルをドラッグした場合
    if( event->window != m_treeview.get_bin_window()->gobj() ){

        save_column_width();
        return true;
    }

    const int x = static_cast<int>(event->x);
    const int y = static_cast<int>(event->y);
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int cell_x;
    int cell_y;

    // 座標からpath取得
    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) ){

        m_path_selected = path;

#ifdef _DEBUG
        std::cout << "BoardViewBase::slot_button_release : path = " << path.to_string()
                  << " title = " << column->get_title()
                  << " x = " << x << " y = " << y
                  << " cellheight = " << m_treeview.get_row_height()
                  << " cell_x = " << cell_x << " cell_y = " << cell_y << std::endl;
#endif

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclicked ) event->type = GDK_2BUTTON_PRESS;

        // スレを開く
        bool openarticle = get_control().button_alloted( event, CONTROL::OpenArticleButton );
        bool openarticletab = get_control().button_alloted( event, CONTROL::OpenArticleTabButton );

        if( openarticle || openarticletab ){

            // 複数行選択中
            if( m_treeview.get_selected_iterators().size() >= 2 ) open_selected_rows( false );

            else open_row( path, openarticletab, false );
        }

        // ポップアップメニューボタン
        else if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){

            show_popupmenu( "", SKELETON::PopupMenuPosition::mouse_pointer );
        }

        else operate_view( get_control().button_press( event ) );

        event->type = type_copy;
    }

    return true;
}



//
// マウス動かした
//
bool BoardViewBase::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

    return true;
}




//
// キー入力
//
bool BoardViewBase::slot_key_press( GdkEventKey* event )
{
    m_pressed_key = get_control().key_press( event );

    if( m_pressed_key != CONTROL::NoOperation ){

        // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
        // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
        // OpenArticle(Tab)は slot_key_release() で処理する
        if( m_pressed_key == CONTROL::OpenArticle ) return true;
        if( m_pressed_key == CONTROL::OpenArticleTab ) return true;

        if( operate_view( m_pressed_key ) ) return true;
    }
    else if( release_keyjump_key( event->keyval ) ) return true;

    return false;
}


//
// キーリリースイベント
//
bool BoardViewBase::slot_key_release( GdkEventKey* event )
{
    const int key = get_control().key_press( event );

    // 押したキーと違うときは処理しない
    if( key == m_pressed_key ){

        // キー入力でスレを開くとkey_releaseイベントがboadviewが画面から
        // 消えてから送られてWIDGET_REALIZED_FOR_EVENT assertionが出るので
        // OpenArticle(Tab)はここで処理する
        if( key == CONTROL::OpenArticle ) operate_view( key );
        if( key == CONTROL::OpenArticleTab ) operate_view( key );
    }

    return true;
}



//
// マウスホイールイベント
//
bool BoardViewBase::slot_scroll_event( GdkEventScroll* event )
{
    // ホイールマウスジェスチャ
    const int control = get_control().MG_wheel_scroll( event );
    if( enable_mg() && control != CONTROL::NoOperation ){
        operate_view( control );
        return true;
    }

    m_treeview.wheelscroll( event );
    return true;
}


//
// ツールチップのセット
//
bool BoardViewBase::slot_query_tooltip( int x, int y, bool keyboard_tooltip,
                                        const Glib::RefPtr<Gtk::Tooltip>& tooltip )
{
    if( keyboard_tooltip ) return false;

    int bin_x, bin_y;
    m_treeview.convert_widget_to_bin_window_coords( x, y, bin_x, bin_y );

    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* column;
    int cell_x, cell_y;
    if( m_treeview.get_path_at_pos( bin_x, bin_y, path, column, cell_x, cell_y ) ) {
        m_treeview.set_tooltip_cell( tooltip, &path, column, nullptr );

        const Glib::ustring title = column->get_title();
        Glib::ustring cell_text;
        if( title == ITEM_NAME_BOARD ) cell_text = get_name_of_cell( path, m_columns.m_col_board );
        else if( title == ITEM_NAME_NAME ) cell_text = get_name_of_cell( path, m_columns.m_col_subject );
        else if( title == ITEM_NAME_SINCE ) cell_text = get_name_of_cell( path, m_columns.m_col_since );
        else if( title == ITEM_NAME_LASTWRITE ) cell_text = get_name_of_cell( path, m_columns.m_col_write );
        else if( title == ITEM_NAME_ACCESS ) cell_text = get_name_of_cell( path, m_columns.m_col_access );

        // セルの内容が空ならツールチップを表示しない
        if( cell_text.empty() ) return false;

        const auto layout = m_treeview.create_pango_layout( MISC::to_plain( cell_text ) );
        int pixel_width, ph;
        layout->get_pixel_size( pixel_width, ph );
        constexpr int offset{ 8 };
        // セルの内容の幅が列幅より小さいならツールチップを表示しない
        if( pixel_width + offset < column->get_width() ) return false;

        // ツールチップにセルの内容をセットする
        if( title == ITEM_NAME_NAME ) {
            tooltip->set_markup( cell_text );
        }
        else {
            tooltip->set_text( cell_text );
        }
        return true;
    }
    return false;
}


//
// D&Dで受信側がデータ送信を要求してきた
//
void BoardViewBase::slot_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                        Gtk::SelectionData& selection_data, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::slot_drag_data_get\n";
#endif

    set_article_to_buffer();
    selection_data.set( m_treeview.get_dndtarget(), get_url_board() );
}


//
// text/url-list がドロップされた
//
void BoardViewBase::slot_dropped_url_list( const std::list< std::string >& url_list )
{
#ifdef _DEBUG
    std::cout << "BoardViewBase::slot_dropped_url_list\n";
#endif

    if( ! url_list.size() ) return;

    // 共有バッファにアドレスをセットしてから import_dat コマンドを発行
    CORE::DATA_INFO_LIST list_info;
    for( const std::string& url : url_list ) {

        if( url.empty() ) continue;
        if( url.find( "file://" ) == std::string::npos ) continue;

        CORE::DATA_INFO info;
        info.type = TYPE_FILE;
        info.url = MISC::remove_str( url, "file://" );
        list_info.push_back( info );

#ifdef _DEBUG
        std::cout << "append " << info.url << std::endl;
#endif
    }

    CORE::SBUF_set_list( list_info );
    CORE::core_set_command( "import_dat", get_url_board(), "no_show_diag", "use_sbuf" );
}


//
// ブックマーク設定、解除
//
void BoardViewBase::slot_bookmark( const int bookmark )
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    for( const Gtk::TreeModel::iterator& iter : list_it ) {
        Gtk::TreeModel::Row row = *iter;
        DBTREE::ArticleBase* art = row[ m_columns.m_col_article ];
        if( art ){
            bool set = bookmark;
            if( bookmark == BOOKMARK_AUTO ) set = ! art->is_bookmarked_thread();
#ifdef _DEBUG
            std::cout << "BoardViewBase::slot_bookmark url = " << art->get_url() << " set = " << set << std::endl;
#endif
            art->set_bookmarked_thread( set );
        }
    }
}


//
// popupmenu でタブで開くを選択
//
void BoardViewBase::slot_open_tab()
{
    if( ! m_path_selected.empty() ) open_row( m_path_selected, true, false );
}


//
// popupmenu でスレ情報を消さずに再取得を選択
//
void BoardViewBase::slot_reget_article()
{
    if( ! m_path_selected.empty() ) open_row( m_path_selected, true, true );
}


//
// スレをお気に入りに登録
//
// ポップアップメニューのslot
//
void BoardViewBase::slot_favorite_thread()
{
    // 共有バッファにデータをセットしてから append_favorite コマンド実行
    set_article_to_buffer();
    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}




//
// 板をお気に入りに追加
//
void BoardViewBase::slot_favorite_board()
{
    // 共有バッファにデータをセットしてから append_favorite コマンド実行
    set_board_to_buffer();
    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}


//
// 新スレをたてる
//
void BoardViewBase::write()
{
    CORE::core_set_command( "create_new_thread", get_url_board() );
}


//
// ツールバーの削除ボタン
//
void BoardViewBase::delete_view()
{
    // IDに紐づけたツールバーボタンを取得してポップアップメニューの表示位置に指定します。
    show_popupmenu( "popup_menu_delete", SKELETON::PopupMenuPosition::toolbar_button,
                    get_admin()->get_anchor_widget( kToolbarWidgetDelete ) );
}


//
// ツールバーのお気に入りボタン
//
void BoardViewBase::set_favorite()
{
    show_popupmenu( "popup_menu_favorite", SKELETON::PopupMenuPosition::toolbar_button,
                    get_admin()->get_anchor_widget( kToolbarWidgetFavoriteAdd ) );
}


//
// スレのURLをコピー
//
void BoardViewBase::slot_copy_url()
{
    if( m_path_selected.empty() ) return;

    const std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    MISC::CopyClipboard( url );
}


// スレの名前とURLをコピー
//
void BoardViewBase::slot_copy_title_url()
{
    if( m_path_selected.empty() ) return;

    const std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    const std::string name = MISC::to_plain( DBTREE::article_subject( url ) );

    MISC::CopyClipboard( name + '\n' + url );
}


//
// 全選択
//
void BoardViewBase::slot_select_all()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "全ての行を選択しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    mdiag.set_default_response( Gtk::RESPONSE_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    m_treeview.get_selection()->select_all();
}


//
// ポップアップメニューでブラウザで開くを選択
//
void BoardViewBase::slot_open_browser()
{
    const std::string url = DBTREE::url_readcgi( path2daturl( m_path_selected ), 0, 0 );
    CORE::core_set_command( "open_url_browser", url );
}



//
// 記事を開く
//
bool BoardViewBase::open_row( const Gtk::TreePath& path, const bool tab, const bool reget )
{
    std::string str_tab = "false";
    if( tab ) str_tab = "opentab";

    std::string mode;

    const std::string url_target = path2daturl( path );

#ifdef _DEBUG
    std::cout << "BoardViewBase::open_row " << url_target << std::endl;
#endif

    if( url_target.empty() ) return false;

    if( reget ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
            mdiag.run();
            return false;
        }

        if( ! DBTREE::article_is_cached( url_target ) ) return false;

        if( DBTREE::article_status( url_target ) & STATUS_OLD ){
            SKELETON::MsgDiag mdiag( get_parent_win(),
                                     "DAT落ちしています。\n\nログが消える恐れがあります。実行しますか？",
                                     false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            mdiag.set_default_response( Gtk::RESPONSE_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return false;
        }

        mode = "reget";
    }

    // datロード終了時に次スレ移行チェックを行う
    DBTREE::article_set_url_pre_article( url_target, get_url_pre_article() );

    CORE::core_set_command( "open_article", url_target, str_tab, mode );

    // 行を長押ししてから素早くクリックし直すとslot_button_press()が呼び出されて
    // スレビューが表示されてから一瞬スレ一覧に切り替わるのを防ぐ
    m_cancel_openrow_counter = CANCEL_OPENROW / TIMER_TIMEOUT;

    return true;
}



//
// 選択した行をまとめて開く
//
void BoardViewBase::open_selected_rows( const bool reget )
{
    std::string mode;
    std::string list_url;
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();

    if( reget ){

        if( ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
            mdiag.run();
            return;
        }

        for( const Gtk::TreeModel::iterator& iter : list_it ) {

            Gtk::TreeModel::Row row = *iter;
            DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];

            if( ! art->is_cached() ) continue;

            if( art->get_status() & STATUS_OLD ){
                SKELETON::MsgDiag mdiag( get_parent_win(),
                                         "DAT落ちしているスレを含んでいます。\n\nログが消える恐れがあります。実行しますか？",
                                         false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
                mdiag.set_default_response( Gtk::RESPONSE_NO );
                if( mdiag.run() != Gtk::RESPONSE_YES ) return;

                break;
            }
        }

        mode = "reget";
    }

    for( const Gtk::TreeModel::iterator& iter : list_it ) {

        if( !list_url.empty() ) list_url += " ";

        Gtk::TreeModel::Row row = *iter;
        DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];

        if( reget && ! art->is_cached() ) continue;

        list_url += art->get_url();

        // datロード終了時に次スレ移行チェックを行う
        art->set_url_pre_article( get_url_pre_article() );
    }

    if( list_url.size() ) CORE::core_set_command( "open_article_list", std::string(), list_url, mode );
}


//
// path -> スレッドの(dat型)URL変換
//
std::string BoardViewBase::path2daturl( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return std::string();

    DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];
    return art->get_url();
}


//
// path -> 板URL変換
//
std::string BoardViewBase::path2url_board( const Gtk::TreePath& path )
{
    if( ! get_url_board().empty() ) return get_url_board();
    if( path.empty() ) return std::string();
    return DBTREE::url_boardbase( path2daturl( path ) );
}


//
// 抽出
//
bool BoardViewBase::drawout( const bool force_reset )
{
    int hit = 0;
    bool reset = false;

    const std::string query = get_search_query();

    // 空の時はリセット
    if( force_reset || query.empty() ) reset = true;

#ifdef _DEBUG
    std::cout << "BoardViewBase::drawout query = " <<  query << std::endl;
#endif

    unsorted_column();

    JDLIB::Regex regex;
    JDLIB::RegexPattern regexptn;
    constexpr bool icase = true; // 大文字小文字区別しない
    constexpr bool newline = true; // . に改行をマッチさせない
    constexpr bool usemigemo = true; // migemo使用
    constexpr bool wchar = true;  // 全角半角の区別をしない

    Gtk::TreeModel::Children child = m_liststore->children();

    if ( ! reset ) regexptn.set( query, icase, newline, usemigemo, wchar );

    for( Gtk::TreeModel::Row row : child ) {

        const Glib::ustring subject = row[ m_columns.m_col_subject ];

        if( reset ) row[ m_columns.m_col_drawbg ] = false;
        else if( regex.match( regexptn, MISC::to_plain( subject ), 0 ) ){
            row[ m_columns.m_col_drawbg ] = true;
            ++hit;

#ifdef _DEBUG
            std::cout << subject << " " << row[ m_columns.m_col_mark_val ] << std::endl;
#endif

        }
        else row[ m_columns.m_col_drawbg ] = false;
    }

    restore_sort();

    if( reset ) CORE::core_set_command( "set_info", "", "" );
    else if( ! hit ) CORE::core_set_command( "set_info", "", "検索結果： ヒット無し" );
    else CORE::core_set_command( "set_info", "", "検索結果： " + std::to_string( hit ) + "件" );

    return true;
}



//
// 検索ボックスの文字列が変わった
//
void BoardViewBase::set_search_query( const std::string& query )
{
    SKELETON::View::set_search_query( query );

#ifdef _DEBUG
    std::cout << "BoardViewBase::set_search_query query = " << get_search_query() << std::endl;
#endif

    if( CONFIG::get_inc_search_board() ){
        drawout( false );
        m_pre_query = std::string();
    }
}


//
// 検索実行
//
void BoardViewBase::exec_search()
{
    const std::string query = get_search_query();
    if( m_pre_query != query ){
        drawout( false );
        focus_view();
        m_pre_query = query;
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_BOARD, query );
        return;
    }

    focus_view();
    if( query.empty() ) return;

    Gtk::TreePath path = m_treeview.get_current_path();
    if( path.empty() ){
        if( m_search_invert ) path = GET_PATH( *( m_liststore->children().begin() ) );
        else {
            GET_PATH( *( std::prev( m_liststore->children().end() ) ) );
        }
    }

    Gtk::TreePath path_start = path;
    JDLIB::Regex regex;
    constexpr size_t offset = 0;
    constexpr bool icase = true; // 大文字小文字区別しない
    constexpr bool newline = true; // . に改行をマッチさせない
    constexpr bool usemigemo = true; // migemo使用
    constexpr bool wchar = true;  // 全角半角の区別をしない

#ifdef _DEBUG
    std::cout << "BoardViewBase::search start = " << path_start.to_string() << " query = " <<  query << std::endl;
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
                path = GET_PATH( *( std::prev( m_liststore->children().end() ) ) );
            }
        }


        if( path == path_start ) break;

        Glib::ustring subject = get_name_of_cell( path, m_columns.m_col_subject );
        if( regex.exec( query, MISC::to_plain( subject ), offset, icase, newline, usemigemo, wchar ) ){
            m_treeview.scroll_to_row( path, 0 );
            m_treeview.set_cursor( path );
            return;
        }
    }
}




// 前検索
void BoardViewBase::up_search()
{
    m_search_invert = true;
    exec_search();
}



// 後検索
void BoardViewBase::down_search()
{
    m_search_invert = false;
    exec_search();
}


//
// 検索entryの操作
//
void BoardViewBase::operate_search( const std::string& controlid )
{
    const int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ) focus_view();
    else if( id == CONTROL::SearchCache ) CORE::core_set_command( "open_article_searchlog", get_url_board() , get_search_query(), "exec" );
}


//
// ハイライト解除
//
void BoardViewBase::clear_highlight()
{
    drawout( true );
    focus_view();
    m_pre_query = std::string();
}


//
// 板プロパティ表示
//
void BoardViewBase::show_preference()
{
    const std::string url_board = path2url_board( m_path_selected );
    if( url_board.empty() ) return;

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_BOARD, url_board );
    pref->run();
}


//
// スレプロパティ表示
//
void BoardViewBase::slot_preferences_article()
{
    if( m_path_selected.empty() ) return;
    const std::string url = path2daturl( m_path_selected );

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_ARTICLE, url );
    pref->run();
}


//
// 戻る
//
void BoardViewBase::back_viewhistory( const int count )
{
    BOARD::get_admin()->set_command( "back_viewhistory", get_url(), std::to_string( count ) );
}


//
// 進む
//
void BoardViewBase::forward_viewhistory( const int count )
{
    BOARD::get_admin()->set_command( "forward_viewhistory", get_url(), std::to_string( count ) );
}



//
// datを保存
//
void BoardViewBase::slot_save_dat()
{
    if( ! m_enable_menuslot ) return;

    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();

#ifdef _DEBUG
    std::cout << "BoardViewBase::slot_save_dat size = " << list_it.size() << std::endl;
#endif

    if( ! list_it.size() ) return;

    // ひとつだけ名前を付けて保存
    if( list_it.size() == 1 ){
        Gtk::TreePath path = m_treeview.get_current_path();

        if( path.empty() ) return;
        const std::string url = path2daturl( path );

        DBTREE::article_save_dat( url, std::string() );
        return;
    }

    // 複数のdatを保存

    int overwrite = Gtk::RESPONSE_NO;

    // ディレクトリ選択
    SKELETON::FileDiag diag( get_parent_win(), "保存先選択", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );
    diag.set_current_folder( SESSION::get_dir_dat() );
    if( diag.run() != Gtk::RESPONSE_ACCEPT ) return;

    diag.hide();

    std::string path_dir = MISC::recover_path( diag.get_filename() );
    if( path_dir.empty() ) return;
    if( path_dir.c_str()[ path_dir.length()-1 ] != '/' ) path_dir += "/";

#ifdef _DEBUG
    std::cout << "dir = " << path_dir << std::endl;
#endif

    if( ! CACHE::jdmkdir( path_dir ) ){

        SKELETON::MsgDiag mdiag( get_parent_win(), path_dir + "\n\nの作成に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。\n\ndatファイルの保存をキャンセルしました。原因を解決してからもう一度保存を行ってください。" );
        mdiag.run();
        return;
    }

    SESSION::set_dir_dat( path_dir );

    for( const Gtk::TreeModel::iterator& iter : list_it ) {

        Gtk::TreeModel::Row row = *iter;
        DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];

        if( ! art->is_cached() ) continue;

        const std::string path_from = CACHE::path_dat( art->get_url() );
        const std::string path_to = path_dir + MISC::get_filename( art->get_url() );

#ifdef _DEBUG
        std::cout << "from = " << path_from  << std::endl;
        std::cout << "to   = " << path_to  << std::endl;
#endif

        bool copy_file = true;

        // 既にファイルがある場合
        if( CACHE::file_exists( path_to ) == CACHE::EXIST_FILE ){

            // すべて上書き
            if( overwrite == SKELETON::OVERWRITE_YES_ALL ) copy_file = true;

            // すべていいえ
            else if( overwrite == SKELETON::OVERWRITE_NO_ALL ) copy_file = false;

            else{

                for(;;){

                    SKELETON::MsgOverwriteDiag mdiag( get_parent_win() );

                    overwrite = mdiag.run();
                    mdiag.hide();

                    switch( overwrite ){

                        // すべて上書き
                        case SKELETON::OVERWRITE_YES_ALL:

                            // 上書き
                        case SKELETON::OVERWRITE_YES:

                            copy_file = true;
                            break;

                            // 名前変更
                        case Gtk::RESPONSE_YES:
                            if( ! art->save_dat( path_to ) ) continue;
                            [[fallthrough]];

                        default:
                            copy_file = false;
                            break;
                    }

                    break;  // for(;;)
                }
            }
        }

        if( copy_file ){

#ifdef _DEBUG
            std::cout << "copy\n";
#endif
            if( ! CACHE::jdcopy( path_from, path_to ) ){

                SKELETON::MsgDiag mdiag( get_parent_win(), path_to + "\n\nの保存に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。\n\ndatファイルの保存をキャンセルしました。原因を解決してからもう一度保存を行ってください。" );
                mdiag.run();

                return;
            }
        }

    }

}


//
// 次スレ検索
//
void BoardViewBase::slot_search_next()
{
    if( m_path_selected.empty() ) return;
    const std::string url = path2daturl( m_path_selected );

    CORE::core_set_command( "open_board_next", DBTREE::url_boardbase( url ), url );
}


/** @brief 選択したスレをあぼーん
 *
 * @details NG スレタイトルは未変換のスレタイトルで比較します。
 */
void BoardViewBase::slot_abone_thread()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    if( ! list_it.size() ) return;

    std::list< std::string > threads = DBTREE::get_abone_list_thread( get_url_board() );

    for( const Gtk::TreeModel::iterator& iter : list_it ) {
        Gtk::TreeModel::Row row = *iter;
        if( const DBTREE::ArticleBase* art = row[ m_columns.m_col_article ]; art ) {
            threads.push_back( art->get_subject() );
        }
    }

    // あぼーん情報更新
    std::list< std::string > words = DBTREE::get_abone_list_word_thread( get_url_board() );
    std::list< std::string > regexs = DBTREE::get_abone_list_regex_thread( get_url_board() );
    const int low_number = DBTREE::get_abone_low_number_thread( get_url_board() );
    const int high_number = DBTREE::get_abone_high_number_thread( get_url_board() );
    const int hour = DBTREE::get_abone_hour_thread( get_url_board() );
    const bool redraw = false; // 板の再描画はしない
    DBTREE::reset_abone_thread( get_url_board(), threads, words, regexs, low_number, high_number, hour, redraw );

    m_treeview.delete_selected_rows( true );
}



//
// path と column からそのセルの内容を取得
//
template < typename ColumnType >
std::string BoardViewBase::get_name_of_cell( Gtk::TreePath& path, const Gtk::TreeModelColumn< ColumnType >& column )
{
    Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( !row ) return std::string();

    const Glib::ustring name = row[ column ];
    return name;
}



//
// 共有バッファに選択中の行を登録する
//
void BoardViewBase::set_article_to_buffer()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    if( list_it.size() ){

        CORE::DATA_INFO_LIST list_info;
        Gtk::TreePath path( "0" );
        for( const Gtk::TreeModel::iterator& iter : list_it ) {

            Gtk::TreeModel::Row row = *iter;
            DBTREE::ArticleBase *art = row[ m_columns.m_col_article ];
            const Glib::ustring name = row[ m_columns.m_col_subject ];

            CORE::DATA_INFO info;
            info.type = TYPE_THREAD;
            info.parent = BOARD::get_admin()->get_win();
            info.url = art->get_url();
            info.name = MISC::to_plain( name.raw() );
            info.path = path.to_string();

            list_info.push_back( info );

#ifdef _DEBUG
            std::cout << "append " << info.name << std::endl;
#endif
            path.next();
        }

        CORE::SBUF_set_list( list_info );
    }
}



//
// 共有バッファに板を登録する
//
void BoardViewBase::set_board_to_buffer()
{
    CORE::DATA_INFO_LIST list_info;
    CORE::DATA_INFO info;

    const std::string url_board = path2url_board( m_path_selected );
    if( url_board.empty() ) return;

    info.type = TYPE_BOARD;
    info.parent = BOARD::get_admin()->get_win();
    info.url = DBTREE::url_boardbase( url_board );
    info.name = DBTREE::board_name( url_board );
    info.path = Gtk::TreePath( "0" ).to_string();

    list_info.push_back( info );

    CORE::SBUF_set_list( list_info );
}


//
// 指定したスレを強調して表示
// dat 落ち等で表示されていないスレも強制的に表示する
// 共有バッファに表示したいスレをセットしてから set_command 経由で呼び出す
//
void BoardViewBase::draw_bg_articles()
{
    // 共有バッファから追加するスレのURLのリストを作成
    if( ! m_list_draw_bg_articles.size() ){

        if( CORE::SBUF_size() == 0 ) return;

        const CORE::DATA_INFO_LIST list_info = CORE::SBUF_list_info();

        for( const CORE::DATA_INFO& info : list_info ) {

            if( info.type != TYPE_THREAD ) continue;

            m_list_draw_bg_articles.push_back( info.url );
        }
    }

    if( ! m_list_draw_bg_articles.size() ) return;

    // ロード中の時はロード後にもう一度呼び出す
    if( is_loading() ) return;

#ifdef _DEBUG
    std::cout << "BoardViewBase::draw_bg_articles size = " << m_list_draw_bg_articles.size() << std::endl;
#endif

    unsorted_column();

    for( const std::string& url : m_list_draw_bg_articles ) {

#ifdef _DEBUG
        std::cout << url << std::endl;
#endif

        Gtk::TreeModel::Row row = get_row_from_url( url );

        // row が無ければ作成
        if( ! row ){
            DBTREE::ArticleBase* art = DBTREE::get_article( url );
            row = prepend_row( art, get_row_size() + 1 );
        }

        // 強調表示
        row[ m_columns.m_col_drawbg ] = true;
    }

    restore_sort();

    m_list_draw_bg_articles.clear();
}

