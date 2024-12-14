// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "iconmanager.h"
#include "iconfiles.h"

#include "cache.h"

#include "config/globalconf.h"
#include "jdlib/miscmsg.h"

#include "jd16.h"
#include "jd32.h"
#include "jd48.h"
#include "jd96.h"

#include "bkmark_update.h"
#include "bkmark.h"
#include "bkmark_broken_subject.h"
#include "bkmark_thread.h"

#include "update.h"
#include "newthread.h"
#include "newthread_hour.h"
#include "broken_subject.h"
#include "check.h"
#include "down.h"
#include "write.h"
#include "post.h"
#include "post_refer.h"
#include "loading.h"
#include "loading_stop.h"

#include "dir.h"
#include "favorite.h"
#include "hist.h"
#include "hist_board.h"
#include "hist_close.h"
#include "hist_closeboard.h"
#include "hist_closeimg.h"

#include "board.h"
#include "board_update.h"
#include "board_updated.h"
#include "thread.h"
#include "thread_update.h"
#include "thread_updated.h"
#include "thread_old.h"
#include "image.h"
#include "link.h"
#include "info.h"

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


Glib::RefPtr< Gdk::Pixbuf > ICON::get_icon( int id )
{
    return get_icon_manager()->get_icon( id );
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

    auto data_dirs = Glib::get_system_data_dirs();
    for( auto& data_dir : data_dirs ) {
        data_dir += "/icons";
        fill_icon( data_dir );
    }

    // ソートして重複した名前を取り除く
    std::sort( names.begin(), names.end() );
    names.erase( std::unique( names.begin(), names.end() ), names.end() );

    return names;
}


namespace {

/// アイコンを読み込むヘルパークラス
struct NamedIconLoader
{
    Glib::RefPtr<Gtk::IconTheme> m_icon_theme = Gtk::IconTheme::get_default();
    Glib::RefPtr<Gdk::Pixbuf> m_missing_icon;

    explicit NamedIconLoader( int size );
    Glib::RefPtr<Gdk::Pixbuf> load_icon( const Glib::ustring& icon_name, int size ) const;
    Glib::RefPtr<Gdk::Pixbuf> choose_icon( const std::vector<Glib::ustring>& icon_names, int size ) const;
};


/** @brief コンストラクタ
 *
 * @param[in] size 見つからないときかわりに返すアイコンのサイズ
 */
NamedIconLoader::NamedIconLoader( int size )
{
    // GTKテーマのforeground colorを取得して矩形枠の色に使う
    Gdk::RGBA rgba;

    Gtk::WidgetPath path;
    path.path_append_type( Gtk::Window::get_type() );
    path.path_append_type( Gtk::Label::get_type() );
    auto context = Gtk::StyleContext::create();
    context->set_path( path );
    if( ! context->lookup_color( "theme_fg_color", rgba ) ) {
        rgba.set_red( 0 );
        rgba.set_green( 0 );
        rgba.set_blue( 0 );
        rgba.set_alpha( 1 );
    }

    // アイコンの読み込みに失敗したときかわりに表示するイメージ (点線の矩形で薄赤色)
    auto surface = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, size, size );
    auto cr = Cairo::Context::create( surface );
    cr->rectangle( 0, 0 , size, size );
    cr->set_source_rgba( 1, 0, 0, 0.0625 );
    cr->fill_preserve();
    cr->set_source_rgba( rgba.get_red(), rgba.get_green(), rgba.get_blue(), rgba.get_alpha() );
    cr->set_line_width( 1 );
    const std::vector<double> dashes = { 1 };
    cr->set_dash( dashes, 0 );
    cr->stroke();
    m_missing_icon = Gdk::Pixbuf::create( surface, 0, 0, size, size );
}


/** @brief アイコンを読み込んで Gdk::Pixbuf を返す
 *
 * アイコンが見つからないときは同名のsymbolic iconを探す。
 * @param[in] icon_name アイコン名
 * @param[in] size アイコンのサイズ
 * @return 読み込んだアイコン。見つからないときはかわりに表示するイメージを返す。
 */
Glib::RefPtr<Gdk::Pixbuf> NamedIconLoader::load_icon( const Glib::ustring& icon_name, int size ) const
{
    Gtk::IconInfo icon_info = m_icon_theme->lookup_icon( icon_name, size );
    if( icon_info ) return icon_info.load_icon();

    icon_info = m_icon_theme->lookup_icon( icon_name, size, Gtk::ICON_LOOKUP_FORCE_SYMBOLIC );
    if( icon_info ) return icon_info.load_icon();

    MISC::MSG( "Icon \"" + icon_name + "\" is not found... "
               "Please make sure that icon theme is installed on your desktop." );
    return m_missing_icon;
}


/** @brief アイコンを読み込んで Gdk::Pixbuf を返す
 *
 * アイコンが見つからないときは同名のsymbolic iconを探す。
 * @param[in] icon_names アイコン名のリスト (1つ以上必須)
 * @param[in] size アイコンのサイズ
 * @return リストから最初に見つけたアイコン。見つからないときはかわりに表示するイメージを返す。
 */
Glib::RefPtr<Gdk::Pixbuf> NamedIconLoader::choose_icon( const std::vector<Glib::ustring>& icon_names, int size ) const
{
    assert( ! icon_names.empty() );
    Gtk::IconInfo icon_info = m_icon_theme->choose_icon( icon_names, size );
    if( icon_info ) return icon_info.load_icon();

    icon_info = m_icon_theme->choose_icon( icon_names, size, Gtk::ICON_LOOKUP_FORCE_SYMBOLIC );
    if( icon_info ) return icon_info.load_icon();

    MISC::MSG( "Icon \"" + icon_names[0] + "\" is not found... "
               "Please make sure that icon theme is installed on your desktop." );
    return m_missing_icon;
}

} // namespace


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


Glib::RefPtr< Gdk::Pixbuf > ICON_Manager::get_icon( const int id )
{
    return m_list_icons[ id ];
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
void ICON_Manager::load_builtin_icons( std::vector<Glib::RefPtr<Gdk::Pixbuf>>& list_icons )
{
    // JDim ロゴ
    list_icons[ ICON::JD16 ] =  Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16 ), icon_jd16 );
    list_icons[ ICON::JD32 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32 ), icon_jd32 );
    list_icons[ ICON::JD48 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48 ), icon_jd48 );
    list_icons[ ICON::JD96 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd96 ), icon_jd96 );

    // サイドバーで使用するアイコン
    list_icons[ ICON::DIR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_dir ), icon_dir );
    list_icons[ ICON::IMAGE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_image ), icon_image );
    list_icons[ ICON::LINK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_link ), icon_link );

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BOARD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board ), icon_board );
    list_icons[ ICON::BOARD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_update ), icon_board_update );
    list_icons[ ICON::THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );
    list_icons[ ICON::THREAD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_update ), icon_thread_update );
    list_icons[ ICON::THREAD_OLD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_old ), icon_thread_old );

    // タブで使用するアイコン
    list_icons[ ICON::BOARD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_updated ), icon_board_updated );
    list_icons[ ICON::THREAD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_updated ), icon_thread_updated );
    list_icons[ ICON::LOADING ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading ), icon_loading );
    list_icons[ ICON::LOADING_STOP ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading_stop ), icon_loading_stop );

    // スレ一覧で使用するアイコン
    list_icons[ ICON::BKMARK_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_update ), icon_bkmark_update );
    list_icons[ ICON::BKMARK_BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_broken_subject ), icon_bkmark_broken_subject );
    list_icons[ ICON::BKMARK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark ), icon_bkmark );
    list_icons[ ICON::UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_update ), icon_update );
    list_icons[ ICON::NEWTHREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread ), icon_newthread );
    list_icons[ ICON::NEWTHREAD_HOUR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread_hour ), icon_newthread_hour );
    list_icons[ ICON::BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_broken_subject ), icon_broken_subject );
    list_icons[ ICON::CHECK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_check ), icon_check );
    list_icons[ ICON::OLD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_down ), icon_down );
    list_icons[ ICON::INFO ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_info ), icon_info );

    // スレビューで使用するアイコン
    list_icons[ ICON::BKMARK_THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_thread ), icon_bkmark_thread );
    list_icons[ ICON::POST ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post ), icon_post );
    list_icons[ ICON::POST_REFER ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post_refer ), icon_post_refer );

    // その他
    list_icons[ ICON::DOWN ] = list_icons[ ICON::OLD ];

    list_icons[ ICON::TRANSPARENT ] = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    list_icons[ ICON::TRANSPARENT ]->fill( 0 );

    // ツールバーのアイコン
    // 共通
    list_icons[ ICON::WRITE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_write ), icon_write );

    // メイン
    list_icons[ ICON::BBSLISTVIEW ] = list_icons[ ICON::DIR ];
    list_icons[ ICON::FAVORITEVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_favorite ), icon_favorite );
    list_icons[ ICON::HISTVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist ), icon_hist );
    list_icons[ ICON::HIST_BOARDVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_board ), icon_hist_board );
    list_icons[ ICON::HIST_CLOSEVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_close ), icon_hist_close );
    list_icons[ ICON::HIST_CLOSEBOARDVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_closeboard ), icon_hist_closeboard );
    list_icons[ ICON::HIST_CLOSEIMGVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_closeimg ), icon_hist_closeimg );
    list_icons[ ICON::BOARDVIEW ] = list_icons[ ICON::BOARD ];
    list_icons[ ICON::ARTICLEVIEW ] = list_icons[ ICON::THREAD ];
    list_icons[ ICON::IMAGEVIEW ] = list_icons[ ICON::IMAGE ];

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_OPEN_ROOT ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );

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
void ICON_Manager::load_themed_color_icons( std::vector<Glib::RefPtr<Gdk::Pixbuf>>& list_icons )
{

    constexpr int size_menu = 16; // Gtk::ICON_SIZE_MENU
    const NamedIconLoader icon_loader{ size_menu };
    std::vector<Glib::ustring> icon_names;

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BBSMENU ] = icon_loader.load_icon( "emblem-documents", size_menu );

    // 共通
    list_icons[ ICON::SEARCH_PREV ] = icon_loader.load_icon( "go-up", size_menu );
    list_icons[ ICON::SEARCH_NEXT ] = icon_loader.load_icon( "go-down", size_menu );
    list_icons[ ICON::STOPLOADING ] = icon_loader.load_icon( "process-stop", size_menu );
    list_icons[ ICON::RELOAD ] = icon_loader.load_icon( "view-refresh", size_menu );
    icon_names.assign( { "bookmark-new", "edit-copy" } );
    list_icons[ ICON::APPENDFAVORITE ] = icon_loader.choose_icon( icon_names, size_menu );
    list_icons[ ICON::DELETE ] = icon_loader.load_icon( "edit-delete", size_menu );
    list_icons[ ICON::QUIT ] = icon_loader.load_icon( "window-close", size_menu );
    list_icons[ ICON::BACK ] = icon_loader.load_icon( "go-previous", size_menu );
    list_icons[ ICON::FORWARD ] = icon_loader.load_icon( "go-next", size_menu );
    icon_names.assign( { "changes-prevent", "window-close" } );
    list_icons[ ICON::LOCK ] = icon_loader.choose_icon( icon_names, size_menu );

    // メイン
    list_icons[ ICON::GO ] = icon_loader.load_icon( "go-jump", size_menu );
    list_icons[ ICON::UNDO ] = icon_loader.load_icon( "edit-undo", size_menu );
    list_icons[ ICON::REDO ] = icon_loader.load_icon( "edit-redo", size_menu );

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_ROOT ] = icon_loader.load_icon( "view-refresh", size_menu );

    // スレビュー
    list_icons[ ICON::SEARCH ] = icon_loader.load_icon( "edit-find", size_menu );
    list_icons[ ICON::LIVE ] = icon_loader.load_icon( "media-playback-start", size_menu );

    // 検索バー
    list_icons[ ICON::CLOSE_SEARCH ] = icon_loader.load_icon( "edit-undo", size_menu );
    list_icons[ ICON::CLEAR_SEARCH ] = icon_loader.load_icon( "edit-clear", size_menu );
    list_icons[ ICON::SEARCH_AND ] = icon_loader.load_icon( "edit-cut", size_menu );
    list_icons[ ICON::SEARCH_OR ] = icon_loader.load_icon( "list-add", size_menu );

    // 書き込みビュー
    list_icons[ ICON::INSERTTEXT ] = icon_loader.load_icon( "document-open", size_menu );
}


/** @brief アイコンテーマからツールバーのシンボリックアイコンを読み込む
 *
 * @param[in,out] list_icons アイコンをキャッシュする配列
 * @see ICON_Manager::load_themed_color_icons()
 */
void ICON_Manager::load_themed_symbolic_icons( std::vector<Glib::RefPtr<Gdk::Pixbuf>>& list_icons )
{
    constexpr int size_menu = 16; // Gtk::ICON_SIZE_MENU
    const NamedIconLoader icon_loader{ size_menu };
    std::vector<Glib::ustring> icon_names;

    // サイドバーやタブで使用するアイコン
    list_icons[ ICON::BBSMENU ] = icon_loader.load_icon( "emblem-documents-symbolic", size_menu );

    // 共通
    list_icons[ ICON::SEARCH_PREV ] = icon_loader.load_icon( "go-up-symbolic", size_menu );
    list_icons[ ICON::SEARCH_NEXT ] = icon_loader.load_icon( "go-down-symbolic", size_menu );
    list_icons[ ICON::STOPLOADING ] = icon_loader.load_icon( "process-stop-symbolic", size_menu );
    list_icons[ ICON::RELOAD ] = icon_loader.load_icon( "view-refresh-symbolic", size_menu );
    icon_names.assign( { "bookmark-new-symbolic", "edit-copy-symbolic" } );
    list_icons[ ICON::APPENDFAVORITE ] = icon_loader.choose_icon( icon_names, size_menu );
    list_icons[ ICON::DELETE ] = icon_loader.load_icon( "edit-delete-symbolic", size_menu );
    list_icons[ ICON::QUIT ] = icon_loader.load_icon( "window-close-symbolic", size_menu );
    list_icons[ ICON::BACK ] = icon_loader.load_icon( "go-previous-symbolic", size_menu );
    list_icons[ ICON::FORWARD ] = icon_loader.load_icon( "go-next-symbolic", size_menu );
    icon_names.assign( { "changes-prevent-symbolic", "window-close-symbolic" } );
    list_icons[ ICON::LOCK ] = icon_loader.choose_icon( icon_names, size_menu );

    // メイン
    list_icons[ ICON::GO ] = icon_loader.load_icon( "go-jump-symbolic", size_menu );
    list_icons[ ICON::UNDO ] = icon_loader.load_icon( "edit-undo-symbolic", size_menu );
    list_icons[ ICON::REDO ] = icon_loader.load_icon( "edit-redo-symbolic", size_menu );

    // サイドバー
    list_icons[ ICON::CHECK_UPDATE_ROOT ] = icon_loader.load_icon( "view-refresh-symbolic", size_menu );

    // スレビュー
    list_icons[ ICON::SEARCH ] = icon_loader.load_icon( "edit-find-symbolic", size_menu );
    list_icons[ ICON::LIVE ] = icon_loader.load_icon( "media-playback-start-symbolic", size_menu );

    // 検索バー
    list_icons[ ICON::CLOSE_SEARCH ] = icon_loader.load_icon( "edit-undo-symbolic", size_menu );
    list_icons[ ICON::CLEAR_SEARCH ] = icon_loader.load_icon( "edit-clear-symbolic", size_menu );
    list_icons[ ICON::SEARCH_AND ] = icon_loader.load_icon( "edit-cut-symbolic", size_menu );
    list_icons[ ICON::SEARCH_OR ] = icon_loader.load_icon( "list-add-symbolic", size_menu );

    // 書き込みビュー
    list_icons[ ICON::INSERTTEXT ] = icon_loader.load_icon( "document-open-symbolic", size_menu );
}
