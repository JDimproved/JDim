// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"
#include "gtkmmversion.h"

#include "bbslistviewbase.h"
#include "bbslistadmin.h"
#include "selectdialog.h"
#include "editlistwin.h"
#include "addetcdialog.h"

#include "skeleton/msgdiag.h"

#include "jdlib/jdregex.h"
#include "jdlib/miscgtk.h"
#include "jdlib/misctime.h"
#include "jdlib/miscutil.h"

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
#include "sign.h"

#include <glib/gi18n.h>


enum
{
    REPLACE_NEXT_NO = 0,
    REPLACE_NEXT_YES,
    REPLACE_NEXT_ADD
};


// row -> path
#define GET_PATH( row ) m_treestore->get_path( row )


/// @brief ポップアップメニューを指定位置に表示
#define SHOW_POPUPMENU(position) do{\
std::string url = path2url( m_path_selected ); \
if( ! m_path_selected.empty() && url.empty() ) url = "dummy_url"; \
show_popupmenu( url, position ); \
}while(0)


// GtkMenu の項目
// GtkBuilderのUI definitionはボイラープレートコードが多いためマクロで整理する

/// メニュー項目のUI definitionを作る
#define MENU_ITEM_ELEM(label, action) \
    R"(<item><attribute name="label" translatable="yes">)" label "</attribute>" \
    R"(<attribute name="action">)" action "</attribute></item>"

/// サブメニューのUI definitionを作る
#define SUBMENU_ELEM(label, ...) \
    R"(<submenu><attribute name="label" translatable="yes">)" label "</attribute>" __VA_ARGS__ "</submenu>"


#define POPUPMENU_BOARD1 \
    "<section>" \
    MENU_ITEM_ELEM("タブで開く(_T)", "bbslist.OpenTab") \
    MENU_ITEM_ELEM(ITEM_NAME_OPEN_BROWSER "(_W)", "bbslist.OpenBrowser") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM(ITEM_NAME_COPY_URL "(_U)", "bbslist.CopyURL") \
    MENU_ITEM_ELEM(ITEM_NAME_COPY_TITLE_URL "(_L)", "bbslist.CopyTitleURL") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite") \
    "</section>"

#define POPUPMENU_BOARD2 \
    "<section>" \
    MENU_ITEM_ELEM("キャッシュ内ログ検索(_S)", "bbslist.SearchCacheBoard") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM("datのインポート(_I)", "bbslist.ImportDat") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM(ITEM_NAME_PREF_BOARD "(_O)...", "bbslist.PreferenceBoard") \
    "</section>" \
    "</menu>"

#define POPUPMENU_ARRANGE_BASE \
    MENU_ITEM_ELEM("種類順(_T)", "bbslist.ArrangeType") \
    MENU_ITEM_ELEM("名前順(_N)", "bbslist.ArrangeName")

#define POPUPMENU_ARRANGE \
    "<section>" \
    SUBMENU_ELEM("並び替え(_G)", POPUPMENU_ARRANGE_BASE) \
    "</section>"

#define POPUPMENU_ARRANGEDIR \
    "<section>" \
    SUBMENU_ELEM("ディレクトリ内の並び替え(_G)", POPUPMENU_ARRANGE_BASE) \
    "</section>"

#define POPUPMENU_DELETE \
    "<section>" \
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("お気に入りから削除する(_D)", "bbslist.Delete")) \
    "</section>"

#define POPUPMENU_SELECT \
    "<section>" \
    MENU_ITEM_ELEM(ITEM_NAME_COPY_URL "(_U)", "bbslist.CopyURL") \
    MENU_ITEM_ELEM(ITEM_NAME_COPY_TITLE_URL "(_L)", "bbslist.CopyTitleURL") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM("名前変更(_R)", "bbslist.Rename") \
    MENU_ITEM_ELEM("新規ディレクトリ(_N)", "bbslist.NewDir") \
    MENU_ITEM_ELEM("コメント挿入(_I)", "bbslist.NewCom") \
    "</section>" \
    POPUPMENU_ARRANGE \
    POPUPMENU_DELETE \
    "<section>" \
    MENU_ITEM_ELEM("キャッシュ内ログ検索(_S)", "bbslist.SearchCacheBoard") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM("datのインポート(_I)", "bbslist.ImportDat") \
    "</section>" \
    "<section>" \
    MENU_ITEM_ELEM(ITEM_NAME_PREF_THREAD "(_P)...", "bbslist.PreferenceArticle") \
    MENU_ITEM_ELEM(ITEM_NAME_PREF_BOARD "(_O)...", "bbslist.PreferenceBoard") \
    MENU_ITEM_ELEM(ITEM_NAME_PREF_IMAGE "(_M)...", "bbslist.PreferenceImage") \
    "</section>"


// GtkBuilderのUI definitionはメニューごとに分けて使うものだけ構築する
namespace {

/// 板一覧 (板)
constexpr const char* popup_menu =
    "<interface>"
    R"(<menu id="popup_menu">)"
    POPUPMENU_BOARD1
    POPUPMENU_BOARD2
    "</interface>";

/// 板一覧 (外部BBSMENU)
constexpr const char* popup_menu_bbsmenu =
    "<interface>"
    R"(<menu id="popup_menu_bbsmenu">)"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_OPEN_BROWSER "(_W)", "bbslist.OpenBrowser")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_COPY_URL "(_U)", "bbslist.CopyURL")
    MENU_ITEM_ELEM(ITEM_NAME_COPY_TITLE_URL "(_L)", "bbslist.CopyTitleURL")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM("編集(_E)...", "bbslist.MoveBBSMenu")
    "</section>"
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("外部BBSMENUを削除する(_D)", "bbslist.DeleteBBSMenu"))
    "</section>"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_PREF_BBSMENU "(_B)...", "bbslist.PreferenceBBSMenu")
    "</section>"
    "</menu>"
    "</interface>";

/// 板一覧 (外部板)
constexpr const char* popup_menu_etc =
    "<interface>"
    R"(<menu id="popup_menu_etc">)"
    POPUPMENU_BOARD1
    MENU_ITEM_ELEM("編集(_E)...", "bbslist.MoveEtc")
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("外部板を削除する(_D)", "bbslist.DeleteEtc"))
    "</section>"
    POPUPMENU_BOARD2
    "</interface>";

/// 板一覧 (複数選択)
constexpr const char* popup_menu_mul =
    "<interface>"
    R"(<menu id="popup_menu_mul">)"
    MENU_ITEM_ELEM("選択した行を開く(_O)", "bbslist.OpenRows")
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    "</menu>"
    "</interface>";

/// 板一覧 (複数選択,外部BBSMENU含む)
constexpr const char* popup_menu_mul_bbsmenu =
    "<interface>"
    R"(<menu id="popup_menu_mul_bbsmenu">)"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("外部BBSMENUを削除する(_D)", "bbslist.DeleteBBSMenu"))
    "</section>"
    "</menu>"
    "</interface>";

/// 板一覧 (複数選択, 外部板含む)
constexpr const char* popup_menu_mul_etc =
    "<interface>"
    R"(<menu id="popup_menu_mul_etc">)"
    MENU_ITEM_ELEM("選択した行を開く(_O)", "bbslist.OpenRows")
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("外部板を削除する(_D)", "bbslist.DeleteEtc"))
    "</menu>"
    "</interface>";

/// 板一覧 (ディレクトリ)
constexpr const char* popup_menu_dir =
    "<interface>"
    R"(<menu id="popup_menu_dir">)"
    MENU_ITEM_ELEM("全て選択(_A)", "bbslist.SelectDir")
    "</menu>"
    "</interface>";

/// 板一覧 (外部BBSMENUディレクトリ)
constexpr const char* popup_menu_bbsmenudir =
    "<interface>"
    R"(<menu id="popup_menu_bbsmenudir">)"
    MENU_ITEM_ELEM("全て選択(_A)", "bbslist.SelectDir")
    "<section>"
    MENU_ITEM_ELEM("外部BBSMENU追加(_B)...", "bbslist.NewBBSMenu")
    "</section>"
    "</menu>"
    "</interface>";

/// 板一覧 (外部板ディレクトリ)
constexpr const char* popup_menu_etcdir =
    "<interface>"
    R"(<menu id="popup_menu_etcdir">)"
    MENU_ITEM_ELEM("全て選択(_A)", "bbslist.SelectDir")
    "<section>"
    MENU_ITEM_ELEM("外部板追加(_E)...", "bbslist.NewEtc")
    "</section>"
    "</menu>"
    "</interface>";

/////////////////////////////////////////

/// お気に入り
constexpr const char* popup_menu_favorite =
    "<interface>"
    R"(<menu id="popup_menu_favorite">)"
    "<section>"
    MENU_ITEM_ELEM("タブで開く(_T)", "bbslist.OpenTab")
    MENU_ITEM_ELEM(ITEM_NAME_OPEN_BROWSER "(_W)", "bbslist.OpenBrowser")
    MENU_ITEM_ELEM(ITEM_NAME_OPEN_CACHE_BROWSER "(_X)", "bbslist.OpenCacheBrowser")
    "</section>"
    POPUPMENU_SELECT
    "</menu>"
    "</interface>";

/// お気に入り (複数選択)
constexpr const char* popup_menu_favorite_mul =
    "<interface>"
    R"(<menu id="popup_menu_favorite_mul">)"
    MENU_ITEM_ELEM("選択した行を開く(_O)", "bbslist.OpenRows")
    "<section>"
    SUBMENU_ELEM("更新チェック(_M)",
    MENU_ITEM_ELEM("更新チェックのみ(_H)", "bbslist.CheckUpdateRows")
    MENU_ITEM_ELEM("更新された行をタブで開く(_E)", "bbslist.CheckUpdateOpenRows")
    "<section>"
    MENU_ITEM_ELEM("キャンセル(_C)", "bbslist.CancelCheckUpdate")
    "</section>"
    )
    "</section>"
    POPUPMENU_DELETE
    "</menu>"
    "</interface>";

/// お気に入り (何もないところをクリック)
constexpr const char* popup_menu_favorite_space =
    "<interface>"
    R"(<menu id="popup_menu_favorite_space">)"
    MENU_ITEM_ELEM("新規ディレクトリ(_N)", "bbslist.NewDir")
    MENU_ITEM_ELEM("コメント挿入(_I)", "bbslist.NewCom")
    "</menu>"
    "</interface>";

/// お気に入り (ディレクトリ)
constexpr const char* popup_menu_favorite_dir =
    "<interface>"
    R"(<menu id="popup_menu_favorite_dir">)"
    SUBMENU_ELEM("更新チェック(_M)",
    MENU_ITEM_ELEM("更新チェックのみ(_R)", "bbslist.CheckUpdateDir")
    MENU_ITEM_ELEM("更新された行をタブで開く(_A)", "bbslist.CheckUpdateOpenDir")
    "<section>"
    MENU_ITEM_ELEM("キャンセル(_C)", "bbslist.CancelCheckUpdate")
    "</section>"
    )
    "<section>"
    MENU_ITEM_ELEM("全て選択(_A)", "bbslist.SelectDir")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM("名前変更(_R)", "bbslist.Rename")
    MENU_ITEM_ELEM("新規ディレクトリ(_N)", "bbslist.NewDir")
    MENU_ITEM_ELEM("コメント挿入(_I)", "bbslist.NewCom")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM("ディレクトリをスレ一覧に表示(_B)", "bbslist.OpenAsBoard")
    MENU_ITEM_ELEM("仮想板作成(_V)", "bbslist.CreateVBoard")
    "</section>"
    POPUPMENU_ARRANGEDIR
    POPUPMENU_DELETE
    "</menu>"
    "</interface>";

/// お気に入り (コメント)
constexpr const char* popup_menu_favorite_com =
    "<interface>"
    R"(<menu id="popup_menu_favorite_com">)"
    "<section>"
    MENU_ITEM_ELEM("名前変更(_R)", "bbslist.Rename")
    MENU_ITEM_ELEM("新規ディレクトリ(_N)", "bbslist.NewDir")
    MENU_ITEM_ELEM("コメント挿入(_I)", "bbslist.NewCom")
    "</section>"
    POPUPMENU_ARRANGE
    POPUPMENU_DELETE
    "</menu>"
    "</interface>";

/// お気に入り (仮想板)
constexpr const char* popup_menu_favorite_vboard =
    "<interface>"
    R"(<menu id="popup_menu_favorite_vboard">)"
    MENU_ITEM_ELEM("タブで開く(_T)", "bbslist.OpenTab")
    "<section>"
    MENU_ITEM_ELEM("名前変更(_R)", "bbslist.Rename")
    MENU_ITEM_ELEM("新規ディレクトリ(_N)", "bbslist.NewDir")
    MENU_ITEM_ELEM("コメント挿入(_I)", "bbslist.NewCom")
    "</section>"
    POPUPMENU_ARRANGE
    POPUPMENU_DELETE
    "</menu>"
    "</interface>";

//////////////////////////////////////

/// 選択(selectlistview)
constexpr const char* popup_menu_select =
    "<interface>"
    R"(<menu id="popup_menu_select">)"
    POPUPMENU_SELECT
    "</menu>"
    "</interface>";

/////////////////////////////////////////

/// 履歴
constexpr const char* popup_menu_history =
    "<interface>"
    R"(<menu id="popup_menu_history">)"
    MENU_ITEM_ELEM("タブで開く(_T)", "bbslist.OpenTab")
    MENU_ITEM_ELEM(ITEM_NAME_OPEN_BROWSER "(_W)", "bbslist.OpenBrowser")
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_COPY_URL "(_U)", "bbslist.CopyURL")
    MENU_ITEM_ELEM(ITEM_NAME_COPY_TITLE_URL "(_L)", "bbslist.CopyTitleURL")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("履歴から削除する(_D)", "bbslist.DeleteHist"))
    "</section>"
    "<section>"
    MENU_ITEM_ELEM("キャッシュ内ログ検索(_S)", "bbslist.SearchCacheBoard")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM("datのインポート(_I)", "bbslist.ImportDat")
    "</section>"
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_PREF_THREAD "(_P)...", "bbslist.PreferenceArticle")
    MENU_ITEM_ELEM(ITEM_NAME_PREF_BOARD "(_O)...", "bbslist.PreferenceBoard")
    MENU_ITEM_ELEM(ITEM_NAME_PREF_IMAGE "(_M)...", "bbslist.PreferenceImage")
    "</section>"
    "</menu>"
    "</interface>";

/// 履歴 (複数選択)
constexpr const char* popup_menu_history_mul =
    "<interface>"
    R"(<menu id="popup_menu_history_mul">)"
    MENU_ITEM_ELEM("選択した行を開く(_O)", "bbslist.OpenRows")
    "<section>"
    MENU_ITEM_ELEM(ITEM_NAME_APPENDFAVORITE "(_F)...", "bbslist.AppendFavorite")
    "</section>"
    "<section>"
    SUBMENU_ELEM("更新チェック(_M)",
    MENU_ITEM_ELEM("更新チェックのみ(_H)", "bbslist.CheckUpdateRows")
    MENU_ITEM_ELEM("更新された行をタブで開く(_E)", "bbslist.CheckUpdateOpenRows")
    "<section>"
    MENU_ITEM_ELEM("キャンセル(_C)", "bbslist.CancelCheckUpdate")
    "</section>"
    )
    "</section>"
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("履歴から削除する(_D)", "bbslist.DeleteHist"))
    "</section>"
    "</menu>"
    "</interface>";

/// 履歴 (仮想板)
constexpr const char* popup_menu_history_vboard =
    "<interface>"
    R"(<menu id="popup_menu_history_vboard">)"
    MENU_ITEM_ELEM("タブで開く(_T)", "bbslist.OpenTab")
    "<section>"
    SUBMENU_ELEM(ITEM_NAME_DELETE "(_D)", MENU_ITEM_ELEM("履歴から削除する(_D)", "bbslist.DeleteHist"))
    "</section>"
    "</menu>"
    "</interface>";


/** @brief メニューIDからUI definitionのデータを取得する
 *
 * @param[in] id メニューID
 * @return GtkBuilder で実体化させるメニューの UI definition
 */
const char* get_menu_ui( const std::string& id )
{
    const char* menu_ui;
    if( id == "popup_menu" ) menu_ui = popup_menu;
    else if( id == "popup_menu_bbsmenu" ) menu_ui = popup_menu_bbsmenu;
    else if( id == "popup_menu_etc" ) menu_ui = popup_menu_etc;
    else if( id == "popup_menu_mul" ) menu_ui = popup_menu_mul;
    else if( id == "popup_menu_mul_bbsmenu" ) menu_ui = popup_menu_mul_bbsmenu;
    else if( id == "popup_menu_mul_etc" ) menu_ui = popup_menu_mul_etc;
    else if( id == "popup_menu_dir" ) menu_ui = popup_menu_dir;
    else if( id == "popup_menu_etcdir" ) menu_ui = popup_menu_etcdir;
    else if( id == "popup_menu_bbsmenudir" ) menu_ui = popup_menu_bbsmenudir;
    else if( id == "popup_menu_favorite" ) menu_ui = popup_menu_favorite;
    else if( id == "popup_menu_favorite_mul" ) menu_ui = popup_menu_favorite_mul;
    else if( id == "popup_menu_favorite_space" ) menu_ui = popup_menu_favorite_space;
    else if( id == "popup_menu_favorite_dir" ) menu_ui = popup_menu_favorite_dir;
    else if( id == "popup_menu_favorite_com" ) menu_ui = popup_menu_favorite_com;
    else if( id == "popup_menu_favorite_vboard" ) menu_ui = popup_menu_favorite_vboard;
    else if( id == "popup_menu_select" ) menu_ui = popup_menu_select;
    else if( id == "popup_menu_history" ) menu_ui = popup_menu_history;
    else if( id == "popup_menu_history_mul" ) menu_ui = popup_menu_history_mul;
    else if( id == "popup_menu_history_vboard" ) menu_ui = popup_menu_history_vboard;
    else menu_ui = nullptr;

    assert( menu_ui );
    return menu_ui;
}

} // namespace


using namespace BBSLIST;


BBSListViewBase::BBSListViewBase( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url )
    , m_treeview( url, DNDTARGET_FAVORITE, m_columns, true, CONFIG::get_fontname( FONT_BBS ),
                  COLOR_CHAR_BBS, COLOR_BACK_BBS, COLOR_BACK_BBS_EVEN )
    , m_jump_y{ -1 }
{
    m_scrwin.add( m_treeview );
    m_scrwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

    pack_start( m_scrwin );
    show_all_children();

    m_treestore = Gtk::TreeStore::create( m_columns );

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

    // エクスパンダ表示とレベルインデント
    m_treeview.set_show_expanders( CONFIG::get_tree_show_expanders() );
    m_treeview.set_level_indentation( CONFIG::get_tree_level_indent() );

    m_treeview.add_events( Gdk::SMOOTH_SCROLL_MASK );

    // treeviewのシグナルにコネクト
    m_treeview.signal_row_expanded().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_row_exp ) );
    m_treeview.signal_row_collapsed().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_row_col ) );
    m_treeview.set_has_tooltip( true );
    m_treeview.signal_query_tooltip().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_query_tooltip) );

    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_button_release ) );
    m_treeview.sig_motion_notify().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_motion_notify ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_key_release ) );
    m_treeview.sig_scroll_event().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_scroll_event ) );
    m_treeview.sig_dropped_from_other().connect( sigc::mem_fun(*this, &BBSListViewBase::slot_dropped_from_other ) );

    ///////////////////

    // ポップアップメニューの設定
    // アクショングループを作ってビューに登録
    m_action_group = Gio::SimpleActionGroup::create();
    m_action_group->add_action( "OpenTab", sigc::mem_fun( *this, &BBSListViewBase::slot_open_tab ) );
    m_action_group->add_action( "OpenBrowser", sigc::mem_fun( *this, &BBSListViewBase::slot_open_browser ) );
    m_action_group->add_action( "OpenCacheBrowser", sigc::mem_fun( *this, &BBSListViewBase::slot_open_cache_browser ) );
    m_action_group->add_action( "AppendFavorite", sigc::mem_fun( *this, &BBSListViewBase::slot_append_favorite ) );
    m_action_group->add_action( "NewDir", sigc::mem_fun( *this, &BBSListViewBase::slot_newdir ) );
    m_action_group->add_action( "NewCom", sigc::mem_fun( *this, &BBSListViewBase::slot_newcomment ) );
    m_action_group->add_action( "NewBBSMenu", sigc::mem_fun( *this, &BBSListViewBase::slot_newbbsmenu ) );
    m_action_group->add_action( "NewEtc", sigc::mem_fun( *this, &BBSListViewBase::slot_newetcboard ) );
    m_action_group->add_action( "MoveEtc", sigc::mem_fun( *this, &BBSListViewBase::slot_moveetcboard ) );
    m_action_group->add_action( "MoveBBSMenu", sigc::mem_fun( *this, &BBSListViewBase::slot_movebbsmenu ) );
    m_action_group->add_action( "Rename", sigc::mem_fun( *this, &BBSListViewBase::slot_rename ) );
    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    m_action_group->add_action( "DeleteBBSMenu", sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    m_action_group->add_action( "DeleteEtc", sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    m_action_group->add_action( "DeleteHist", sigc::mem_fun( *this, &BBSListViewBase::delete_view_impl ) );
    m_action_group->add_action( "OpenRows", sigc::mem_fun( *this, &BBSListViewBase::open_selected_rows ) );

    m_action_group->add_action( "CheckUpdateRows",
                                sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_selected_rows ) );
    m_action_group->add_action( "CheckUpdateOpenRows",
                                sigc::mem_fun( *this, &BBSListViewBase::slot_checkupdate_open_selected_rows ) );

    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &BBSListViewBase::slot_copy_url ) );
    m_action_group->add_action( "CopyTitleURL", sigc::mem_fun( *this, &BBSListViewBase::slot_copy_title_url ) );
    m_action_group->add_action( "SelectDir", sigc::mem_fun( *this, &BBSListViewBase::slot_select_all_dir ) );

    m_action_group->add_action( "CheckUpdateDir", sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_dir ) );
    m_action_group->add_action( "CheckUpdateOpenDir",
                                sigc::mem_fun( *this, &BBSListViewBase::slot_check_update_open_dir ) );
    m_action_group->add_action( "CancelCheckUpdate", sigc::mem_fun( *this, &BBSListViewBase::stop ) );

    m_action_group->add_action( "ArrangeType",
                                sigc::bind( sigc::mem_fun( *this, &BBSListViewBase::slot_sort ), SKELETON::SORT_BY_TYPE ) );
    m_action_group->add_action( "ArrangeName",
                                sigc::bind( sigc::mem_fun( *this, &BBSListViewBase::slot_sort ), SKELETON::SORT_BY_NAME ) );

    m_action_group->add_action( "OpenAsBoard", sigc::mem_fun( *this, &BBSListViewBase::slot_opendir_as_board ) );
    m_action_group->add_action( "CreateVBoard", sigc::mem_fun( *this, &BBSListViewBase::slot_create_vboard ) );
    m_action_group->add_action( "SearchCacheBoard", sigc::mem_fun( *this, &BBSListViewBase::slot_search_cache_board ) );
    m_action_group->add_action( "ImportDat", sigc::mem_fun( *this, &BBSListViewBase::slot_import_dat ) );

    m_action_group->add_action( "PreferenceArticle", sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_article ) );
    m_action_group->add_action( "PreferenceBBSMenu", sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_bbsmenu ) );
    m_action_group->add_action( "PreferenceBoard", sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_board ) );
    m_action_group->add_action( "PreferenceImage", sigc::mem_fun( *this, &BBSListViewBase::slot_preferences_image ) );

    insert_action_group( "bbslist", m_action_group );

    // メニューは使う直前まで構築しない、 id2popupmenu() を参照
    m_builder = Gtk::Builder::create();

    // マウスジェスチャ可
    set_enable_mg( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_BBSLIST );
}


// EditListWin が不完全型のためヘッダーではデストラクタを定義できない
BBSListViewBase::~BBSListViewBase() noexcept = default;


void BBSListViewBase::save_session()
{
    save_xml();
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
bool BBSListViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    if( command == "append_item" ) append_item();
    else if( command == "append_history" ) append_history();
    else if( command == "remove_item" ) remove_item( arg1 );
    else if( command == "remove_headitem" ) remove_headitem();
    else if( command == "remove_allitems" ) remove_allitems();
    else if( command == "edit_tree" ) edit_tree();
    else if( command == "save_xml" ) save_xml();
    else if( command == "toggle_articleicon" ) toggle_articleicon( arg1 );
    else if( command == "toggle_boardicon" ) toggle_boardicon( arg1 );
    else if( command == "replace_thread" ) replace_thread( arg1, arg2 );

    else if( command == "hide_popup" ) m_treeview.hide_popup();

    else if( command == "check_update_root" ) check_update_dir( true, false );
    else if( command == "check_update_open_root" ) check_update_dir( true, true );
    else if( command == "cancel_check_update" ) stop();

    else if( command == "select_item" ) select_item( arg1 );

    return true;
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

        auto adjust = m_treeview.get_vadjustment();
        if( adjust && adjust->get_upper() > m_jump_y ){

#ifdef _DEBUG
            std::cout << "BBSListViewBase::clock_in jump to = " << m_jump_y
                      << " upper = " << static_cast<int>(adjust->get_upper()) << std::endl;
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
    for( Gtk::TreeModel::Row row : children ) {

        const int type = row2type( row );
        if( type == TYPE_COMMENT ) row[ m_columns.m_fgcolor ] = Gdk::RGBA( CONFIG::get_color( COLOR_CHAR_BBS_COMMENT ) );
        else if( type == TYPE_DIR ) set_fgcolor_of_comment( row.children() );
    }
}


//
// 再描画
//
void BBSListViewBase::redraw_view()
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::ViewBase::redraw_view " << get_url() << std::endl;
#endif

    m_treeview.redraw_view();
}


//
// 色やフォントなどの変更
//
void BBSListViewBase::relayout( const bool completely )
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

    m_treeview.hide_popup();
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
    const bool force = true;
    m_treeview.delete_selected_rows( force );
}



//
// ツリー内の全ての項目をURLを新しいアドレスに変更 ( id は未使用 )
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
bool BBSListViewBase::operate_view( const int control )
{
    if( CONTROL::operate_common( control, get_url(), BBSLIST::get_admin() ) ) return true;

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

        case CONTROL::PrevDir:
            prev_dir();
            break;

        case CONTROL::NextDir:
            next_dir();
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

            if( const Gtk::TreeModel::Row row = m_treeview.get_row( path ) ) {

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
            SHOW_POPUPMENU( SKELETON::PopupMenuPosition::view_top_left );
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
    using P = Glib::RefPtr<Gio::SimpleAction>;
    auto act_search = P::cast_dynamic( m_action_group->lookup_action( "SearchCacheBoard" ) );
    auto act_import = P::cast_dynamic( m_action_group->lookup_action( "ImportDat" ) );
    auto act_board = P::cast_dynamic( m_action_group->lookup_action( "PreferenceBoard" ) );
    auto act_article = P::cast_dynamic( m_action_group->lookup_action( "PreferenceArticle" ) );
    auto act_image = P::cast_dynamic( m_action_group->lookup_action( "PreferenceImage" ) );
    auto act_opencache = P::cast_dynamic( m_action_group->lookup_action( "OpenCacheBrowser" ) );
    auto act_opentab = P::cast_dynamic( m_action_group->lookup_action( "OpenTab" ) );

    if( act_search ) act_search->set_enabled( false );
    if( act_import ) act_import->set_enabled( false );
    if( act_board ) act_board->set_enabled( false );
    if( act_article ) act_article->set_enabled( false );
    if( act_image ) act_image->set_enabled( false );
    if( act_opencache ) act_opencache->set_enabled( false );
    if( act_opentab ) act_opentab->set_enabled( true );

    int type = path2type( m_path_selected );
    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            if( act_search ) act_search->set_enabled( true );
            if( act_import ) act_import->set_enabled( true );
            if( act_board ) act_board->set_enabled( true );
            break;

        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            if( act_article ) act_article->set_enabled( true );
            break;

        case TYPE_IMAGE:
            if( act_image && ! DBIMG::get_abone( url ) ) act_image->set_enabled( true );
            if( act_opencache && DBIMG::is_cached( url ) ) act_opencache->set_enabled( true );
            break;

        case TYPE_DIR:
            break;

        case TYPE_LINK:
            if( act_opentab ) act_opentab->set_enabled( false );
            break;
    }
}


// idからポップアップメニュー取得
Gtk::Menu* BBSListViewBase::id2popupmenu( const std::string& id )
{
    auto it = m_map_popupmenu.find( id );
    if( it == m_map_popupmenu.end() ) {
        // アクセラレーターキーやマウスジェスチャーの追加は行わない
        // CONTROL::set_menu_motion() はGTK4で廃止されるGtk::MenuのAPIを使っている
        m_builder->add_from_string( get_menu_ui( id ) );
        auto menumodel = Glib::RefPtr<Gio::MenuModel>::cast_dynamic( m_builder->get_object( id ) );
        auto pair = m_map_popupmenu.emplace( id, menumodel );
        pair.first->second.attach_to_widget( *this );
        return std::addressof( pair.first->second );
    }
    return std::addressof( it->second );
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
// 前のディレクトリに移動
//
void BBSListViewBase::prev_dir()
{
    m_treeview.prev_dir();
}

//
// 次のディレクトリに移動
//
void BBSListViewBase::next_dir()
{
    m_treeview.next_dir();
}


//
// 他のviewのtreestoreをcopyして表示
//
void BBSListViewBase::copy_treestore( const Glib::RefPtr< Gtk::TreeStore >& store )
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
    m_clicked = true;

    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    // ダブルクリック
    // button_release_eventでは event->type に必ず GDK_BUTTON_RELEASE が入る
    m_dblclicked = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclicked = true;

    // 親ウィンドウがメインウィンドウならフォーカスを移す
    if( ! get_parent_win() ) BBSLIST::get_admin()->set_command( "switch_admin" );

    return true;
}


//
// マウスボタン離した
//
bool BBSListViewBase::slot_button_release( GdkEventButton* event )
{
    if( ! m_clicked ) return true;
    m_clicked = false;

    /// マウスジェスチャ
    int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない
    if( get_control().MG_wheel_end( event ) ) return true;

    if( mg != CONTROL::NoOperation && enable_mg() ){
        operate_view( mg );
        return true;
    }

    show_status();

    m_path_selected = m_treeview.get_path_under_xy( static_cast<int>(event->x), static_cast<int>(event->y) );

    // ダブルクリックの処理のため一時的にtypeを切替える
    GdkEventType type_copy = event->type;
    if( m_dblclicked ) event->type = GDK_2BUTTON_PRESS;

    // 行を開く
    if( get_control().button_alloted( event, CONTROL::OpenBoardButton ) ) operate_view( CONTROL::OpenBoardButton );

    // タブで開く
    else if( get_control().button_alloted( event, CONTROL::OpenBoardTabButton ) ) operate_view( CONTROL::OpenBoardTabButton );

    // ポップアップメニューボタン
    else if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ) {
        SHOW_POPUPMENU( SKELETON::PopupMenuPosition::mouse_pointer );
    }

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

    const int x = static_cast<int>(event->x);
    const int y = static_cast<int>(event->y);
    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* column;
    int cell_x;
    int cell_y;

    if( m_treeview.get_path_at_pos( x, y, path, column, cell_x, cell_y ) && m_treeview.get_row( path ) ){

        Gtk::TreeModel::Row row = m_treeview.get_row( path );
        const Glib::ustring& ustr_url = row[ m_columns.m_url ];
        int type = row[ m_columns.m_type ];

        m_treeview.reset_pre_popupurl( ustr_url.raw() );

        // 画像ポップアップ
        if( type == TYPE_IMAGE ){

            if( DBIMG::get_type_ext( ustr_url.raw() ) != DBIMG::T_UNKNOWN && DBIMG::get_code( ustr_url.raw() ) != HTTP_INIT ){

                if( m_treeview.pre_popup_url() != ustr_url.raw() ){

                    SKELETON::View* view = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP, ustr_url.raw() );
                    m_treeview.show_popup( ustr_url.raw(), view );
                }
            }
            else m_treeview.hide_popup();
        }

        // ツールチップはslot_query_tooltipでセットする
        else{
            m_treeview.hide_popup();
        }
    }
    else{
        m_treeview.hide_popup();
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
    if( enable_mg() && control != CONTROL::NoOperation ){
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

    for( const CORE::DATA_INFO& info : list_info ) {
        const int type = info.type;

        switch( type ){

            case TYPE_BOARD:
            case TYPE_BOARD_UPDATE:

                m_set_board.insert( DBTREE::url_boardbase( info.url ) );
                break;

            case TYPE_VBOARD:
                m_set_board.insert( info.url );
                break;

            case TYPE_THREAD:
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

                if( m_set_bookmark ) DBTREE::set_bookmarked_thread( info.url, true );
                m_set_thread.insert( DBTREE::url_dat( info.url ) );
                break;

            case TYPE_IMAGE:

                if( m_set_bookmark ) DBIMG::set_protect( info.url, true );
                m_set_image.insert( info.url );
                break;
        }
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
// 画像キャッシュをブラウザで開く
//
void BBSListViewBase::slot_open_cache_browser()
{
    if( m_path_selected.empty() ) return;

    std::string url = path2url( m_path_selected );
    if( ! DBIMG::is_cached( url ) ) return;

    url = "file://" + DBIMG::get_cache_path( url );
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


/**
 * @brief メニューで外部BBSMENU追加を選択
 */
void BBSListViewBase::slot_newbbsmenu()
{
    add_newbbsmenu( false, "", "" );
}


//
// メニューで外部板追加を選択
//
void BBSListViewBase::slot_newetcboard()
{
    add_newetcboard( false, "", "", "", "" );
}


/**
 * @brief メニューで外部BBSMENU編集を選択
 */
void BBSListViewBase::slot_movebbsmenu()
{
    add_newbbsmenu( true, "", "" );
}


//
// メニューで外部板編集を選択
//
void BBSListViewBase::slot_moveetcboard()
{
    add_newetcboard( true, "", "", "", "" );
}


/** @brief 外部BBSMENU追加
 *
 * @param[in] move  true なら編集モード
 * @param[in] _url  BBSMENUのURL
 * @param[in] _name BBSMENUの名前
 */
void BBSListViewBase::add_newbbsmenu( const bool move, const std::string& _url, const std::string& _name )
{
    if( m_path_selected.empty() ) return;

    std::string url_old;
    std::string name_old;

    std::string url = _url;
    std::string name = _name;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::add_newbbsmenu move = " << move << std::endl;
#endif

    if( move ) {
        url_old = path2url( m_path_selected );
        name_old = path2name( m_path_selected );

#ifdef _DEBUG
        std::cout << "url_old = " << url_old << std::endl
                  << "name_old = " << name_old << std::endl
                  << "bbsname_name = " << DBTREE::bbsmenu_name( url_old ) << std::endl;
#endif

        if( DBTREE::bbsmenu_name( url_old ) != name_old ) return;

        if( url.empty() ) url = url_old;
        if( name.empty() ) name = name_old;
    }

    BBSLIST::AddEtcBBSMenuDialog diag{ get_parent_win(), move, url, name };
    if( diag.run() != Gtk::RESPONSE_OK ) return;

    diag.hide();

    std::string url_org = MISC::utf8_trim( diag.get_url() );
    name = MISC::utf8_trim( diag.get_name() );
    url = url_org;

    if( name.empty() || url.empty() ) {
        SKELETON::MsgDiag mdiag( get_parent_win(), "BBSMENU名またはアドレスが空白です",
                                 false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        mdiag.hide();
        add_newbbsmenu( move, url_org, name );
        return;
    }

    // http[s] が無ければ付ける
    if( url.find( "://" ) == std::string::npos ) url = "http://" + url;

#ifdef _DEBUG
    std::cout << "url_old = " << url_old << std::endl
              << "name_old = " << name_old << std::endl
              << "url = " << url << std::endl
              << "name = " << name << std::endl;
#endif

    // 既に登録されているか確認
    if( ! move && ! DBTREE::bbsmenu_name( url ).empty() ) {
        SKELETON::MsgDiag mdiag( get_parent_win(), name + "\n" + url + "\n\nは既に登録されています",
                                 false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        mdiag.hide();
        add_newbbsmenu( move, url_org, name );
        return;
    }

    // データベースに登録してツリーに表示
    if( ! move && DBTREE::add_bbsmenu( url, name ) ) {

        CORE::DATA_INFO_LIST list_info;
        CORE::DATA_INFO info;
        info.type = TYPE_BBSMENU;
        info.url = url;
        info.name = name;
        info.path = m_path_selected.to_string();
        list_info.push_back( info );

        constexpr bool before = false;
        constexpr bool scroll = false;
        constexpr bool force = true; // 強制的に追加
        constexpr bool cancel_undo_commit = false;
        constexpr int check_dup = 0; // 項目の重複チェックをしない
        m_treeview.append_info( list_info, m_path_selected, before, scroll, force, cancel_undo_commit, check_dup );
        m_path_selected = m_treeview.get_current_path();

        // bbsmenu.txt保存
        DBTREE::save_bbsmenu();
        DBTREE::download_bbsmenu( url );
    }

    // 編集
    else if( move && DBTREE::move_bbsmenu( url_old, url, name_old, name ) ) {

        Gtk::TreeModel::Row row = m_treeview.get_row( m_path_selected );
        if( row ){
            row[ m_columns.m_url ] = url;
            row[ m_columns.m_name ] = name;
            static_cast<void>( row ); // cppcheck: unreadVariable
            DBTREE::download_bbsmenu( url );
        }
    }

    else {

        SKELETON::MsgDiag mdiag( get_parent_win(), "外部BBSMENUの登録に失敗しました。アドレスを確認してください",
                                 false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        mdiag.hide();
        add_newbbsmenu( move, url_org, name );
        return;
    }
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
        size_t i = basicauth.find( ':' );

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

        std::string url_org = MISC::utf8_trim( diag.get_url() );
        name = MISC::utf8_trim( diag.get_name() );
        url = url_org;
        id = MISC::utf8_trim( diag.get_id() );
        passwd = MISC::utf8_trim( diag.get_passwd() );
        if( ! id.empty() && ! passwd.empty() ) basicauth = id + ":" + passwd;

        if( name.empty() || url.empty() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "板名またはアドレスが空白です", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            mdiag.hide();
            add_newetcboard( move, url_org, name, id, passwd );
            return;
        }

        // http[s] が無ければ付ける
        if( url.find( "://" ) == std::string::npos ) url = "http://" + url;

        // .htmlを取り除く
        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;

        if( regex.exec( "(.*)/[^/]+\\.html?$" , url, offset, icase, newline, usemigemo, wchar ) ) url = regex.str( 1 );

        // 末尾の / を取り除く
        url.erase( url.find_last_not_of( '/' ) + 1 );

        // url の最後に/を付ける
        url.push_back( '/' );

        // boardid 取得
        if( ! regex.exec( "(https?://.*)/([^/]*)/$" , url, offset, icase, newline, usemigemo, wchar ) ){
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
            info.path = m_path_selected.to_string();
            list_info.push_back( info );

            const bool before = false;
            const bool scroll = false;
            const bool force = true;  // 強制的に追加
            const bool cancel_undo_commit = false;
            const int check_dup = 0; // 項目の重複チェックをしない
            m_treeview.append_info( list_info, m_path_selected, before, scroll, force, cancel_undo_commit, check_dup );
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
                static_cast<void>( row ); // cppcheck: unreadVariable
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

    const std::string url = path2url( m_path_selected );
    std::string name;

    const int type = path2type( m_path_selected );
    switch( type ){
        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
        case TYPE_THREAD_OLD:
            name = MISC::to_plain( DBTREE::article_subject( url ) );
            break;

        default:
            name = path2name( m_path_selected );
    }

    MISC::CopyClipboard( name + '\n' + url + '\n' );
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
    for( const Gtk::TreeIter& iter : child ) expand_all_dir( m_treestore->get_path( iter ) );

    m_treeview.scroll_to_row( m_treestore->get_path( *child.begin() ), 0 );
    for( const Gtk::TreeIter& iter : child ) {
        m_treeview.get_selection()->select( iter );
        m_treeview.select_all_dir( m_treestore->get_path( iter ) );
    }
}


//
// ディレクトリ以下を更新チェック
//
// root : true ならルートから検索する。falseの場合は m_path_selected にパスをセットしておくこと
// open : チェック後に更新していたら開く
//
void BBSListViewBase::check_update_dir( const bool root, const bool open )
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::check_update_dir root = " << root << std::endl;
#endif

    Gtk::TreePath path;
    if( ! root ){
        if( m_path_selected.empty() ) return;
        path = m_path_selected;

#ifdef _DEBUG
        std::cout << "path = " << path.to_string() << std::endl;
#endif
    }

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, path );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const int type = row2type( row );
        const std::string url = row2url( row );
#ifdef _DEBUG
        std::cout << row2name( row ) << std::endl;
#endif

        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) CORE::get_checkupdate_manager()->push_back( DBTREE::url_dat( url ), open );
        else if( CONFIG::get_check_update_board() && ( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ) )
            CORE::get_checkupdate_manager()->push_back( DBTREE::url_boardbase( url ), open );

    }

    CORE::get_checkupdate_manager()->run();
}

void BBSListViewBase::slot_check_update_dir()
{
    check_update_dir( false, false );
}

void BBSListViewBase::slot_check_update_open_dir()
{
    check_update_dir( false, true );
}


//
// ディレクトリをスレ一覧に表示
//
void BBSListViewBase::slot_opendir_as_board()
{
    if( m_path_selected.empty() ) return;
    const size_t dirid = m_treeview.path_to_dirid( m_path_selected );
    if( ! dirid ) return;

    const std::string tab = "newtab";
    const std::string mode = "";
    CORE::core_set_command( "open_sidebar_board", get_url(), tab, mode, std::to_string( dirid ) );
}


//
// 仮想板作成
//
void BBSListViewBase::slot_create_vboard()
{
    if( m_path_selected.empty() ) return;
    const size_t dirid = m_treeview.path_to_dirid( m_path_selected );
    if( ! dirid ) return;

    CORE::DATA_INFO_LIST list_info;
    CORE::DATA_INFO info;
    info.type = TYPE_VBOARD;
    info.parent = BBSLIST::get_admin()->get_win();
    info.url = get_url() + SIDEBAR_SIGN + std::to_string( dirid );
    info.name = path2name( m_path_selected );
    info.path = m_path_selected.to_string();
    list_info.push_back( info );
    CORE::SBUF_set_list( list_info );

    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}


//
// 更新チェックキャンセル
//
void BBSListViewBase::stop()
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::stop " << get_url() << std::endl;
#endif

    CORE::get_checkupdate_manager()->stop();
}


//
// キャッシュ内のログ検索
//
void BBSListViewBase::slot_search_cache_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    CORE::core_set_command( "open_article_searchlog", url, "", "noexec" );
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


/**
 * @brief 外部BBSMENUのプロパティ表示
 */
void BBSListViewBase::slot_preferences_bbsmenu()
{
    if( m_path_selected.empty() ) return;
    const std::string url = path2url( m_path_selected );

    Glib::ustring modified = "最終更新日時 ：";

    if( DBTREE::bbsmenu_get_date_modified( url ).empty() ) modified += "未取得";
    else {
        const std::time_t time_modified = DBTREE::bbsmenu_get_time_modified( url );
        modified += MISC::timettostr( time_modified, MISC::TIME_WEEK ) + " ( "
                    + MISC::timettostr( time_modified, MISC::TIME_PASSED ) + " )";
    }

    SKELETON::MsgDiag mdiag( get_parent_win(), modified, false );
    mdiag.set_title( "BBSMENUのプロパティ" );
    mdiag.run();
}


//
// 板プロパティ表示
//
void BBSListViewBase::slot_preferences_board()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_BOARD, DBTREE::url_boardbase( url ) );
    pref->run();
}



//
// スレプロパティ表示
//
void BBSListViewBase::slot_preferences_article()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_ARTICLE, DBTREE::url_dat( url ) );
    pref->run();
}


//
// 画像プロパティ表示
//
void BBSListViewBase::slot_preferences_image()
{
    if( m_path_selected.empty() ) return;
    std::string url = path2url( m_path_selected );

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_IMAGE, url );
    pref->run();
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
        && path.size() == 1  // 子フォルダの時は閉じない
        ){
        m_expanding = true;
        m_treeview.collapse_all();
        m_treeview.expand_row( path, false );
        m_expanding = false;
    }

    m_treeview.set_cursor( path );
    if( CONFIG::get_scroll_tree() ) m_treeview.scroll_to_row( path, 0.1f );
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
bool BBSListViewBase::open_row( Gtk::TreePath& path, const bool tab )
{
    const Gtk::TreeModel::Row row = m_treeview.get_row( path );
    if( ! row ) return false;

    std::string str_tab = "false";
    if( tab ) str_tab = "opentab";

    const std::string str_mode = "";

    const std::string url = row2url( row );
    const int type = row2type( row );

    if( type != TYPE_DIR && url.empty() ) return false;

    bool open_image = false;
    std::string open_mode;
    switch( type ){

        case TYPE_BOARD:
        case TYPE_BOARD_UPDATE:
            CORE::core_set_command( "open_board", url, str_tab, str_mode );
            break;

        case TYPE_THREAD_OLD:
            toggle_articleicon( url ); // break;しない
            [[fallthrough]];
        case TYPE_THREAD:
        case TYPE_THREAD_UPDATE:
            CORE::core_set_command( "open_article", url, str_tab, str_mode );
            break;

        case TYPE_IMAGE:

            if( DBIMG::get_abone( url ) ) {
                SKELETON::MsgDiag mdiag( get_parent_win(), "あぼ〜んされています", false,
                                         Gtk::MESSAGE_INFO, Gtk::BUTTONS_YES_NO );
                std::string abone_reason = DBIMG::get_img_abone_reason( url );
                if( ! abone_reason.empty() ) {
                    abone_reason = MISC::replace_str( abone_reason, "<br>", "\n" );
                    abone_reason.append( "\n\n" );
                }
                abone_reason.append( DBIMG::kTemporaryMosaicQuestion );
                mdiag.set_secondary_text( abone_reason );
                const int response = mdiag.run();
                if( response == Gtk::RESPONSE_YES ) {
                    open_image = true;
                    open_mode = "force_mosaic";
                }
            }
            else {
                open_image = true;
            }

            if( open_image ) {
                CORE::core_set_command( "open_image", url, open_mode );
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

        case TYPE_VBOARD:
            CORE::core_set_command( "open_sidebar_board", url, str_tab, str_mode, "", "set_history" );
            break;
    }

#ifdef _DEBUG
    std::cout << "BBSListViewBase::open_row : path = " << path.to_string() << " tab = " << tab << std::endl;
#endif

    // treeviewが編集されていたらxml保存
    if( type != TYPE_DIR && m_treeview.is_updated() ){
        save_xml();
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

    for( const Gtk::TreeIter& iter : m_treeview.get_selected_iterators() ) {

        Gtk::TreeModel::Row row = *iter;
        const int type = row2type( row );
        const std::string url = row2url( row );

        switch( type ){

            case TYPE_BOARD:
            case TYPE_BOARD_UPDATE:
                if( ! list_url_board.empty() ) list_url_board.push_back( ' ' );
                list_url_board.append( url );
                break;

            case TYPE_THREAD:
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:
                if( ! list_url_article.empty() ) list_url_article.push_back( ' ' );
                list_url_article.append( url );
                break;

            case TYPE_IMAGE:
                if( ! DBIMG::get_abone( url ) ) CORE::core_set_command( "open_image", url );
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
    for( const Gtk::TreeIter& iter : m_treeview.get_selected_iterators() ) {

        Gtk::TreeModel::Row row = *iter;
        const int type = row2type( row );
        const std::string url = row2url( row );

        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE ) {
            CORE::get_checkupdate_manager()->push_back( url, open );
        }
        else if( CONFIG::get_check_update_board() && ( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ) ) {
            CORE::get_checkupdate_manager()->push_back( url, open );
        }
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


void BBSListViewBase::slot_sort( const int mode )
{
    m_treeview.sort( m_path_selected, mode );
}


//
// path -> url 変換
//
std::string BBSListViewBase::path2rawurl( const Gtk::TreePath& path )
{
    const Gtk::TreeModel::Row row = m_treeview.get_row( path );
    return row2url( row );
}


// 移転をチェックするバージョン
std::string BBSListViewBase::path2url( const Gtk::TreePath& path )
{
    const Gtk::TreeModel::Row row = m_treeview.get_row( path );
    std::string url = row2url( row );

    // url を最新のものに変換しておく
    switch( row2type( row ) ){

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
// 板の場合は boardbase
// スレの場合は dat 型のアドレスを返す
//
std::string BBSListViewBase::row2url( const Gtk::TreeModel::Row& row ) const
{
    if( ! row ) return {};
    const Glib::ustring& ustr_url = row[ m_columns.m_url ];
    return ustr_url.raw();
}



//
// path -> name 変換
//
std::string BBSListViewBase::path2name( const Gtk::TreePath& path )
{
    const Gtk::TreeModel::Row row = m_treeview.get_row( path );
    return row2name( row );
}



//
// row -> name 変換
//
std::string BBSListViewBase::row2name( const Gtk::TreeModel::Row& row ) const
{
    if( ! row ) return {};
    const Glib::ustring& ustr_name = row[ m_columns.m_name ];
    return ustr_name.raw();
}


//
// path -> type 変換
//
int BBSListViewBase::path2type( const Gtk::TreePath& path )
{
    const Gtk::TreeModel::Row row = m_treeview.get_row( path );
    return row2type( row );
}


//
// row -> type 変換
//
int BBSListViewBase::row2type( const Gtk::TreeModel::Row& row ) const
{
    if( ! row ) return TYPE_UNKNOWN;
    return row[ m_columns.m_type ];
}


//
// row -> dirid 変換
//
size_t BBSListViewBase::row2dirid( const Gtk::TreeModel::Row& row ) const
{
    if( !row ) return 0;
    return row[ m_columns.m_dirid ];
}


/**
 * @brief 外部BBSMENUのディレクトリか
 */
bool BBSListViewBase::is_bbsmenudir( Gtk::TreePath path )
{
    const std::string name = path2name( path );

#ifdef _DEBUG
    std::cout << "BBSListViewBase:is_bbsmenudir path = " << path.to_string() << " name = " << name << std::endl;
#endif
    return name == SUBDIR_BBSMENU;
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
bool BBSListViewBase::is_etcboard( const Gtk::TreeModel::iterator& it )
{
    Gtk::TreePath path = get_treestore()->get_path( *it );
    return is_etcboard( path );
}


bool BBSListViewBase::is_etcboard( Gtk::TreePath path )
{
    path.up();
    return is_etcdir( path );
}


/** @brief 外部BBSMENUか
 *
 * @param[in] it TreeModelのイテレーター
 */
bool BBSListViewBase::is_bbsmenu( const Gtk::TreeModel::iterator& it )
{
    Gtk::TreePath path = get_treestore()->get_path( *it );
    return is_bbsmenu( path );
}


/** @brief 外部BBSMENUか
 *
 * @param[in] path TreeViewのパス
 */
bool BBSListViewBase::is_bbsmenu( Gtk::TreePath path )
{
    path.up();
    return is_bbsmenudir( path );
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
    const auto adjust = m_treeview.get_vadjustment();
    if( adjust )
    {
        if( m_jump_y != -1 && adjust->get_upper() > m_jump_y ) y = m_jump_y;
        else  y = static_cast<int>(adjust->get_value());
    }
    else if( m_jump_y != -1 ) y = m_jump_y;

    // 選択中のパス
    std::string path;
    Gtk::TreeModel::Path focused_path = m_treeview.get_current_path();
    if( ! focused_path.empty() ) path = focused_path.to_string();

    // ルート要素に属性( path, y )の値を設定
    XML::Dom* root = m_document.get_root_element( root_name );
    if( root ){
        if( ! m_date_modified.empty() ) root->setAttribute( "date_modified", m_date_modified );
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
    std::cout << " 子ノード数=" << m_document.size() << std::endl;
#endif

    m_treeview.xml2tree( m_document, m_treestore, root_name );

    // ルート要素を取り出す
    const XML::Dom* root = m_document.get_root_element( root_name );
    if( root ){

	// ルート要素から属性( date_modified, path, y )の値を取得
	m_date_modified = root->getAttribute( "date_modified" );
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
    std::cout << "BBSListViewBase::update_urls " << get_url() << std::endl;
#endif

    bool updated = false;

    // 移転情報の保存( Root::save_movetable() )を無効にする。
    // 無効にしないと DBTREE::url_boardbase() -> Root::get_board() 経由で
    // 繰り返し Root::save_movetable() が実行されてJDが固まったようになる
    DBTREE::set_enable_save_movetable( false );

    m_set_board.clear();
    m_set_thread.clear();
    m_set_image.clear();

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& ustr_url = row[ m_columns.m_url ];
        const int type = row[ m_columns.m_type ];

#ifdef _DEBUG
        std::cout << row2name( row ) << std::endl;
#endif

        std::string url_new;
        switch( type ){

            case TYPE_BOARD: // 板
            case TYPE_BOARD_UPDATE:

                url_new = DBTREE::url_boardbase( ustr_url.raw() );
                if( ustr_url.raw() != url_new ){
                    updated = true;
                    row[ m_columns.m_url ] = url_new;
#ifdef _DEBUG
                    std::cout << ustr_url << " -> " << url_new << std::endl;
#endif
                }

                m_set_board.insert( url_new );
                break;

            case TYPE_VBOARD:
                m_set_board.insert( ustr_url.raw() );
                break;

            case TYPE_THREAD: // スレ
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

                url_new = DBTREE::url_dat( ustr_url.raw() );
                if( ustr_url.raw() != url_new ){
                    updated = true;
                    row[ m_columns.m_url ] = url_new;
#ifdef _DEBUG
                    std::cout << ustr_url << " -> " << url_new << std::endl;
#endif
                }

                m_set_thread.insert( url_new );
                break;

            case TYPE_IMAGE:
                m_set_image.insert( ustr_url.raw() );
                break;
        }
    }


    DBTREE::set_enable_save_movetable( true );

    if( updated ){
        save_xml();
        DBTREE::save_movetable();
    }
}


//
// アイコン表示(スレ)の切り替え
//
void BBSListViewBase::toggle_articleicon( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return;

    // ツリーの中に無い場合は処理しない
    if( m_set_thread.find( url ) == m_set_thread.end() ) return;

    bool erase = true;
    int type = TYPE_THREAD;
    const int status = DBTREE::article_status( url );
    if( status & STATUS_OLD ) type = TYPE_THREAD_OLD;
    else if( status & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::toggle_articleicon url = " << url << " type = " << type << std::endl;
#endif

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& url_row = row[ m_columns.m_url ];
        const int type_row = row[ m_columns.m_type ];

        if( type_row == TYPE_THREAD || type_row == TYPE_THREAD_UPDATE || type_row == TYPE_THREAD_OLD ){

            if( url == url_row.raw() ){
#ifdef _DEBUG
                std::cout << "hit " << url << " == " << url_row << std::endl;
                std::cout << row2name( row ) << std::endl;
#endif
                row[ m_columns.m_type ] = type;
                row[ m_columns.m_image ] = XML::get_icon( type );

                erase = false;
            }
        }
    }

    if( erase ) m_set_thread.erase( url );
}



//
// アイコン表示(板)の切り替え
//
void BBSListViewBase::toggle_boardicon( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return;

    const std::string url_boardbase = DBTREE::url_boardbase( url );

    // ツリーの中に無い場合は処理しない
    if( m_set_board.find( url_boardbase ) == m_set_board.end() ) return;

    bool erase = true;
    int type = TYPE_BOARD;
    const int status = DBTREE::board_status( url );
    if( status & STATUS_UPDATE ) type = TYPE_BOARD_UPDATE;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::toggle_boardicon url = " << url_boardbase << " type = " << type << std::endl;
#endif

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& url_row = row[ m_columns.m_url ];
        const int type_row = row[ m_columns.m_type ];

        if( type_row == TYPE_BOARD || type_row == TYPE_BOARD_UPDATE ){

            if( url_boardbase == url_row.raw() ){
#ifdef _DEBUG
                std::cout << "hit " << url_boardbase << " == " << url_row << std::endl;
                std::cout << row2name( row ) << std::endl;
#endif
                row[ m_columns.m_type ] = type;
                row[ m_columns.m_image ] = XML::get_icon( type );

                erase = false;
            }
        }
    }

    if( erase ) m_set_board.erase( url_boardbase );
}



//
// URLを選択
//
void BBSListViewBase::select_item( const std::string& url )
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return;

    std::string url_item;

    if( m_set_thread.find( url ) != m_set_thread.end()
            || m_set_image.find( url ) != m_set_image.end()){
        // スレまたは画像の場合
        url_item = url;
    }
    else {
        // 板以外の履歴は処理しない
        if( get_url() == URL_HISTTHREADVIEW || get_url() == URL_HISTCLOSEVIEW ||
            get_url() == URL_HISTCLOSEIMGVIEW ) return;

        // 板の場合
        url_item = DBTREE::url_boardbase( url );

        // 未登録の画像などで、板が見つからない場合は処理しない
        if( url_item.empty() ) return;
    }

    // 現在選択しているものと一致する場合は何もしない
    Gtk::TreePath current_path = m_treeview.get_current_path();
    if( ! current_path.empty() ){
        // 板一覧やお気に入りに、該当するURLが2つある場合がある
        if( url_item == path2rawurl( current_path ) || url_item == path2url( current_path ) ) return;
    }

    Gtk::TreePath closed_path;
    int closed_found = false;

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& ustr_url = row[ m_columns.m_url ];

        if( url_item == ustr_url.raw() ){
            Gtk::TreePath path = GET_PATH( row );

            // 最初に見つかったものにフォーカスする
            if( m_treeview.is_expand( path ) ){
                // 開いているエントリ
                m_treeview.get_selection()->unselect_all();
                m_treeview.set_cursor( path );

                return;
            }

            // 最初の閉じたエントリを覚えておく
            if( ! closed_found ){
                closed_found = true;
                closed_path = path;
            }
        }
    }

    // 開いたエントリが見つからない
    if( closed_found && CONFIG::get_select_item_sync() == 2 ){
        // 閉じたエントリの上位フォルダを開いてフォーカスする
        m_treeview.get_selection()->unselect_all();
        m_treeview.expand_parents( closed_path );
        m_treeview.set_cursor( closed_path );
    }
}



//
// 新スレ移行時などにスレの url と 名前を変更
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

    const std::string name_new = MISC::to_plain( DBTREE::article_modified_subject( urldat_new ) );
    if( name_new.empty() ) return;

    bool show_diag = CONFIG::show_diag_replace_favorite();
    int mode = CONFIG::get_replace_favorite_next();
    if( ! show_diag && mode == REPLACE_NEXT_NO ) return;

    const std::string urldat = DBTREE::url_dat( url );
    const std::string name_old = MISC::to_plain( DBTREE::article_modified_subject( urldat ) );

    int type = TYPE_THREAD;
    const int status = DBTREE::article_status( urldat_new );
    if( status & STATUS_OLD ) type = TYPE_THREAD_OLD;
    if( status & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;

#ifdef _DEBUG
    std::cout << "name_new = " << name_new << std::endl
              << "name_old = " << name_old << std::endl
              << "type = " << type << std::endl;
#endif

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& ustr_url = row[ m_columns.m_url ];

        switch( row[ m_columns.m_type ] ){

            case TYPE_THREAD:
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

                if( urldat == ustr_url.raw() ){

                    if( show_diag ){

                        show_diag = false;

                        SKELETON::MsgCheckDiag mdiag( get_parent_win(),
                                                      "お気に入りに前スレが登録されています。\n\n名前とアドレスを新スレの物に置き換えますか？"
                                                      , "今後表示しない(_D)",
                                                      Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE
                            );

                        mdiag.add_default_button( g_dgettext( GTK_DOMAIN, "_No" ), Gtk::RESPONSE_NO );
                        mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );
                        mdiag.add_button( "新スレをお気に入りに追加(_F)", Gtk::RESPONSE_CANCEL );

                        mdiag.set_title( "お気に入り更新" );
                        const int ret = mdiag.run();

                        switch( ret ){

                            case Gtk::RESPONSE_YES: mode = REPLACE_NEXT_YES; break;
                            case Gtk::RESPONSE_NO: mode = REPLACE_NEXT_NO; break;
                            case Gtk::RESPONSE_CANCEL: mode = REPLACE_NEXT_ADD; break;
                        }

                        if( mdiag.get_chkbutton().get_active() ){

                            CONFIG::set_show_diag_replace_favorite( false );
                            CONFIG::set_replace_favorite_next( mode );
                        }

                        if( mode == REPLACE_NEXT_NO ) return;
                    }

                    // 行を開いた時にxmlを保存
                    m_treeview.set_updated( true );

                    // 置き換え
                    if( mode == REPLACE_NEXT_YES ){

                        row[ m_columns.m_url ] = urldat_new;

                        // 名前が古いものであったら更新
                        // 手動で変更されていたらそのまま
                        const Glib::ustring& ustr_name = row[ m_columns.m_name ];
#ifdef _DEBUG
                        std::cout << "name_row = " << ustr_name << std::endl;
#endif
                        if( MISC::utf8_trim( ustr_name.raw() ) == name_old ){
#ifdef _DEBUG
                            std::cout << "replace name\n";
#endif
                            row[ m_columns.m_name ] = name_new;
                        }

                        row[ m_columns.m_type ] = type;
                        row[ m_columns.m_image ] = XML::get_icon( type );
                    }

                    // 追加
                    else if( mode == REPLACE_NEXT_ADD ){

                        CORE::DATA_INFO_LIST list_info;
                        CORE::DATA_INFO info;
                        info.type = type;
                        info.parent = BBSLIST::get_admin()->get_win();
                        info.url = urldat_new;
                        info.name = name_new;
                        info.path = Gtk::TreePath( "0" ).to_string();
                        list_info.push_back( info );
                        CORE::SBUF_set_list( list_info );

                        CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );

                        return;
                    }
                }
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

    constexpr bool icase_name = true; // 大文字小文字区別しない
    constexpr bool newline_name = true; // . に改行をマッチさせない
    constexpr bool usemigemo_name = true; // migemo使用
    constexpr bool wchar_name = true;  // 全角半角の区別をしない
    const JDLIB::RegexPattern regex_name( query, icase_name, newline_name, usemigemo_name, wchar_name );

    constexpr bool icase_url = true; // 大文字小文字区別しない
    constexpr bool newline_url = true;
    constexpr bool usemigemo_url = false;
    constexpr bool wchar_url = false;
    const JDLIB::RegexPattern regex_url( query, icase_url, newline_url, usemigemo_url, wchar_url );

    JDLIB::Regex regex;

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

                path = GET_PATH( *( std::prev( m_treestore->children().end() ) ) );
                Gtk::TreePath path_tmp = path;
                while( m_treeview.get_row( path_tmp ) ){
                    path = path_tmp;
                    path_tmp = m_treeview.next_path( path_tmp, false );
                }
            }

            else path = m_treeview.prev_path( path, false );
        }

        const std::string name = path2name( path );
        const std::string url = path2url( path );

        const size_t offset = 0;
        if( regex.match( regex_name, name, offset ) || regex.match( regex_url, url, offset ) ) hit = true;

        // 一周したら終わり
        if( path == path_start ) break;

        // カーソル移動
        if( hit ){
            m_treeview.expand_parents( path );
            m_treeview.scroll_to_row( path, 0.1f );
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
    CORE::DATA_INFO_LIST list_info_bkup = CORE::SBUF_list_info();

    std::string path_str;

    const int show_diag = CONFIG::get_show_favorite_select_diag();
    if( show_diag == 1 ) path_str = "-1";
    else if( show_diag == 2 ) path_str = "";
    else{

        // 挿入先ダイアログ表示
        Gtk::Window* parent = ( *list_info_bkup.begin() ).parent;
        SelectListDialog diag( parent, get_url(), get_treestore() );
        if( diag.run() != Gtk::RESPONSE_OK ) return;

        path_str = diag.get_path();
        if( list_info_bkup.size() == 1 && ! diag.get_name().empty() ) ( *list_info_bkup.begin() ).name = diag.get_name();
    }

    bool before = false;
    Gtk::TreePath path;

    // 先頭
    if( path_str == "-1" ){
        path = Gtk::TreePath( "0" );
        before = true;
    }
    // 最後
    else if( path_str.empty() ) path = Gtk::TreePath();

    else path = Gtk::TreePath( path_str );

    const bool scroll = true;
    const bool force = false;
    const bool cancel_undo_commit = false;
    const int check_dup = CONFIG::get_check_favorite_dup(); // 重複チェックするか

    const CORE::DATA_INFO_LIST list_info = m_treeview.append_info( list_info_bkup, path, before, scroll, force, cancel_undo_commit, check_dup );
    CORE::SBUF_clear_info();
    slot_dropped_from_other( list_info );

    // 行を開いた時にxmlを保存
    m_treeview.set_updated( true );
}


//
// 履歴のセット
//
// 先頭にアイテムを追加する。ツリーにアイテムが含まれている場合は移動する
// あらかじめ共有バッファにデータを入れておくこと
//
void BBSListViewBase::append_history()
{
    if( CORE::SBUF_size() == 0 ) return;

    CORE::DATA_INFO_LIST list_info = CORE::SBUF_list_info();
    CORE::DATA_INFO_LIST::iterator it_info = list_info.begin();
    ( *it_info ).path = Gtk::TreePath( "0" ).to_string();

#ifdef _DEBUG
    std::cout << "BBSListViewBase::append_history url = " << ( *it_info ).url << std::endl;
#endif

    // ツリーにアイテムが含まれている場合は削除
    // 履歴はサブディレクトリが無いと仮定してサブディレクトリの探査はしない
    if( ( ( *it_info ).type == TYPE_THREAD && m_set_thread.find( ( *it_info ).url ) != m_set_thread.end() )
        || ( ( ( *it_info ).type == TYPE_BOARD || ( *it_info ).type == TYPE_VBOARD )  && m_set_board.find( ( *it_info ).url ) != m_set_board.end() )
        || ( ( *it_info ).type == TYPE_IMAGE && m_set_image.find( ( *it_info ).url ) != m_set_image.end() )
        ){

        std::vector< Gtk::TreePath > del_path;

        for( Gtk::TreeModel::Row row : m_treestore->children() ) {
            if( row2url( row ) == ( *it_info ).url ) del_path.push_back( GET_PATH( row ) );
        }

        for( int i = del_path.size() -1; i >= 0 ; --i ){
#ifdef _DEBUG
            std::cout << "erase " << del_path[ i ].to_string() << std::endl;
#endif
            m_treestore->erase( m_treeview.get_row( del_path[ i ] ) );
        }
    }

    // 先頭にアイテム追加
    Gtk::TreePath path;
    if( ! m_treestore->children().empty() ) path = Gtk::TreePath( "0" );
    const bool before = true;
    const bool subdir = false;

    m_treeview.append_one_row( ( *it_info ).url, ( *it_info ).name, ( *it_info ).type, ( *it_info ).dirid, ( *it_info ).data, path, before, subdir );

    // サイズが越えていたら最後を削除
    const Gtk::TreeNodeChildren children = m_treestore->children();
    const int history_size = CONFIG::get_historyview_size();
    if( static_cast< int >( children.size() ) > history_size ) {
        const auto end = children.end();
        auto it = std::next( children.begin(), history_size );
        while( it != end ) {
            it = m_treestore->erase( *it );
#ifdef _DEBUG
            std::cout << "erase bottom\n";
#endif
        }
    }

    goto_top();

    CORE::SBUF_clear_info();
    slot_dropped_from_other( list_info );
}


//
// 履歴を DATA_INFO_LIST 型で取得
//
void BBSListViewBase::get_history( CORE::DATA_INFO_LIST& info_list ) const
{
    info_list.clear();

    CORE::DATA_INFO info;

    // 履歴はサブディレクトリが無いと仮定してサブディレクトリの探査はしない
    for( Gtk::TreeModel::Row row : m_treestore->children() ) {

        info.type = row2type( row );
        info.url = row2url( row );
        info.name = row2name( row );
        info.dirid = row2dirid( row );

        info_list.push_back( info );
    }
}


//
// 指定したidのディレクトリに含まれるスレのアドレスを取得
//
void BBSListViewBase::get_threads( const size_t dirid, std::vector< std::string >& list_url )
{
    list_url.clear();

#ifdef _DEBUG
    std::cout << "BBSListViewBase::get_threads " << dirid << std::endl;
#endif

    Gtk::TreePath path = m_treeview.dirid_to_path( dirid );
    if( dirid && path.empty() ) return;

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, path );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const int type = row2type( row );
        if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE || type == TYPE_THREAD_OLD ){

#ifdef _DEBUG
            std::cout << row2name( row ) << std::endl;
#endif

            list_url.push_back( row2url( row ) );
        }
    }
}


// 指定したidのディレクトリの名前を取得
std::string BBSListViewBase::get_dirname( const int dirid )
{
    if( ! dirid ) return get_label();

    return path2name( m_treeview.dirid_to_path( dirid ) );
}


//
// url で指定した項目を削除
//
void BBSListViewBase::remove_item( const std::string& url )
{
    std::string url_target = DBTREE::url_dat( url );
    if( url_target.empty() ) url_target = DBTREE::url_boardbase( url );
    if( url_target.empty() && DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ) url_target = url;
    if( url_target.empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::remove_item url = " << url_target << std::endl;
#endif

    std::list< Gtk::TreePath > list_path;

    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;
        const Glib::ustring& url_row = row[ m_columns.m_url ];
        const int type = row[ m_columns.m_type ];

        switch( type ){

            case TYPE_BOARD: // 板
            case TYPE_BOARD_UPDATE:

            case TYPE_THREAD: // スレ
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

            case TYPE_IMAGE: // 画像

                if( url_row.raw() == url_target ){
#ifdef _DEBUG
                    std::cout << "hit " << url_row << " == " << url_target << std::endl;
                    std::cout << row2name( row ) << std::endl;
#endif
                    list_path.push_back( it.get_path() );
                }
                break;
        }
    }

    const bool force = true;
    m_treeview.delete_path( list_path, force );
}


//
// 先頭項目を削除
//
void BBSListViewBase::remove_headitem()
{
    if( ! m_ready_tree ) return;
    if( m_treestore->children().empty() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::remove_headitem\n";
#endif

    std::list< Gtk::TreePath > list_path;
    SKELETON::EditTreeViewIterator it( m_treeview, m_columns, Gtk::TreePath() );
    list_path.push_back( it.get_path() );

    const bool force = true;
    m_treeview.delete_path( list_path, force );
}


//
// 全項目を削除
//
void BBSListViewBase::remove_allitems()
{
#ifdef _DEBUG
    std::cout << "BBSListViewBase::remove_allitems\n";
#endif

    m_treestore->clear();
    m_set_board.clear();
    m_set_thread.clear();
    m_set_image.clear();
}


//
// ツリーの編集ウィンドウを開く
//
void BBSListViewBase::edit_tree()
{
    if( m_editlistwin ) m_editlistwin->present();

    else{

        m_editlistwin = std::make_unique<EditListWin>( get_url(), get_treestore() );
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

    m_editlistwin.reset();
}


//
// ツールチップのセット
//
bool BBSListViewBase::slot_query_tooltip( int x, int y, bool keyboard_tooltip,
                                          const Glib::RefPtr<Gtk::Tooltip>& tooltip )
{
    if( keyboard_tooltip ) return false;

    int bin_x, bin_y;
    m_treeview.convert_widget_to_bin_window_coords( x, y, bin_x, bin_y );
#ifdef _DEBUG
    std::cout << "BBSListViewBase::slot_query_tooltip"
        << " x(" << x << ") y(" << y << ") bin_x(" << bin_x << ") bin_y(" << bin_y << ")" << std::endl;
#endif

    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* column;
    int cell_x, cell_y;
    if( m_treeview.get_path_at_pos( bin_x, bin_y, path, column, cell_x, cell_y ) ) {
        m_treeview.set_tooltip_row( tooltip, path );

        const Gtk::TreeModel::Row row = m_treeview.get_row( path );
        const Glib::ustring& subject = row[ m_columns.m_name ];
        const Glib::ustring& url = row[ m_columns.m_url ];
        const int type = row[ m_columns.m_type ];

        m_treeview.reset_pre_popupurl( url );

        // 画像ポップアップはslot_motion_notifyでセットする
        if( type == TYPE_IMAGE ) return false;

        Gdk::Rectangle cell_area;
        m_treeview.get_cell_area( path, *column, cell_area );
        const int capacity_width = m_treeview.get_width() - cell_area.get_x();

        const auto layout = m_treeview.create_pango_layout( subject );
        int pixel_width, ph;
        layout->get_pixel_size( pixel_width, ph );

        constexpr int icon_size{ 16 };
#ifdef _DEBUG
        std::cout << "BBSListViewBase::slot_query_tooltip" << " pixel_width(" << pixel_width
            << ") + icon_size(" << icon_size << ")" << " > capacity_width(" << capacity_width << ")" << std::endl;
#endif
        // 板一覧の幅よりセルの内容が長いならツールチップを表示する
        if( pixel_width + icon_size > capacity_width ) {
            m_treeview.hide_popup();
            tooltip->set_text( subject );
            return true;
        }
    }
    m_treeview.hide_popup();
    return false;
}



/** @brief 板一覧などサイドバーのビューをXMLに保存する
 *
 * @details remove_dirs != empty() の時は指定された名前のディレクトリを除外して保存する。
 * また、data属性の値が "nosave" のディレクトリも取り除く。
 * @param[in] file        XMLを保存するファイルパス
 * @param[in] root        DOMのルート要素名
 * @param[in] remove_dirs 除外するディレクトリ名のリスト
 */
void BBSListViewBase::save_xml_impl( const std::string& file, const std::string& root,
                                     JDLIB::span<const std::string_view> remove_dirs )
{
    if( file.empty() ) return;
    if( root.empty() ) return;
    if( ! get_ready_tree() ) return;

#ifdef _DEBUG
    std::cout << "BBSListViewBase::save_xml file = " << file << " root = " << root
              << " remove_dirs size = " << remove_dirs.size() << std::endl;
#endif

    tree2xml( root );

    XML::Dom* domroot = m_document.get_root_element( root );

    // 指定したディレクトリを取り除く
    if( ! remove_dirs.empty() ) {

        for( std::string_view remove_dir : remove_dirs ) {
            if( remove_dir.empty() ) continue;

            const auto is_remove_dir = [&remove_dir]( const XML::Dom* child ) {
                return child->nodeName() == "subdir" && child->getAttribute( "name" ) == remove_dir;
            };
            auto it = std::find_if( domroot->begin(), domroot->end(), is_remove_dir );
            if( it != domroot->end() ) {
                domroot->removeChild( *it );
            }
        }
    }
    // data属性の値が "nosave" のディレクトリを取り除く
    if( domroot && domroot->hasChildNodes() ) {
        const auto is_remove_dir = []( const XML::Dom* child ) {
            return child->nodeName() == "subdir" && child->getAttribute( "data" ) == "nosave";
        };
        domroot->remove_if( is_remove_dir );
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


/** @brief TreeView の項目のアイコンを再読み込みする
 *
 * @note Gio::ThemedIcon のアイコンで表示する TreeView の項目は「外部BBSMENU」に
 * 限られているため SKELETON::EditTreeViewIterator ですべての row を巡回すると無駄が多い。
 * 効率化のため TreeModel の巡回範囲を「外部BBSMENU」直下の項目に決め打ちしている。
 */
void BBSListViewBase::reload_list_icon()
{
    auto row = m_treeview.get_row( Gtk::TreePath( "0:0" ) );
    for( ; row; ++row ) {
        const int type = row[ m_columns.m_type ];
        row[ m_columns.m_image ] = XML::get_icon( type );
    }
}
