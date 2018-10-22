// License GPL2

#ifdef _WIN32
// require Windows XP SP2 or Server 2003 SP1 for GetNativeSystemInfo, KEY_WOW64_64KEY
#define WINVER 0x0502
#endif

//#define _DEBUG
#include "jddebug.h"

#include "environment.h"

#include "cache.h"
#include "config/globalconf.h"
#include "jdversion.h"
#include "jdlib/miscutil.h"

#include <cstring>
#include <sstream>

#include "gtkmm.h"

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h> // uname()
#endif
#ifdef _WIN32
#include <windows.h>
#endif

std::string ENVIRONMENT::get_jdcomments(){ return std::string( JDCOMMENT ); }
std::string ENVIRONMENT::get_jdcopyright(){ return std::string( JDCOPYRIGHT ); }
std::string ENVIRONMENT::get_jdbbs(){ return std::string( JDBBS ); }
std::string ENVIRONMENT::get_jd2chlog(){ return std::string( JD2CHLOG ); }
std::string ENVIRONMENT::get_jdhelp(){ return std::string( JDHELP ); }
std::string ENVIRONMENT::get_jdhelpcmd(){ return std::string( JDHELPCMD ); }
std::string ENVIRONMENT::get_jdlicense(){ return std::string( JDLICENSE ); }


int window_manager = ENVIRONMENT::WM_UNKNOWN;


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
    const size_t end_quote_pos = args.rfind( "'" );

    // 複数の項目
    bool multi = false;

    // 省略形として"--with-"や"--enable-"などを取り出す
    while( ( found_pos = args.find( "'--with-", search_pos ) ) != std::string::npos ||
            ( found_pos = args.find( "'--enable-", search_pos ) ) != std::string::npos )
    {
        size_t quote_pos = args.find( "'", found_pos + 1 );

        if( quote_pos != std::string::npos && quote_pos != end_quote_pos)
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
// SVNリビジョンとして表示する文字列を返す
//
std::string get_svn_revision( const char* rev = NULL )
{
    std::string svn_revision = "SVN:";

    if( rev )
    {
        // "2000:2002MS"など[0-9:MS]の形式かどうか
        bool valid = true;
        unsigned int n;
        const size_t rev_length = strlen( rev );
        for( n = 0; n < rev_length; ++n )
        {
            if( (unsigned char)( rev[n] - 0x30 ) > 0x0A
                && rev[n] != 'M'
                && rev[n] != 'S' )
            {
                valid = false;
                break;
            }
        }

        if( valid ) svn_revision.append( std::string( "Rev." ) + std::string( rev ) );
    }

    if( svn_revision.compare( "SVN:" ) == 0 )
    {
        svn_revision.append( std::string( __DATE__ ) + "-" + std::string( __TIME__ ) );
    }

    return svn_revision;
}

//
// GITリビジョンとして表示する文字列を返す
// リビジョンが得られなかった場合（tarballのソース等）は、fallbackの日付を返す
// git_dirtyは、まだcommitされてない変更があるかどうか
//
std::string get_git_revision (const char *git_date, const char *git_hash, const int git_dirty, const char *fallback_date)
{
	std::string git_revision = "";

	// ハッシュ省略表記はgit 2.11から長さを自動で調整するようになった
	// ビルドメタデータが表記揺れするのは紛らわしいので固定長にする
	constexpr const size_t fixed_hash_length = 10;
	size_t string_length;
	size_t n;

	bool date_valid = false;
	if (git_date)
	{
		date_valid = true;
		string_length = strlen(git_date);
		if (string_length < 8) date_valid = false;
		if (date_valid)
		{
			for (n = 0; n < string_length; n++)
			{
				if (! isdigit(git_date[n]))
				{
					date_valid = false;
					break;
				}
			}
		}
		if (git_date[0] < '1') date_valid = false;
	}

	bool hash_valid = false;
	if (git_hash)
	{
		hash_valid = true;
		string_length = strlen(git_hash);
		if (string_length != fixed_hash_length) hash_valid = false;
		if (hash_valid)
		{
			for (n = 0; n < fixed_hash_length; n++)
			{
				if (! isgraph(git_hash[n]))
				{
					hash_valid = false;
					break;
				}
			}
		}
	}


	if (date_valid && hash_valid)
	{
		git_revision.append( std::string(git_date) + "(git:");
		git_revision.append( std::string(git_hash), 0, fixed_hash_length);
		if (git_dirty)
		{
			git_revision.append(":M");
		}
		git_revision.append(")");
	}
	else {
		git_revision.append( std::string(fallback_date) );
	}

	return git_revision;
}


//
// JDのバージョンを取得
//
std::string ENVIRONMENT::get_jdversion()
{
    std::stringstream jd_version;

#ifdef JDVERSION_SVN

#ifdef SVN_REVISION
    jd_version << get_svn_revision( SVN_REVISION );
#else
    jd_version << get_svn_revision( NULL );
#endif // SVN_REVISION

#else
    jd_version << MAJORVERSION << "."
                << MINORVERSION << "."
                << MICROVERSION << "-"
                << JDTAG << get_git_revision(GIT_DATE, GIT_HASH, GIT_DIRTY, JDDATE);
#endif // JDVERSION_SVN

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

    std::string dist_name;

#ifdef _WIN32
    LONG rc;
    DWORD dwSize;
    HKEY hKey;
    char *regVal;

    std::ostringstream vstr;
    vstr << "Windows";

    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((OSVERSIONINFO *)&osvi) != 0)
    {
        switch (osvi.dwPlatformId)
        {
        case VER_PLATFORM_WIN32_NT:
            // Read Windows edition from the registry
            rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                    0, KEY_READ | KEY_WOW64_64KEY, &hKey );
            if( rc == ERROR_SUCCESS )
            {
                // read size of ProductName
                rc = RegQueryValueEx( hKey, "ProductName",
                        NULL, NULL, NULL, &dwSize );
                if( rc == ERROR_SUCCESS && dwSize > 0 )
                {
                    // get buffer
                    regVal = (char *)malloc( dwSize );
                    if( regVal != NULL )
                    {
                        // read ProductName
                        rc = RegQueryValueEx( hKey, "ProductName",
                                NULL, NULL, (LPBYTE)regVal, &dwSize );
                        if( rc == ERROR_SUCCESS && strlen( regVal ) > 0 )
                        {
                            // may be contains "Windows" in the value
                            vstr.str( "" );
                            vstr << regVal;
                        }
                        free( regVal );
                    }
                }

                // read size of CSDVersion
                rc = RegQueryValueEx( hKey, "CSDVersion",
                        NULL, NULL, NULL, &dwSize );
                if( rc == ERROR_SUCCESS && dwSize > 0 )
                {
                    // get buffer
                    regVal = (char *)malloc( dwSize );
                    if( regVal != NULL )
                    {
                        // read CSDVersion
                        rc = RegQueryValueEx( hKey, "CSDVersion",
                                NULL, NULL, (LPBYTE)regVal, &dwSize );
                        if( rc == ERROR_SUCCESS && strlen( regVal ) > 0 )
                        {
                            // ServicePack version
                            vstr << " " << regVal;
                        }
                        free( regVal );
                    }
                }

                // close registry handle
                RegCloseKey( hKey ); // ignore errors
            }

            // OS architecture
            if (osvi.dwMajorVersion >= 5)
            {
                SYSTEM_INFO sysi;
                GetNativeSystemInfo(&sysi);
                if (sysi.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                    vstr << " x64";
                else if (sysi.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
                    vstr << " ia64";
            }
            break;
        case VER_PLATFORM_WIN32_WINDOWS:
            // Not support versions (Me, 98, 95, ... )
            break;
        default:
            // Unknown versions
            break;
        }

        // Build version
        vstr << " (build " << osvi.dwMajorVersion
                << "." << osvi.dwMinorVersion
                << "." << osvi.dwBuildNumber << ")";
    }
    dist_name = vstr.str();

#else // _WIN32
    std::string tmp;
    std::string text_data;

    // 各ディストリビューション共通の形式として定められた
    // http://www.freedesktop.org/software/systemd/man/os-release.html
    // 旧形式のコードは頃合いを見て削除する予定
    if( CACHE::load_rawdata( "/etc/os-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::reverse_iterator it = lines.rbegin();
        while( it != lines.rend() )
        {
            std::string name, value;

            size_t e;
            if( ( e = (*it).find( "=" ) ) != std::string::npos )
            {
                name = MISC::remove_spaces( (*it).substr( 0, e ) );
                value = MISC::remove_spaces( (*it).substr( e + 1 ) );
            }

            if( name == "PRETTY_NAME" && ! value.empty() )
            {
                tmp = MISC::cut_str( value, "\"", "\"" );
                break;
            }

            ++it;
        }
    }
    // LSB系 ( Ubuntu ..etc )
    else if( CACHE::load_rawdata( "/etc/lsb-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::reverse_iterator it = lines.rbegin();
        while( it != lines.rend() )
        {
            std::string lsb_name, lsb_data;

            size_t e;
            if( ( e = (*it).find( "=" ) ) != std::string::npos )
            {
                lsb_name = MISC::remove_spaces( (*it).substr( 0, e ) );
                lsb_data = MISC::remove_spaces( (*it).substr( e + 1 ) );
            }

            // 「DISTRIB_DESCRIPTION="Ubuntu 7.10"」などから「Ubuntu 7.10」を取得
            if( lsb_name == "DISTRIB_DESCRIPTION" && ! lsb_data.empty() )
            {
                tmp = MISC::cut_str( lsb_data, "\"", "\"" );
                break;
            }

            ++it;
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
        std::list< std::string >::iterator it = lines.begin();
        while( it != lines.end() )
        {
            // 名前が含まれている行を取得
            if( (*it).find( "BeleniX" ) != std::string::npos
                || (*it).find( "Nexenta" ) != std::string::npos
                || (*it).find( "SchilliX" ) != std::string::npos
                || (*it).find( "Solaris" ) != std::string::npos )
            {
                tmp = *it;
                break;
            }

            ++it;
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
    dist_name = MISC::remove_spaces( tmp );

    // 取得した文字が異常に長い場合は空にする
    if( dist_name.length() > 50 ) dist_name.clear();

#ifdef HAVE_SYS_UTSNAME_H

    char *sysname = 0, *release = 0, *machine = 0;

    // システムコール uname() 準拠：SVr4, POSIX.1-2001.
    struct utsname* uts;
    uts = (struct utsname*)malloc( sizeof( struct utsname ) );
    if( uname( uts ) != -1 )
    {
        sysname = uts->sysname;
        release = uts->release;
        machine = uts->machine;
    }

    // FreeBSD等やディストリ名が取得できなかった場合は"$ uname -rs"と同じ様式
    if( dist_name.empty() && sysname && release )
    {
        dist_name = std::string( sysname ) + " " + std::string( release );
    }

    // アーキテクチャがx86でない場合
    if( machine &&
        ( strlen( machine ) != 4
          || ! ( machine[0] == 'i'
               && machine[1] >= '3' && machine[1] <= '6'
               && machine[2] == '8' && machine[3] == '6' ) ) )
    {
        const std::string arch = "(" + std::string( machine ) + ")";

        if ( dist_name.find(arch, 0) == std::string::npos ) dist_name.append( " " + arch);
    }

    free( uts );
    uts = NULL;

#endif
#endif // _WIN32

    return dist_name;
}


//
// WM 判定
// TODO: 環境変数で判定できない場合の判定方法を考える
//
const int ENVIRONMENT::get_wm()
{
    if( window_manager != WM_UNKNOWN ) return window_manager;

    const std::string str_wm = MISC::getenv_limited( "DESKTOP_SESSION", 5 );

    if( str_wm.find( "xfce" ) != std::string::npos ) window_manager = WM_XFCE;
    else if( str_wm.find( "gnome" ) != std::string::npos ) window_manager = WM_GNOME;
    else if( str_wm.find( "kde" ) != std::string::npos ) window_manager = WM_KDE;
    else if( str_wm.find( "LXDE" ) != std::string::npos ) window_manager = WM_LXDE;

    if( window_manager == WM_UNKNOWN )
    {
        if( ! MISC::getenv_limited( "GNOME_DESKTOP_SESSION_ID" ).empty() )
        {
            window_manager = WM_GNOME;
        }
        else
        {
            const std::string str_wm = MISC::getenv_limited( "KDE_FULL_SESSION", 4 );
            if( str_wm == "true" ) window_manager = WM_KDE;
        }
    }

    return window_manager;
}


//
// WM名を文字列で返す
//
std::string ENVIRONMENT::get_wm_str()
{
    std::string desktop;

#ifdef _WIN32
#ifdef __MINGW32__
    return std::string( "build by mingw32" );
#else
    return std::string( "build with _WIN32" );
#endif
#else
    switch( get_wm() )
    {
        case WM_GNOME : desktop = "GNOME"; break;
        case WM_XFCE  : desktop = "XFCE";  break;
        case WM_KDE   : desktop = "KDE";   break;
        case WM_LXDE  : desktop = "LXDE";  break;
    }
#endif

    return desktop;
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
// 動作環境を取得
//
std::string ENVIRONMENT::get_jdinfo()
{
    std::stringstream jd_info;

    // バージョンを取得(jdversion.h)
    const std::string version = get_jdversion();

    // ディストリビューション名を取得
    const std::string distribution = get_distname();

    // デスクトップ環境を取得( 環境変数から判別可能の場合 )
    std::string desktop = get_wm_str();

    // その他
    std::string other;

    // $LANG が ja_JP.UTF-8 でない場合は"その他"に追加する。
    const std::string lang = MISC::getenv_limited( "LANG", 11 );
    if( lang.empty() ) other.append( "LANG 未定義" );
    else if( lang != "ja_JP.utf8" && lang != "ja_JP.UTF-8" ) other.append( "LANG = " + lang );

    jd_info <<
    "[バージョン] " << version << "\n" <<
//#ifdef SVN_REPOSITORY
//    "[リポジトリ ] " << SVN_REPOSITORY << "\n" <<
//#endif
    "[ディストリ ] " << distribution << "\n" <<
    "[パッケージ] " << "バイナリ/ソース( <配布元> )" << "\n" <<
    "[ DE／WM ] " << desktop << "\n" <<
    "[　gtkmm 　] " << get_gtkmm_version() << "\n" <<
    "[　glibmm 　] " << get_glibmm_version()<< "\n" <<
#ifdef CONFIGURE_ARGS
    "[オプション ] " << get_configure_args( CONFIGURE_OMITTED_MULTILINE ) << "\n" <<
#endif
    "[ そ の 他 ] " << other << "\n";

    return jd_info.str();
}

