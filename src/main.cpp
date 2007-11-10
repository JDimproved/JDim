// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "config/globalconf.h"

#include "winmain.h"
#include "cache.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include <signal.h>
#include <string>
#include <sys/time.h>
#include <errno.h>

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

WinMain* Win_Main = NULL;

//
// ロック
//
// 下のいずれかの値が戻る
//

enum
{
    LOCK_OK = 0,  // ロックした
    LOCK_ALIVE,   // 他のJDが起動している
    LOCK_EXIST    // ロックファイルはあるがJDは起動していない
};

int lock_jd()
{
    pid_t pid = getpid();
    char str_pid[ 256 ];
    std::string path = CACHE::path_lock();

    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ){

        CACHE::load_rawdata( path, str_pid, 256 );
        pid = atoi( str_pid );

#ifdef _DEBUG
        std::cout << "lock file exits pid = " << pid << std::endl;
#endif

        int killret = kill( pid, 0 );
        if( killret == 0 ||  errno == EPERM ){

#ifdef _DEBUG
            std::cout << "other jd is alive\n";
#endif
            return LOCK_ALIVE;

        }

        return LOCK_EXIST;
    }

    snprintf( str_pid, 256, "%d", pid );
    CACHE::save_rawdata( path, str_pid, strlen( str_pid ) );

    return LOCK_OK;
}


// ロック解除
void unlock_jd()
{
    std::string path = CACHE::path_lock();

    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
}



// バックアップ復元
void restore_bkup()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    std::string path_main = CACHE::path_xml_listmain();
    std::string path_favor = CACHE::path_xml_favorite();
    std::string path_main_bkup = CACHE::path_xml_listmain_bkup();
    std::string path_favor_bkup = CACHE::path_xml_favorite_bkup();
    std::string path_main_old = path_main + "." + MISC::itostr( tv.tv_sec );
    std::string path_favor_old = path_favor + "." + MISC::itostr( tv.tv_sec );

    bool bkup_main = ( CACHE::file_exists( path_main_bkup ) == CACHE::EXIST_FILE );
    bool bkup_favor = ( CACHE::file_exists( path_favor_bkup ) == CACHE::EXIST_FILE );

    if( bkup_main || bkup_favor ){

        Gtk::MessageDialog* mdiag
        = new Gtk::MessageDialog( "前回の起動時に正しくJDが終了されませんでした。\n\n板リストとお気に入りをバックアップファイルから復元しますか？\nキャンセルを押すとバックアップファイルを削除します。",
                                  false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        int ret = mdiag->run();
        delete mdiag;
        if( ret == Gtk::RESPONSE_OK ){

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

            mdiag = new Gtk::MessageDialog( msg );
            mdiag->run();
            delete mdiag;
        }
        else{
            if( bkup_main ) unlink( path_main_bkup.c_str() );
            if( bkup_favor ) unlink( path_favor_bkup.c_str() );
        }
    }
}



// SIGINTのハンドラ
void sig_handler( int sig )
{
    if( sig == SIGHUP || sig == SIGINT || sig == SIGQUIT ){

#ifdef _DEBUG
        std::cout << "sig_handler sig = " << sig << std::endl;
#endif
        if( Win_Main ) Win_Main->shutdown();
        unlock_jd();
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
    unlock_jd();

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
        unlock_jd();
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

int main( int argc, char **argv )
{
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

    GMemVTable vtable;
    vtable = *glib_mem_profiler_table;
    vtable.malloc = malloc;
    vtable.realloc = realloc;
    vtable.free = free;
    vtable.calloc = calloc;
    vtable.try_malloc = malloc;
    vtable.try_realloc = realloc;
    g_mem_set_vtable( &vtable );

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
    gnome_init( "jd", "1.0", argc, argv );
    GnomeClient *client_gnome = gnome_master_client();
    if( client_gnome ) gtk_signal_connect( GTK_OBJECT( client_gnome ), "save_yourself", GTK_SIGNAL_FUNC( save_yourself_gnome ), NULL );
    else MISC::ERRMSG( "failed to connect to gnome session manager" );
#endif

    // 全体設定ロード
    bool init = !( CONFIG::load_conf() );

    if( init ){

        // 設定ファイルが読み込めないときに存在するか確認
        int exists = CACHE::file_exists( CACHE::path_conf() );
        if( exists == CACHE::EXIST_FILE || exists == CACHE::EXIST_DIR ){

            std::string msg = "JDの設定ファイル(" + CACHE::path_conf() + ")は存在しますが読み込むことが出来ませんでした。\n\n起動しますか？";
            Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( msg,
                                                                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            int ret = mdiag->run();
            delete mdiag;
            if( ret != Gtk::RESPONSE_OK ) return 0;
        }

        // 初回起動時にルートを作る
        CACHE::mkdir_root();
    }

    // ロック
    int lock = lock_jd();
    if( lock == LOCK_ALIVE ){

        Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( "JDは既に起動しています。起動しますか？",
                                                            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        int ret = mdiag->run();
        delete mdiag;
        if( ret != Gtk::RESPONSE_OK ) return 0;

        unlock_jd();
        lock_jd();
    }
    else if( lock == LOCK_EXIST ){ 

        Gtk::MessageDialog* mdiag = new Gtk::MessageDialog( "前回起動時にJDが異常終了しました。\n\nロックファイルを削除して起動しますか？",
                                                            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );

        mdiag->set_default_response( Gtk::RESPONSE_OK );
        int ret = mdiag->run();
        delete mdiag;
        if( ret != Gtk::RESPONSE_OK ) return 0;

        unlock_jd();
        lock_jd();
    }

    // バックアップファイル復元
    restore_bkup();

    Win_Main = new WinMain( init );
    if( Win_Main ){

        m.run( *Win_Main );

        delete Win_Main;
        Win_Main = NULL;
    }

#ifdef USE_XSMP
    xsmp_session_end( &xsmpdata );
#endif

    // ロック解除
    unlock_jd();

    return 0;
}
