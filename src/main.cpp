// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_MEM_PROFILE
#include "jddebug.h"

#include "config/globalconf.h"

#include "winmain.h"
#include "cache.h"
#include "environment.h"
#include "iomonitor.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"
#include "jdlib/ssl.h"
#include "jdlib/jdregex.h"

#include <signal.h>
#include <string>
#include <sys/time.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <getopt.h>

#ifdef USE_GNOMEUI
#include <gnome.h>
#endif

#ifdef USE_XSMP
#include <X11/ICE/ICElib.h>
#include <X11/SM/SMlib.h>

struct XSMPDATA
{
    SmcConn smc_connect;
    IceConn ice_connect;
    guint id_process_message;
};
#endif

enum
{
    MAX_SAFE_ARGC = 4,   // 引数の数の制限値
    MAX_SAFE_ARGV = 1024 // 各引数の文字列の制限値
};

JDWinMain* Win_Main = NULL;


// バックアップ復元
void restore_bkup( const bool no_restore_bkup )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    const std::string path_main = CACHE::path_xml_listmain();
    const std::string path_favor = CACHE::path_xml_favorite();
    const std::string path_main_bkup = CACHE::path_xml_listmain_bkup();
    const std::string path_favor_bkup = CACHE::path_xml_favorite_bkup();
    const std::string path_main_old = path_main + "." + MISC::itostr( tv.tv_sec );
    const std::string path_favor_old = path_favor + "." + MISC::itostr( tv.tv_sec );

    const bool bkup_main = ( CACHE::file_exists( path_main_bkup ) == CACHE::EXIST_FILE );
    const bool bkup_favor = ( CACHE::file_exists( path_favor_bkup ) == CACHE::EXIST_FILE );

    if( bkup_main || bkup_favor ){

        bool restore = false;

        if( ! no_restore_bkup ){

            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog(
                "前回の起動時に正しくJDimが終了されませんでした。\n\n"
                "板リストとお気に入りをバックアップファイルから復元しますか？\n"
                "いいえを押すとバックアップファイルを削除します。",
                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag->run() == Gtk::RESPONSE_YES ) restore = true;
            delete mdiag;
        }

        if( restore ){

            if( bkup_main ){
                rename( path_main.c_str(), path_main_old.c_str() );
                rename( path_main_bkup.c_str(), path_main.c_str() );
            }
            if( bkup_favor ){
                rename( path_favor.c_str(), path_favor_old.c_str() );
                rename( path_favor_bkup.c_str(), path_favor.c_str() );
            }

            std::string msg = "更新しました。\n\n古いリストはそれぞれ\n\n";

            if( bkup_main ) msg += path_main_old + "\n";
            if( bkup_favor ) msg += path_favor_old + "\n";

            msg += "\nに移動しました。";

            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( msg );
            mdiag->run();
            delete mdiag;
        }
        else{
            if( bkup_main ) unlink( to_locale_cstr( path_main_bkup ) );
            if( bkup_favor ) unlink( to_locale_cstr( path_favor_bkup ) );
        }
    }
}



// SIGINTのハンドラ
void sig_handler( int sig )
{
#ifdef _WIN32
    if( sig == SIGINT ){
#else
    if( sig == SIGHUP || sig == SIGINT || sig == SIGQUIT ){
#endif

#ifdef _DEBUG
        std::cout << "sig_handler sig = " << sig << std::endl;
#endif
        if( Win_Main ) Win_Main->save_session();
    }

    exit(0);
}



// GNOMEUI によるセッション管理
#ifdef USE_GNOMEUI

// gnomeセッションマネージャから "save_yourself" シグナルを受け取った
static int save_yourself_gnome( GnomeClient *client,
                          int phase,
                          GnomeSaveStyle save_style,
                          int shutdown,
                          GnomeInteractStyle interact_style,
                          int fast,
                          gpointer client_data
    )
{

    if( Win_Main ) Win_Main->save_session();

    return TRUE;
}

#endif



// XSMPによるセッション管理
#ifdef USE_XSMP

// セッションが終了したので情報を保存するコールバック関数
void xsmp_session_save_yourself( SmcConn smc_connect,
                                 SmPointer client_data,
                                 int save_type,
                                 Bool shutdown,
                                 int interact_style,
                                 Bool fast )
{
    if( shutdown && !fast ){
#ifdef _DEBUG
        std::cout << "session_save_yourself\n";
#endif
        if( Win_Main ) Win_Main->save_session();
    }

    SmcSaveYourselfDone( smc_connect, TRUE ) ;
}


// ダミー
void xsmp_session_die( SmcConn conn, SmPointer data ){}
void xsmp_session_save_complete( SmcConn conn, SmPointer data ){}
void xsmp_session_shutdown_cancelled( SmcConn conn, SmPointer data ){} 



gboolean ice_process_message( GIOChannel *channel,
                              GIOCondition condition,
                              XSMPDATA *xsmpdata )
{
    if( ! xsmpdata->ice_connect ) return FALSE;

    IceProcessMessagesStatus status = IceProcessMessages( xsmpdata->ice_connect, NULL, NULL );

#ifdef _DEBUG
    std::cout << "ice_process_message status = " << status << std::endl;
#endif

    if( status == IceProcessMessagesIOError ){

#ifdef _DEBUG
        std::cout << "ice_process_message IOError\n";
#endif
            
        IceCloseConnection( xsmpdata->ice_connect );
        xsmpdata->ice_connect = NULL;
        return FALSE;
    }

    return TRUE;
}


void ice_watch_proc( IceConn ice_connect,
                     IcePointer client_data,
                     Bool opening,
                     IcePointer *watch_data )
{
    XSMPDATA *xsmpdata = ( XSMPDATA* ) client_data;
    xsmpdata->ice_connect = ice_connect;

    if( xsmpdata->id_process_message ){

#ifdef _DEBUG
        std::cout << "ice_watch_proc remove\n";
#endif
        g_source_remove( xsmpdata->id_process_message );
        xsmpdata->id_process_message = 0;
    }

    else if( opening && xsmpdata->ice_connect ){

        int fd = IceConnectionNumber( xsmpdata->ice_connect );
        if( fd >= 0 ){

#ifdef _DEBUG
            std::cout << "ice_watch_proc opening fd = " << fd << std::endl;
#endif

            GIOChannel *channel = g_io_channel_unix_new( fd );
            *watch_data =  xsmpdata;
            xsmpdata->id_process_message = g_io_add_watch_full( channel,
                                                                G_PRIORITY_DEFAULT,
                                                                ( GIOCondition )( G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP ),
                                                                ( GIOFunc ) ice_process_message,
                                                                xsmpdata,
                                                                NULL );
            g_io_channel_unref( channel );
        }
    }
}



// XSMP初期化関数
void xsmp_session_init( XSMPDATA* xsmpdata )
{
    if( xsmpdata->smc_connect ) return;
    if( g_getenv("SESSION_MANAGER") == NULL ) return;

#ifdef _DEBUG
    std::cout << "SESSION_MANAGER = " << g_getenv("SESSION_MANAGER") << std::endl;
#endif

    IceAddConnectionWatch( ice_watch_proc, xsmpdata );

    SmcCallbacks smc_callbacks;
    memset( &smc_callbacks, 0, sizeof( SmcCallbacks ) );
    smc_callbacks.save_yourself.callback  = xsmp_session_save_yourself;
    smc_callbacks.die.callback                 = xsmp_session_die;
    smc_callbacks.save_complete.callback       = xsmp_session_save_complete;
    smc_callbacks.shutdown_cancelled.callback  = xsmp_session_shutdown_cancelled;

    gchar *id = NULL;
    gchar errstr[ 1024 ];
    xsmpdata->smc_connect
    = SmcOpenConnection( NULL, NULL, SmProtoMajor, SmProtoMinor,
                         SmcSaveYourselfProcMask | SmcDieProcMask | SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask,
                         &smc_callbacks,
                         NULL,
                         &id,
                         1024, errstr );
    if( ! xsmpdata->smc_connect ){
        MISC::ERRMSG( "SmcOpenConnection failed." );
        return;
    }
}


// XSMP終了関数
void xsmp_session_end( XSMPDATA* xsmpdata )
{
    if( xsmpdata->smc_connect ) SmcCloseConnection( xsmpdata->smc_connect, 0, NULL );
    xsmpdata->smc_connect = NULL;
}

#endif

////////////////////////////////////////////////////////////

void usage( const int status )
{
    // -h, --help で表示するメッセージ
    std::stringstream help_message;
    help_message <<
    "Usage: jdim [OPTION] [URL,FILE]\n"
    "\n"
    "-h, --help\n"
    "        Display this information\n"
    //"-t <url>, --tab=<url>\n"
    //"        URL open of BBS etc by Tab\n"
    "-m, --multi\n"
    "        Do not quit even if multiple sub-process\n"
    "-n, --norestore\n"
    "        Do not restore backup files\n"
    "-s, --skip-setup\n"
    "        Skip the setup dialog\n"
    "-l, --logfile\n"
    "        Write message to msglog file\n"
    "-g, --geometry WxH-X+Y\n"
    "        The initial size and location\n"
    "-V, --version\n"
    "        Display version of this program\n";

    std::cout << help_message.str() << std::endl;

    exit( status );
}


int main( int argc, char **argv )
{
    /*--- 引数処理 --------------------------------------------------*/

    // 引数の数を制限
    if( argc > MAX_SAFE_ARGC )
    {
        MISC::ERRMSG( "main(): Too many arguments." );
        exit( EXIT_FAILURE );
    }

    // 各引数の文字数を制限
    int i;
    for( i = 0; i < argc; ++i )
    {
        if( strlen( argv[ i ] ) > MAX_SAFE_ARGV )
        {
            MISC::ERRMSG( "main(): Too many the strings in argument." );
            exit( EXIT_FAILURE );
        }
    }

    // "現在のタブ/新規タブ"など引数によって開き方を変えたい場合は、--tab=<url>
    // など新しいオプションを追加する
    // --help, --tab=<url>, --multi, --norestore, --logfile --version
    const struct option options[] =
    {
        { "help", 0, 0, 'h' },
        //{ "tab", 1, 0, 't' },
        { "multi", 0, 0, 'm' },
        { "norestore", 0, 0, 'n' },
        { "skip-setup", 0, 0, 's' },
        { "logfile", 0, 0, 'l' },
        { "geometry", required_argument, NULL, 'g' },
        { "version", 0, 0, 'V' },
        { 0, 0, 0, 0 }
    };

    std::string url;
    bool multi_mode = false;
    bool no_restore_bkup = false;
    bool skip_setupdiag = false;
    bool logfile_mode = false;
    int init_w = -1;
    int init_h = -1;
    int init_x = -1;
    int init_y = -1;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;
    std::string query;

    // -h, -t <url>, -m, -n, -s, -l, -g WxH+X+Y, -V
    int opt = 0;
    while( ( opt = getopt_long( argc, argv, "ht:mnslg:V", options, NULL ) ) != -1 )
    {
        switch( opt )
        {
            case 'h':
                usage( EXIT_SUCCESS );
                break;

            //case 't':
                //url = optarg;
                //break;

            case 'm':
#ifndef _WIN32
                multi_mode = true;
#endif
                break;

            case 'n':
                no_restore_bkup = true;
                break;

            case 's':
                skip_setupdiag = true;
                break;

            case 'l': // メッセージをログファイルに出力
                logfile_mode = true;
                break;

            case 'g':
                if( ! optarg ) usage( EXIT_FAILURE );

                query = "(([0-9]*)x([0-9]*))?\\-([0-9]*)\\+([0-9]*)";
                if( regex.exec( query, optarg, offset, icase, newline, usemigemo, wchar ) ){

                    if( ! regex.str( 2 ).empty() ) init_w = atoi( regex.str( 2 ).c_str() );
                    if( ! regex.str( 3 ).empty() ) init_h = atoi( regex.str( 3 ).c_str() );
                    if( ! regex.str( 4 ).empty() ) init_x = atoi( regex.str( 4 ).c_str() );
                    if( ! regex.str( 5 ).empty() ) init_y = atoi( regex.str( 5 ).c_str() );
                }
                else usage( EXIT_FAILURE );

                break;

            case 'V': // バージョンと完全なconfigureオプションを表示
                std::cout << ENVIRONMENT::get_progname() << " " << ENVIRONMENT::get_jdversion() << "\n" <<
                ENVIRONMENT::get_jdcopyright() << "\n"
                "configure: " << ENVIRONMENT::get_configure_args( ENVIRONMENT::CONFIGURE_FULL ) << std::endl;
                exit( 0 );
                break;

            default:
                usage( EXIT_FAILURE );
        }
    }

    if( url.empty() )
    {
        // 引数がURLのみの場合
        if( argc > optind )
        {
            url = argv[ optind ];
        }
        // -m 、-s でなく、URLを含まない引数だけの場合は終了
        else if( optind > 1 && ! ( multi_mode || no_restore_bkup || skip_setupdiag || logfile_mode ) ){
            usage( EXIT_FAILURE );
        }
    }
    /*---------------------------------------------------------------*/

#ifndef _WIN32
    // SIGINT、SIGQUITのハンドラ設定
    struct sigaction sigact;
    sigset_t blockset;

    sigemptyset( &blockset );
    sigaddset( &blockset, SIGHUP );
    sigaddset( &blockset, SIGINT );
    sigaddset( &blockset, SIGQUIT );
    sigaddset( &blockset, SIGTERM );

    memset( &sigact, 0, sizeof(struct sigaction) );  
    sigact.sa_handler = sig_handler;
    sigact.sa_mask = blockset;
    sigact.sa_flags = SA_RESETHAND;

    // シグナルハンドラ設定
    if( sigaction( SIGHUP, &sigact, NULL ) != 0
        || sigaction( SIGINT, &sigact, NULL ) != 0
        || sigaction( SIGQUIT, &sigact, NULL ) != 0 ){
        fprintf( stderr, "sigaction failed\n" );
        exit( 1 );
    }
#endif

#if ! GLIB_CHECK_VERSION(2, 46, 0)
#ifdef _DEBUG_MEM_PROFILE
    g_mem_set_vtable( glib_mem_profiler_table );
    atexit( g_mem_profile );
#else
    GMemVTable vtable;
    vtable = *glib_mem_profiler_table;
    vtable.malloc = malloc;
    vtable.realloc = realloc;
    vtable.free = free;
    vtable.calloc = calloc;
    vtable.try_malloc = malloc;
    vtable.try_realloc = realloc;
    g_mem_set_vtable( &vtable );
#endif
#endif

#if defined(WITH_GTKMM_3_0) && !defined(ENABLE_XINPUT_EXTENSION)
    // 板一覧やスレ一覧でマウスホイールによるスクロールが動作しないことがあるため
    // 環境変数を設定してXInputの拡張機能を無効にする
    // https://developer.gnome.org/gtk3/stable/gtk-x11.html
    Glib::setenv( "GDK_CORE_DEVICE_EVENTS", "1" );
#endif

    Gtk::Main m( &argc, &argv );

    // XSMPによるセッション管理
#ifdef USE_XSMP

#ifdef _DEBUG
    std::cout << "USE_XSMP\n";
#endif

    XSMPDATA xsmpdata;
    memset( &xsmpdata, 0, sizeof( XSMPDATA ) );
    xsmp_session_init( &xsmpdata );    
#endif

    // GNOMEUIによるセッション管理
#ifdef USE_GNOMEUI

#ifdef _DEBUG
    std::cout << "USE_GNOMEUI\n";
#endif

    // gnomeセッションマネージャとつないでログアウト時に"save_yourself"シグナルをもらう
    gnome_init( "jdim", "1.0", argc, argv );
    GnomeClient *client_gnome = gnome_master_client();
    if( client_gnome ) gtk_signal_connect( GTK_OBJECT( client_gnome ), "save_yourself", GTK_SIGNAL_FUNC( save_yourself_gnome ), NULL );
    else MISC::ERRMSG( "failed to connect to gnome session manager" );
#endif

    JDLIB::init_ssl();

    // 全体設定ロード
    bool init = !( CONFIG::load_conf() );

    if( init ){

        // 設定ファイルが読み込めないときに存在するか確認
        int exists = CACHE::file_exists( CACHE::path_conf() );
        if( exists == CACHE::EXIST_FILE || exists == CACHE::EXIST_DIR ){

            std::string msg = "JDimの設定ファイル(" + CACHE::path_conf()
                              + ")は存在しますが読み込むことが出来ませんでした。\n\n起動しますか？";
            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( msg,
                                                                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            int ret = mdiag->run();
            delete mdiag;
            if( ret != Gtk::RESPONSE_YES ) return 0;
        }

        // 初回起動時にルートを作る
        CACHE::mkdir_root();
    }

    // メッセージをログファイルに出力
    if( logfile_mode && CACHE::mkdir_logroot() ){
        FILE *tmp; // warning 消し
        const char *logfile = to_locale_cstr( CACHE::path_msglog() );
        tmp = freopen( logfile, "ab", stdout );
        setbuf( tmp, NULL );
        tmp = freopen( logfile, "ab", stderr );
        setbuf( tmp, NULL );
    }

    /*--- IOMonitor -------------------------------------------------*/
    CORE::IOMonitor iomonitor;

    // FIFOの状態をチェックする
    if( iomonitor.get_fifo_stat() == CORE::FIFO_OK )
    {
        // 引数にURLがある
        if( ! url.empty() )
        {
            // ローカルファイルかどうかチェック
            const std::string url_real = CACHE::get_realpath( url );
            if( ! url_real.empty() ) url = url_real;

            // FIFOに書き込む
            iomonitor.send_command( url.c_str() );

            // マルチモードでなく、メインプロセスでもない場合は終了
            if( ! multi_mode && ! iomonitor.is_main_process() ) return 0;
        }
        // マルチモードでなく、メインプロセスでもない場合は問い合わせる
        else if( ! multi_mode && ! iomonitor.is_main_process() )
        {
            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( "JDimは既に起動しています。起動しますか？",
                                                                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            int ret = mdiag->run();
            delete mdiag;
            if( ret != Gtk::RESPONSE_YES ) return 0;
        }
    }
    // FIFOに問題がある(FATで作成出来ないなど)
    else
    {
        if( CONFIG::get_show_diag_fifo_error() )
        {
            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( CACHE::path_lock() + "の作成またはオープンに問題があります。このまま起動しますか？",
                                                                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

            Gtk::CheckButton chk_button( "今後表示しない" );
            mdiag->get_vbox()->pack_start( chk_button, Gtk::PACK_SHRINK );
            chk_button.show();

            const int ret = mdiag->run();

            CONFIG::set_show_diag_fifo_error( ! chk_button.get_active() );

            delete mdiag;

            if( ret != Gtk::RESPONSE_YES )
            {
                CONFIG::save_conf();
                return 1;
            }
        }
    }

    /*---------------------------------------------------------------*/
    // バックアップファイル復元
    restore_bkup( no_restore_bkup );

    Win_Main = new JDWinMain( init, skip_setupdiag, init_w, init_h, init_x, init_y );
    if( Win_Main ){

        m.run( *Win_Main );

        delete Win_Main;
        Win_Main = NULL;
    }

#ifdef USE_XSMP
    xsmp_session_end( &xsmpdata );
#endif

    JDLIB::deinit_ssl();

    return 0;
}
