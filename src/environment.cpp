// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "environment.h"

#include "cache.h"
#include "config/globalconf.h"
#include "jdversion.h"
#include "jdlib/miscutil.h"

#include <gtkmm.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#ifdef GDK_WINDOWING_BROADWAY
#include <gdk/gdkbroadway.h>
#endif

#if __has_include(<sys/utsname.h>)
#define HAVE_SYS_UTSNAME_H
#include <sys/utsname.h> // uname()
#endif

#if defined(USE_GNUTLS)
#  include <gnutls/gnutls.h>
#elif defined(USE_OPENSSL)
#  include <openssl/ssl.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>


std::string ENVIRONMENT::get_progname() { return "JDim"; }
std::string ENVIRONMENT::get_jdcomments(){ return JDCOMMENT; }
std::string ENVIRONMENT::get_jdcopyright(){ return JDCOPYRIGHT; }
std::string ENVIRONMENT::get_jdbbs(){ return JDBBS; }
std::string ENVIRONMENT::get_jd2chlog(){ return JD2CHLOG; }
std::string ENVIRONMENT::get_jdhelp(){ return JDHELP; }
std::string ENVIRONMENT::get_jdhelpcmd(){ return JDHELPCMD; }
std::string ENVIRONMENT::get_jdhelpreplstr() { return JDHELPREPLSTR; }
std::string ENVIRONMENT::get_jdhelpimghash() { return JDHELPIMGHASH; }
std::string ENVIRONMENT::get_jdlicense(){ return JDLICENSE; }


static ENVIRONMENT::DesktopType window_manager = ENVIRONMENT::DesktopType::unknown;


//
// CONFIGURE_ARGSを返す
//
// mode: 整形モード(デフォルト = CONFIGURE_OMITTED 省略したものを一行で)
//
std::string ENVIRONMENT::get_configure_args( const int mode )
{
    std::string configure_args;

#ifdef CONFIGURE_ARGS

    const std::string args = CONFIGURE_ARGS;

    // FULLはそのまま返す
    if( mode == CONFIGURE_FULL ) return args;

    size_t search_pos = 0, found_pos = 0;
    const size_t end_quote_pos = args.rfind( '\'' );

    // 複数の項目
    bool multi = false;

    // 省略形として"--with-"や"--enable-"などを取り出す
    while( ( found_pos = args.find( "'--", search_pos ) ) != std::string::npos )
    {
        const size_t quote_pos = args.find( '\'', found_pos + 1 );

        if( args.compare( found_pos + 3, 4, "with" ) != 0
            && args.compare( found_pos + 3, 6, "enable" ) != 0
            && args.compare( found_pos + 3, 7, "disable" ) != 0 )
        {
            search_pos = quote_pos + 1;
        }
        else if( quote_pos != std::string::npos && quote_pos != end_quote_pos )
        {
            // 項目が複数の場合は改行かスペースを付与
            if( multi )
            {
                if( mode == CONFIGURE_OMITTED_MULTILINE ) configure_args.append( "\n" );
                else configure_args.append( " " );
            }
            multi = true;

            configure_args.append( args.substr( found_pos, ( quote_pos - found_pos ) + 1 ) );
            search_pos = quote_pos + 1;
        }
        else
        {
            configure_args.append( args.substr( found_pos ) );
            break;
        }
    }

#endif

    return configure_args;
}


//
// GITリビジョンとして表示する文字列を返す
// リビジョンが得られなかった場合（tarballのソース等）は、fallbackの日付を返す
// git_dirtyは、まだcommitされてない変更があるかどうか
//
static std::string get_git_revision (const char *git_date, const char *git_hash, const int git_dirty, const char *fallback_date)
{
    bool date_valid = false;
    if( git_date )
    {
        const std::size_t length = std::strlen( git_date );
        if( length >= 8 && git_date[0] >= '1' )
        {
            // isdigit() requires a value within the range of unsigned char.
            date_valid = std::all_of( git_date, git_date + length,
                                      []( unsigned char c ) { return std::isdigit( c ); } );
        }
    }

    bool hash_valid = false;
    if( git_hash )
    {
        // ハッシュ省略表記はgit 2.11から長さを自動で調整するようになった
        // ビルドメタデータが表記揺れするのは紛らわしいので固定長にする
        constexpr const size_t fixed_hash_length = 10;
        if( std::strlen( git_hash ) == fixed_hash_length )
        {
            // isxdigit() requires a value within the range of unsigned char.
            hash_valid = std::all_of( git_hash, git_hash + fixed_hash_length,
                                      []( unsigned char c ) { return std::isxdigit( c ); } );
        }
    }

    std::string git_revision;

    if( date_valid && hash_valid )
    {
        git_revision.append( git_date );
        git_revision.append( "(git:" );
        git_revision.append( git_hash );
        if( git_dirty )
        {
            git_revision.append( ":M" );
        }
        git_revision.push_back( ')' );
    }
    else {
        git_revision.append( fallback_date );
    }

    return git_revision;
}


//
// JDのバージョンを取得
//
std::string ENVIRONMENT::get_jdversion()
{
    std::stringstream jd_version;

    jd_version << MAJORVERSION << "."
                << MINORVERSION << "."
                << MICROVERSION << "-"
                << JDTAG << get_git_revision(GIT_DATE, GIT_HASH, GIT_DIRTY, JDDATE_FALLBACK);

    return jd_version.str();
}


//
// ファイル等からディストリ名を取得
//
std::string ENVIRONMENT::get_distname()
{
#ifdef _DEBUG
    std::cout << "SESSION::get_dist_name\n";
#endif

    std::string tmp;
    std::string text_data;

    // 各ディストリビューション共通の形式として定められた
    // http://www.freedesktop.org/software/systemd/man/os-release.html
    // 旧形式のコードは頃合いを見て削除する予定
    if( CACHE::load_rawdata( "/etc/os-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        for( auto it = lines.rbegin(); it != lines.rend(); ++it )
        {
            std::string name, value;

            if( const auto sep = it->find( '=' ); sep != std::string::npos )
            {
                name = MISC::ascii_trim( it->substr( 0, sep ) );
                value = MISC::ascii_trim( it->substr( sep + 1 ) );
            }

            if( name == "PRETTY_NAME" && ! value.empty() )
            {
                tmp = MISC::cut_str( value, "\"", "\"" );
                break;
            }
        }
    }
    // LSB系 ( Ubuntu ..etc )
    else if( CACHE::load_rawdata( "/etc/lsb-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        for( auto it = lines.rbegin(); it != lines.rend(); ++it )
        {
            std::string lsb_name, lsb_data;

            if( const auto sep = it->find( '=' ); sep != std::string::npos )
            {
                lsb_name = MISC::ascii_trim( it->substr( 0, sep ) );
                lsb_data = MISC::ascii_trim( it->substr( sep + 1 ) );
            }

            // 「DISTRIB_DESCRIPTION="Ubuntu 7.10"」などから「Ubuntu 7.10」を取得
            if( lsb_name == "DISTRIB_DESCRIPTION" && ! lsb_data.empty() )
            {
                tmp = MISC::cut_str( lsb_data, "\"", "\"" );
                break;
            }
        }
    }
    // KNOPPIX (LSB？)
    else if( CACHE::load_rawdata( "/etc/knoppix-version", text_data ) )
    {
        tmp = "KNOPPIX ";
        tmp.append( text_data );
    }
    // SUSE
    else if( CACHE::load_rawdata( "/etc/SuSE-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        tmp = lines.front(); // 1行目のみ
    }
    // Debian
    else if( CACHE::load_rawdata( "/etc/debian_version", text_data ) )
    {
        tmp = "Debian GNU/Linux ";
        tmp.append( text_data );
    }
    // Solaris系
    else if( CACHE::load_rawdata( "/etc/release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        auto it = std::find_if( lines.begin(), lines.end(),
                                // 名前が含まれている行を取得
                                []( const std::string& line )
                                { return line.find( "BeleniX" ) != std::string::npos
                                      || line.find( "Nexenta" ) != std::string::npos
                                      || line.find( "SchilliX" ) != std::string::npos
                                      || line.find( "Solaris" ) != std::string::npos; } );
        if( it != lines.end() ) {
            tmp = std::move( *it );
        }
    }
    // ファイルの中身がそのままディストリ名として扱える物
    else
    {
        // ディストリ名が書かれているファイル
        std::string dist_files[] =
        {
            "/etc/fedora-release",
            "/etc/gentoo-release",
            "/etc/lfs-release",
            "/etc/mandriva-release",
            "/etc/momonga-release",
            "/usr/lib/setup/plamo-version",
            "/etc/puppyversion",
            "/etc/redhat-release", // Redhat, CentOS, WhiteBox, PCLinuxOS
            "/etc/sabayon-release",
            "/etc/slackware-version",
            "/etc/turbolinux-release",
            "/etc/vine-release",
            "/etc/zenwalk-version"
        };

        unsigned int i;
        for( i = 0; i < sizeof( dist_files ) / sizeof( std::string ); ++i )
        {
            if( CACHE::load_rawdata( dist_files[i], text_data ) )
            {
                tmp = text_data;
                break;
            }
        }
    }

    // 文字列両端のスペースなどを削除する
    std::string dist_name = MISC::ascii_trim( tmp );

    // 取得した文字が異常に長い場合は空にする
    if( dist_name.length() > 50 ) dist_name.clear();

#ifdef HAVE_SYS_UTSNAME_H

    char *sysname = nullptr, *release = nullptr, *machine = nullptr;

    // システムコール uname() 準拠：SVr4, POSIX.1-2001.
    struct utsname uts;
    if( uname( &uts ) == 0 )
    {
        sysname = uts.sysname;
        release = uts.release;
        machine = uts.machine;
    }

    // FreeBSD等やディストリ名が取得できなかった場合は"$ uname -rs"と同じ様式
    if( dist_name.empty() && sysname && release )
    {
        dist_name.append( sysname );
        dist_name.push_back( ' ' );
        dist_name.append( release );
    }

    // アーキテクチャがx86でない場合
    if( machine &&
        ( strlen( machine ) != 4
          || ! ( machine[0] == 'i'
               && machine[1] >= '3' && machine[1] <= '6'
               && machine[2] == '8' && machine[3] == '6' ) ) )
    {
        const std::string arch = "(" + std::string( machine ) + ")";

        if ( dist_name.find( arch ) == std::string::npos ) dist_name.append( " " + arch );
    }

#endif

    return dist_name;
}


static constexpr const char* tbl_desktop[] = {
    "GNOME", "XFCE", "KDE", "LXDE", "UNITY", "CINNAMON", "MATE",
    "BUDGIE", "PANTHEON", "ENLIGHTENMENT", "LXQT", "(unknown)" };

//
// WM 判定
// TODO: 環境変数で判定できない場合の判定方法を考える
//
ENVIRONMENT::DesktopType ENVIRONMENT::get_wm()
{
    if( window_manager != DesktopType::unknown ) return window_manager;

    constexpr const char* envvar[] = { "DESKTOP_SESSION", "XDG_CURRENT_DESKTOP" };

    for( const char* v : envvar ) {

        const std::string str_wm = MISC::toupper_str( MISC::getenv_limited( v, 20 ) );

        if( !str_wm.empty() ){
            constexpr auto table_size = static_cast< std::size_t >( DesktopType::unknown );
            for( std::size_t i = 0; i < table_size; ++i ){
                if( str_wm.find( tbl_desktop[ i ] ) != std::string::npos ){
                    window_manager = static_cast< DesktopType >( i );
                }
            }
        }
    }

    if( window_manager == DesktopType::unknown )
    {
        if( ! MISC::getenv_limited( "GNOME_DESKTOP_SESSION_ID" ).empty() )
        {
            window_manager = DesktopType::gnome;
        }
        else
        {
            const std::string str_wm = MISC::getenv_limited( "KDE_FULL_SESSION", 4 );
            if( str_wm == "true" ) window_manager = DesktopType::kde;
        }
    }

    return window_manager;
}


//
// WM名を文字列で返す
//
std::string ENVIRONMENT::get_wm_str()
{
    return tbl_desktop[ static_cast< int >( get_wm() ) ];
}


/** @brief ディスプレイサーバーの種類を文字列で返す
 *
 * @details ディスプレイサーバーが不明のときは空文字列を返す。
 * @return 区切りの空白と () で囲われたディスプレイサーバーの種類の文字列
 */
const char* ENVIRONMENT::get_display_str()
{
    constexpr const char* tbl_display[] = { " (Wayland)", " (X11)", " (Broadway)", "" };

    const auto display_type = ENVIRONMENT::get_display_type();
    return tbl_display[ static_cast<unsigned int>( display_type ) ];
}


/** @brief ディスプレイサーバーの種類を判定してenum型で返す
 *
 * @details ディスプレイサーバーが不明のときは DisplayType::unknown を返す。
 * @return ディスプレイサーバーの種類を表すenum型
 */
ENVIRONMENT::DisplayType ENVIRONMENT::get_display_type()
{
    GdkDisplay* display = gdk_display_get_default();

#ifdef GDK_WINDOWING_WAYLAND
    if( GDK_IS_WAYLAND_DISPLAY( display ) ) return DisplayType::wayland;
#endif

#ifdef GDK_WINDOWING_X11
    if( GDK_IS_X11_DISPLAY( display ) ) return DisplayType::x11;
#endif

#ifdef GDK_WINDOWING_BROADWAY
    if( GDK_IS_BROADWAY_DISPLAY( display ) ) return DisplayType::broadway;
#endif

    return DisplayType::unknown;
}


//
// gtkmmのバージョンを取得
//
std::string ENVIRONMENT::get_gtkmm_version()
{
    std::stringstream gtkmm_ver;
    gtkmm_ver << GTKMM_MAJOR_VERSION << "."
               << GTKMM_MINOR_VERSION << "."
               << GTKMM_MICRO_VERSION;

    return gtkmm_ver.str();
}


//
// glibmmのバージョンを取得
//
std::string ENVIRONMENT::get_glibmm_version()
{
    std::stringstream glibmm_ver;
    glibmm_ver << GLIBMM_MAJOR_VERSION << "."
                << GLIBMM_MINOR_VERSION << "."
                << GLIBMM_MICRO_VERSION;

    return glibmm_ver.str();
}


//
// TLSライブラリのバージョンを取得
//
std::string ENVIRONMENT::get_tlslib_version()
{
    std::string version;

#if defined(USE_GNUTLS)
    version = "GnuTLS ";
    version += gnutls_check_version(nullptr);
#elif defined(USE_OPENSSL)
    version = SSLeay_version(SSLEAY_VERSION);
#endif

    return version;
}



//
// 動作環境を取得
//
std::string ENVIRONMENT::get_jdinfo()
{
    std::stringstream jd_info;

    const std::string progname = get_progname();

    // バージョンを取得(jdversion.h)
    const std::string version = get_jdversion();

    // ディストリビューション名を取得
    const std::string distribution = get_distname();

    // デスクトップ環境を取得( 環境変数から判別可能の場合 )
    std::string desktop = get_wm_str();

    // ディスプレイサーバーの種類を取得
    const char* display = get_display_str();

    // その他
    std::string other;

    // $LANG が ja_JP.UTF-8 でない場合は"その他"に追加する。
    const std::string lang = MISC::getenv_limited( "LANG", 11 );
    if( lang.empty() ) other.append( "LANG 未定義" );
    else if( lang != "ja_JP.utf8" && lang != "ja_JP.UTF-8" ) other.append( "LANG = " + lang );

    jd_info <<
    "[バージョン] " << progname << " " << version << "\n" <<
    "[ディストリ ] " << distribution << "\n" <<
    "[パッケージ] " << JDIM_PACKAGER << "\n" <<
    "[ DE／WM ] " << desktop << display << "\n" <<
    "[　gtkmm 　] " << get_gtkmm_version() << "\n" <<
    "[　glibmm 　] " << get_glibmm_version() << "\n" <<
    "[　TLS lib　] " << get_tlslib_version() << "\n" <<
#ifdef CONFIGURE_ARGS
    "[オプション ] " << get_configure_args( CONFIGURE_OMITTED_MULTILINE ) << "\n" <<
#endif
    "[ そ の 他 ] " << other << "\n";

    return jd_info.str();
}


// Client-Side Decorationを使うか
static bool should_use_header_bar()
{
    const int flag = CONFIG::get_use_header_bar();
    // 0: 使わない 1: 使う
    if( flag == 0 || flag == 1 ) return static_cast<bool>( flag );

    // 2: デスクトップに合わせる のときはデスクトップ環境を調べる
    return ENVIRONMENT::get_wm() == ENVIRONMENT::DesktopType::gnome;
}


// ダイアログでClient-Side Decorationを使うか
// JDimの設定とGtkSettingsから使うか判断する
bool ENVIRONMENT::get_dialog_use_header_bar()
{
    const bool dialog_use_header = Gtk::Settings::get_default()->property_gtk_dialogs_use_header();

    return dialog_use_header && should_use_header_bar();
}
