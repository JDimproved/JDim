// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "iconmanager.h"
#include "iconfiles.h"

#include "cache.h"

#include "config/globalconf.h"
#include "jdlib/miscmsg.h"

#include <algorithm>
#include <fstream>


static ICON::ICON_Manager* instance_icon_manager = nullptr;


ICON::ICON_Manager* ICON::get_icon_manager()
{
    if( ! instance_icon_manager ) instance_icon_manager = new ICON::ICON_Manager();
    assert( instance_icon_manager );

    return instance_icon_manager;
}


void ICON::delete_icon_manager()
{
    if( instance_icon_manager ) delete instance_icon_manager;
    instance_icon_manager = nullptr;
}


/** @brief アイコンIDに対応した Gio::Icon を返す
 *
 * @param[in] id JDimで定義したアイコンID
 * @return アイコンテーマ(Gio::ThemedIcon)、
 * または組み込みアイコン(Gdk::Pixbuf)がアップキャストされている。
 * @see ICON::ICON_Manager::get_icon()
 */
Glib::RefPtr<Gio::Icon> ICON::get_icon( int id )
{
    return get_icon_manager()->get_icon( id );
}


/** @brief アイコンIDに対応した Gdk::Pixbuf を返す
 *
 * @details この関数は、組み込みアイコンを Gdk::Pixbuf で取得する。
 * 組み込みアイコンを使う処理のうち Gio::Icon が使用できない、
 * または Gio::Icon に変更する必要性が低い処理で使う。
 *
 * @param[in] id JDimで定義したアイコンID
 * @return 組み込みアイコンなら画像データを返す。そうでないならnullポインターを返す。
 * @see ICON::ICON_Manager::get_pixbuf()
 */
Glib::RefPtr<Gdk::Pixbuf> ICON::get_pixbuf( int id )
{
    return get_icon_manager()->get_pixbuf( id );
}


/**
 * @brief インストールされているGTKテーマの名前をまとめて返す
 *
 * @details GTKテーマを探す場所にあるテーマ名のディレクトリに
 * gtk-3.X/gtk.css (Xは0, 2, 4, ..., 24) のファイルが存在するかチェックします。
 *
 * GTKテーマを探す場所
 * 1. $XDG_DATA_HOME/themes/
 * 2. $HOME/.themes/
 * 3. $XDG_DATA_DIRS にある themes/
 *
 * @return 辞書順でソートされたテーマ名のvector
 */
std::vector<std::string> ICON::get_installed_gtk_theme_names()
{
    constexpr int minor = GTK_MINOR_VERSION + 1;
    std::vector<std::string> names;

    auto fill_gtk = [&]( const auto& themes_dir ) {
        if( ! Glib::file_test( themes_dir, Glib::FILE_TEST_IS_DIR ) ) return;

        for( auto dir_name : Glib::Dir( themes_dir ) ) {
            for( int i = 0; i < minor; i += 2 ) {
                auto file_path = Glib::ustring::compose( "%1/%2/gtk-3.%3/gtk.css", themes_dir, dir_name, i );
                if( Glib::file_test( file_path, Glib::FILE_TEST_IS_REGULAR ) ) {
                    names.push_back( std::move( dir_name ) );
                    break;
                }
            }
        }
    };

    fill_gtk( Glib::get_user_data_dir() + "/themes" );
    fill_gtk( Glib::get_home_dir() + "/.themes" );

    auto data_dirs = Glib::get_system_data_dirs();
    for( auto& data_dir : data_dirs ) {
        data_dir += "/themes";
        fill_gtk( data_dir );
    }

    // ソートして重複した名前を取り除く
    std::sort( names.begin(), names.end() );
    names.erase( std::unique( names.begin(), names.end() ), names.end() );

    return names;
}


/**
 * @brief インストールされているアイコンテーマの名前をまとめて返す
 *
 * @details アイコンテーマを探す場所にあるテーマ名のディレクトリに
 * index.theme のファイルが存在するかチェックし、必須のセクションとキーを確認します。
 *
 * アイコンテーマを探す場所
 * 1. $XDG_DATA_HOME/icons/
 * 2. $HOME/.icons/
 * 3. $XDG_DATA_DIRS にある icons/
 *
 * @return 辞書順でソートされたテーマ名のvector
 */
std::vector<std::string> ICON::get_installed_icon_theme_names()
{
    std::vector<std::string> names;

    auto fill_icon = [&]( const auto& themes_dir ) {
        if( ! Glib::file_test( themes_dir, Glib::FILE_TEST_IS_DIR ) ) return;

        for( auto dir_name : Glib::Dir( themes_dir ) ) {

            std::ifstream index( themes_dir + "/" + dir_name + "/index.theme" );
            if( ! index ) continue;

            std::string line;
            std::getline( index, line );
            if( line.compare( 0, 12, "[Icon Theme]" ) != 0 ) continue;

            // cursor テーマだけインストールされている場合があるので必須のキーがあるか調べる
            while( std::getline( index, line ) ) {
                // Directories キーが空データでないことを確認する
                if( line.size() > 12 && line.compare( 0, 12, "Directories=" ) == 0 ) {
                    names.push_back( std::move( dir_name ) );
                    break;
                }
            }
        }
    };

    fill_icon( Glib::get_user_data_dir() + "/icons" );
    fill_icon( Glib::get_home_dir() + "/.icons" );

    const bool running_on_snap{ ! Glib::getenv( "SNAP_DESKTOP_RUNTIME" ).empty()
                                && Glib::getenv( "GTK_USE_PORTAL" ) == "1" };

    auto data_dirs = Glib::get_system_data_dirs();
    for( auto& data_dir : data_dirs ) {
        // Snapで実行しているときは以下のディレクトリにアクセスすると Permission denied が発生する
        if( running_on_snap && data_dir == "/var/lib/snapd/desktop" ) continue;

        data_dir += "/icons";
        fill_icon( data_dir );
    }

    // ソートして重複した名前を取り除く
    std::sort( names.begin(), names.end() );
    names.erase( std::unique( names.begin(), names.end() ), names.end() );

    return names;
}


///////////////////////////////////////////////

using namespace ICON;


ICON_Manager::ICON_Manager()
{
    m_list_icons.resize( NUM_ICONS );

    load_builtin_icons( m_list_icons );

    reload_themed_icons( CONFIG::get_use_symbolic_icon() );

    load_theme();
}


ICON_Manager::~ICON_Manager()
{
#ifdef _DEBUG
    std::cout << "ICON::~ICON_Manager\n";
#endif
}


/** @brief アイコンIDに対応した Gio::Icon を返す
 *
 * @param[in] id JDimで定義したアイコンID
 * @return アイコンテーマ(Gio::ThemedIcon)、
 * または組み込みアイコン(Gdk::Pixbuf)がアップキャストされている。
 */
Glib::RefPtr<Gio::Icon> ICON_Manager::get_icon( const int id )
{
    return m_list_icons[ id ];
}


/** @brief アイコンIDに対応した Gdk::Pixbuf を返す
 *
 * @details このメンバー関数は、組み込みアイコンを Gdk::Pixbuf で取得する。
 * 組み込みアイコンを使う処理のうち Gio::Icon が使用できない、
 * または Gio::Icon に変更する必要性が低い処理で使う。
 *
 * @note アイコンテーマ(Gio::ThemedIcon)で読み込んだアイコンを指定した場合は、
 * ダウンキャストに失敗してnullポインターを返す。
 * @param[in] id JDimで定義したアイコンID
 * @return 組み込みアイコンなら画像データを返す。そうでないならnullポインターを返す。
 */
Glib::RefPtr<Gdk::Pixbuf> ICON_Manager::get_pixbuf( const int id )
{
    return Glib::RefPtr<Gdk::Pixbuf>::cast_dynamic( m_list_icons[ id ] );
}


/** @brief アイコンテーマからロードしたアイコンを再読み込みをする
 *
 * @param[in] use_symbolic シンボリックアイコンで表示するならtrue
 */
void ICON_Manager::reload_themed_icons( const bool use_symbolic )
{
    if( use_symbolic ) {
        load_themed_symbolic_icons( m_list_icons );
    }
    else {
        load_themed_color_icons( m_list_icons );
    }
}


//
// アイコンテーマ読み込み
//
void ICON_Manager::load_theme()
{
    if( CACHE::file_exists( CACHE::path_theme_icon_root() ) != CACHE::EXIST_DIR ) return;

    const std::list< std::string > files = CACHE::get_filelist( CACHE::path_theme_icon_root() );
    if( ! files.size() ) return;

#ifdef _DEBUG
    std::cout << "ICON::load_theme\n";
#endif

    for( const std::string& filename : files ) {
#ifdef _DEBUG
        std::cout << filename << std::endl;
#endif

        int id = 0;

        // 拡張子を探す
        const std::size_t i = filename.rfind( '.' );

        while( iconfiles[ id ][ 0 ] != '\0' ){

            // 拡張子を除いたファイル名を比較
            if( filename.compare( 0, i, iconfiles[ id ] ) == 0 ) {
#ifdef _DEBUG
                std::cout << "hit : " << iconfiles[ id ] << " id = " << id << std::endl;
#endif
                // ツールバーボタンのアイコンより大きな画像はサイズ調整する
                // 視認性を良くするため組み込みのアイコンよりサイズを一回り大きめにとる
                auto pixbuf = Gdk::Pixbuf::create_from_file( CACHE::path_theme_icon_root() + filename );
                constexpr int size_menu = 24; // Gtk::ICON_SIZE_LARGE_TOOLBAR
                if( pixbuf->get_width() > size_menu || pixbuf->get_height() > size_menu ) {
                    m_list_icons[id] = pixbuf->scale_simple( size_menu, size_menu, Gdk::INTERP_HYPER );
                }
                else {
                    m_list_icons[id] = pixbuf;
                }
                break;
            }

            ++id;
        }
    }
}


/** @brief JDimに組み込まれたアイコンを読み込む
 *
 * @details 組み込みのアイコンはアイコンテーマを変更しても変わらない。
 */
void ICON_Manager::load_builtin_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons )
{
    // JDim ロゴ
    list_icons[ ICON::JD16 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd16.png" );
    list_icons[ ICON::JD32 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd32.png" );
    list_icons[ ICON::JD48 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd48.png" );
    list_icons[ ICON::JD96 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd96.png" );

    // サイドバーで使用するアイコン
    list_icons[ ICON::DIR ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/dir.png" );
    list_icons[ ICON::IMAGE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/image.png" );
    list_icons[ ICON::LINK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/link.png" );

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BOARD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board.png" );
    list_icons[ ICON::BOARD_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board_update.png" );
    list_icons[ ICON::THREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread.png" );
    list_icons[ ICON::THREAD_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_update.png" );
    list_icons[ ICON::THREAD_OLD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_old.png" );

    // タブで使用するアイコン
    list_icons[ ICON::BOARD_UPDATED ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board_updated.png" );
    list_icons[ ICON::THREAD_UPDATED ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_updated.png" );
    list_icons[ ICON::LOADING ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/loading.png" );
    list_icons[ ICON::LOADING_STOP ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/loading_stop.png" );

    // スレ一覧で使用するアイコン
    list_icons[ ICON::BKMARK_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_update.png" );
    list_icons[ ICON::BKMARK_BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_broken_subject.png" );
    list_icons[ ICON::BKMARK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark.png" );
    list_icons[ ICON::UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/update.png" );
    list_icons[ ICON::NEWTHREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/newthread.png" );
    list_icons[ ICON::NEWTHREAD_HOUR ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/newthread_hour.png" );
    list_icons[ ICON::BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/broken_subject.png" );
    list_icons[ ICON::CHECK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/check.png" );
    list_icons[ ICON::OLD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/down.png" );
    list_icons[ ICON::INFO ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/info.png" );

    // スレビューで使用するアイコン
    list_icons[ ICON::BKMARK_THREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_thread.png" );
    list_icons[ ICON::POST ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/post.png" );
    list_icons[ ICON::POST_REFER ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/post_refer.png" );

    // その他
    list_icons[ ICON::DOWN ] = list_icons[ ICON::OLD ];

    auto pixbuf = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    pixbuf->fill( 0 );
    list_icons[ ICON::TRANSPARENT ] = std::move( pixbuf );

    // ツールバーのアイコン
    // 共通
    list_icons[ ICON::WRITE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/write.png" );

    // メイン
    list_icons[ ICON::BBSLISTVIEW ] = list_icons[ ICON::DIR ];
    list_icons[ ICON::FAVORITEVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/favorite.png" );
    list_icons[ ICON::HISTVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist.png" );
    list_icons[ ICON::HIST_BOARDVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_board.png" );
    list_icons[ ICON::HIST_CLOSEVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_close.png" );
    list_icons[ ICON::HIST_CLOSEBOARDVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_closeboard.png" );
    list_icons[ ICON::HIST_CLOSEIMGVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_closeimg.png" );
    list_icons[ ICON::BOARDVIEW ] = list_icons[ ICON::BOARD ];
    list_icons[ ICON::ARTICLEVIEW ] = list_icons[ ICON::THREAD ];
    list_icons[ ICON::IMAGEVIEW ] = list_icons[ ICON::IMAGE ];

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_OPEN_ROOT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread.png" );

    // 書き込みビュー
    list_icons[ ICON::PREVIEW ] = list_icons[ ICON::THREAD ];
}


/** @brief アイコンテーマからツールバーのカラーアイコンを読み込む
 *
 * @details アイコン名は freedesktop.org の規格とGTK3デフォルトテーマの Adwaita を参照する。
 * - https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
 * - https://gitlab.gnome.org/GNOME/adwaita-icon-theme
 * @param[in,out] list_icons アイコンをキャッシュする配列
 */
void ICON_Manager::load_themed_color_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons )
{
    std::vector<Glib::ustring> icon_names;

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BBSMENU ] = Gio::ThemedIcon::create( "emblem-documents" );

    // 共通
    list_icons[ ICON::SEARCH_PREV ] = Gio::ThemedIcon::create( "go-up" );
    list_icons[ ICON::SEARCH_NEXT ] = Gio::ThemedIcon::create( "go-down" );
    list_icons[ ICON::STOPLOADING ] = Gio::ThemedIcon::create( "process-stop" );
    list_icons[ ICON::RELOAD ] = Gio::ThemedIcon::create( "view-refresh" );
    icon_names.assign( { "bookmark-new", "edit-copy" } );
    list_icons[ ICON::APPENDFAVORITE ] = Gio::ThemedIcon::create( icon_names );
    list_icons[ ICON::DELETE ] = Gio::ThemedIcon::create( "edit-delete" );
    list_icons[ ICON::QUIT ] = Gio::ThemedIcon::create( "window-close" );
    list_icons[ ICON::BACK ] = Gio::ThemedIcon::create( "go-previous" );
    list_icons[ ICON::FORWARD ] = Gio::ThemedIcon::create( "go-next" );
    icon_names.assign( { "changes-prevent", "window-close" } );
    list_icons[ ICON::LOCK ] = Gio::ThemedIcon::create( icon_names );

    // メイン
    list_icons[ ICON::GO ] = Gio::ThemedIcon::create( "go-jump" );
    list_icons[ ICON::UNDO ] = Gio::ThemedIcon::create( "edit-undo" );
    list_icons[ ICON::REDO ] = Gio::ThemedIcon::create( "edit-redo" );

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_ROOT ] = Gio::ThemedIcon::create( "view-refresh" );

    // スレビュー
    list_icons[ ICON::SEARCH ] = Gio::ThemedIcon::create( "edit-find" );
    list_icons[ ICON::LIVE ] = Gio::ThemedIcon::create( "media-playback-start" );

    // 検索バー
    list_icons[ ICON::CLOSE_SEARCH ] = Gio::ThemedIcon::create( "edit-undo" );
    list_icons[ ICON::CLEAR_SEARCH ] = Gio::ThemedIcon::create( "edit-clear" );
    list_icons[ ICON::SEARCH_AND ] = Gio::ThemedIcon::create( "edit-cut" );
    list_icons[ ICON::SEARCH_OR ] = Gio::ThemedIcon::create( "list-add" );

    // 書き込みビュー
    list_icons[ ICON::INSERTTEXT ] = Gio::ThemedIcon::create( "document-open" );
}


/** @brief アイコンテーマからツールバーのシンボリックアイコンを読み込む
 *
 * @param[in,out] list_icons アイコンをキャッシュする配列
 * @see ICON_Manager::load_themed_color_icons()
 */
void ICON_Manager::load_themed_symbolic_icons( std::vector<Glib::RefPtr<Gio::Icon>>& list_icons )
{
    std::vector<Glib::ustring> icon_names;

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BBSMENU ] = Gio::ThemedIcon::create( "emblem-documents-symbolic" );

    // 共通
    list_icons[ ICON::SEARCH_PREV ] = Gio::ThemedIcon::create( "go-up-symbolic" );
    list_icons[ ICON::SEARCH_NEXT ] = Gio::ThemedIcon::create( "go-down-symbolic" );
    list_icons[ ICON::STOPLOADING ] = Gio::ThemedIcon::create( "process-stop-symbolic" );
    list_icons[ ICON::RELOAD ] = Gio::ThemedIcon::create( "view-refresh-symbolic" );
    icon_names.assign( { "bookmark-new-symbolic", "edit-copy-symbolic" } );
    list_icons[ ICON::APPENDFAVORITE ] = Gio::ThemedIcon::create( icon_names );
    list_icons[ ICON::DELETE ] = Gio::ThemedIcon::create( "edit-delete-symbolic" );
    list_icons[ ICON::QUIT ] = Gio::ThemedIcon::create( "window-close-symbolic" );
    list_icons[ ICON::BACK ] = Gio::ThemedIcon::create( "go-previous-symbolic" );
    list_icons[ ICON::FORWARD ] = Gio::ThemedIcon::create( "go-next-symbolic" );
    icon_names.assign( { "changes-prevent-symbolic", "window-close-symbolic" } );
    list_icons[ ICON::LOCK ] = Gio::ThemedIcon::create( icon_names );

    // メイン
    list_icons[ ICON::GO ] = Gio::ThemedIcon::create( "go-jump-symbolic" );
    list_icons[ ICON::UNDO ] = Gio::ThemedIcon::create( "edit-undo-symbolic" );
    list_icons[ ICON::REDO ] = Gio::ThemedIcon::create( "edit-redo-symbolic" );

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_ROOT ] = Gio::ThemedIcon::create( "view-refresh-symbolic" );

    // スレビュー
    list_icons[ ICON::SEARCH ] = Gio::ThemedIcon::create( "edit-find-symbolic" );
    list_icons[ ICON::LIVE ] = Gio::ThemedIcon::create( "media-playback-start-symbolic" );

    // 検索バー
    list_icons[ ICON::CLOSE_SEARCH ] = Gio::ThemedIcon::create( "edit-undo-symbolic" );
    list_icons[ ICON::CLEAR_SEARCH ] = Gio::ThemedIcon::create( "edit-clear-symbolic" );
    list_icons[ ICON::SEARCH_AND ] = Gio::ThemedIcon::create( "edit-cut-symbolic" );
    list_icons[ ICON::SEARCH_OR ] = Gio::ThemedIcon::create( "list-add-symbolic" );

    // 書き込みビュー
    list_icons[ ICON::INSERTTEXT ] = Gio::ThemedIcon::create( "document-open-symbolic" );
}
