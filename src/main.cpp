// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_MEM_PROFILE
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "config/globalconf.h"

#include "winmain.h"
#include "cache.h"
#include "environment.h"
#include "iomonitor.h"

#include "jdlib/miscmsg.h"
#include "jdlib/jdsocket.h"
#include "jdlib/jdregex.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <string>

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


static JDWinMain* Win_Main = nullptr;


// SIGINTのハンドラ
[[noreturn]]
static void sig_handler( int sig )
{
    if( sig == SIGHUP || sig == SIGINT || sig == SIGQUIT ){

#ifdef _DEBUG
        std::cout << "sig_handler sig = " << sig << std::endl;
#endif
        if( Win_Main ) Win_Main->save_session();
    }

    exit(0);
}



// XSMPによるセッション管理
#ifdef USE_XSMP

// セッションが終了したので情報を保存するコールバック関数
static void xsmp_session_save_yourself( SmcConn smc_connect,
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
static void xsmp_session_die( SmcConn conn, SmPointer data ) {}
static void xsmp_session_save_complete( SmcConn conn, SmPointer data ) {}
static void xsmp_session_shutdown_cancelled( SmcConn conn, SmPointer data ) {}



static gboolean ice_process_message( GIOChannel *channel,
                                     GIOCondition condition,
                                     XSMPDATA *xsmpdata )
{
    if( ! xsmpdata->ice_connect ) return FALSE;

    IceProcessMessagesStatus status = IceProcessMessages( xsmpdata->ice_connect, nullptr, nullptr );

#ifdef _DEBUG
    std::cout << "ice_process_message status = " << status << std::endl;
#endif

    if( status == IceProcessMessagesIOError ){

#ifdef _DEBUG
        std::cout << "ice_process_message IOError\n";
#endif
            
        IceCloseConnection( xsmpdata->ice_connect );
        xsmpdata->ice_connect = nullptr;
        return FALSE;
    }

    return TRUE;
}


static void ice_watch_proc( IceConn ice_connect,
                            IcePointer client_data,
                            Bool opening,
                            IcePointer *watch_data )
{
    XSMPDATA *xsmpdata = reinterpret_cast<XSMPDATA*>( client_data );
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
                                                                static_cast<GIOCondition>( G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP ),
                                                                reinterpret_cast<GIOFunc>(ice_process_message),
                                                                xsmpdata,
                                                                nullptr );
            g_io_channel_unref( channel );
        }
    }
}



// XSMP初期化関数
static void xsmp_session_init( XSMPDATA* xsmpdata )
{
    if( xsmpdata->smc_connect ) return;
    if( g_getenv("SESSION_MANAGER") == nullptr ) return;

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

    gchar *id = nullptr;
    char errstr[1024] = "[XSMP] Failed connecting to the session manager. ";
    const int header_msg_size = std::strlen( errstr );
    xsmpdata->smc_connect
    = SmcOpenConnection( nullptr, nullptr, SmProtoMajor, SmProtoMinor,
                         SmcSaveYourselfProcMask | SmcDieProcMask | SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask,
                         &smc_callbacks,
                         nullptr,
                         &id,
                         1023 - header_msg_size, errstr + header_msg_size );
    if( ! xsmpdata->smc_connect ){
        MISC::ERRMSG( errstr );
        return;
    }
}


// XSMP終了関数
static void xsmp_session_end( XSMPDATA* xsmpdata )
{
    if( xsmpdata->smc_connect ) SmcCloseConnection( xsmpdata->smc_connect, 0, nullptr );
    xsmpdata->smc_connect = nullptr;
}

#endif

////////////////////////////////////////////////////////////


/** @brief Gioのアプリケーションを管理する
 *
 * シグナルハンドラを安全に切断するため sigc::trackable を継承する
 */
class App : public sigc::trackable
{
    Glib::RefPtr<Gio::Application> m_app;

    bool multi_mode{}; ///< サブプロセスのとき多重起動するか
    bool skip_setupdiag{}; ///< 初回起動時の設定ダイアログを表示するか
    bool logfile_mode{}; ///< エラーなどのメッセージをファイルに出力するか

    int init_w = -1; ///< メインウィンドウの幅
    int init_h = -1; ///< メインウィンドウの高さ
    int init_x = -1; ///< メインウィンドウを表示する位置(横軸)
    int init_y = -1; ///< メインウィンドウを表示する位置(縦軸)

    bool init{}; ///< 初回起動か
    std::FILE* m_redirect_stdout{};
    std::FILE* m_redirect_stderr{};
    CORE::IOMonitor iomonitor;

    bool m_with_gui{};

public:
    App();
    ~App() noexcept;

    int run( int argc, char* argv[] ) { return m_app->run( argc, argv ); }
    void quit() { m_app->quit(); }

private:
    int slot_handle_local_options( const Glib::RefPtr<Glib::VariantDict>& options );
    bool slot_option_geometry( const Glib::ustring& option_name, const Glib::ustring& value, bool has_value );

    void slot_startup();
    bool setup_fifo( std::string url );
    JDWinMain* create_window();
    void slot_activate();
    void slot_open( const Gtk::Application::type_vec_files& files, const Glib::ustring& hint );
    void slot_hide_window();
};


App::App()
{
    Glib::ustring app_id = "com.github.jdimproved.jdim";
    // アプリケーションの排他制御は {Gtk,Gio}::Application ではなくロックファイルで行う
    const auto flags = Gio::APPLICATION_NON_UNIQUE | Gio::APPLICATION_HANDLES_OPEN;

    if( gtk_init_check( nullptr, nullptr ) ) {
        m_app = Gtk::Application::create( app_id, flags );
        m_with_gui = true;
    }
    else {
        // デスクトップが無い環境でもヘルプやバージョンを出力できるように初期化する
        m_app = Gio::Application::create( app_id, flags );
    }
    Gio::Application::set_default( m_app );

    m_app->add_main_option_entry( Gio::Application::OPTION_TYPE_BOOL,
                                  "multi", 'm', "Do not quit even if multiple sub-process" );
    m_app->add_main_option_entry( Gio::Application::OPTION_TYPE_BOOL,
                                  "skip-setup", 's', "Skip the setup dialog" );
    m_app->add_main_option_entry( Gio::Application::OPTION_TYPE_BOOL,
                                  "logfile", 'l', "Write message to msglog file" );
    m_app->add_main_option_entry( sigc::mem_fun( *this, &App::slot_option_geometry ),
                                  "geometry", 'g', "The initial size and location", "WxH-X+Y" );
    m_app->add_main_option_entry( Gio::Application::OPTION_TYPE_BOOL,
                                  "version", 'V', "Display version of this program" );

    m_app->signal_handle_local_options().connect( sigc::mem_fun( *this, &App::slot_handle_local_options ) );
    m_app->signal_startup().connect( sigc::mem_fun( *this, &App::slot_startup ) );
    m_app->signal_activate().connect( sigc::mem_fun( *this, &App::slot_activate ) );
    m_app->signal_open().connect( sigc::mem_fun( *this, &App::slot_open ) );
}


App::~App() noexcept
{
    if( m_redirect_stdout ) std::fclose( m_redirect_stdout );
    if( m_redirect_stderr ) std::fclose( m_redirect_stderr );
}


/** @brief コマンドラインオプションの解析
 *
 * @param[in] options オプションの解析結果
 * @return プログラムを続行するときは-1、それ以外は終了コード
 */
int App::slot_handle_local_options( const Glib::RefPtr<Glib::VariantDict>& options )
{
    if( options->contains( "version" ) ) {
        // バージョンと完全なビルドオプションを表示
        std::cout << ENVIRONMENT::get_progname() << " " << ENVIRONMENT::get_jdversion() << "\n" <<
        ENVIRONMENT::get_jdcopyright() << "\n"
        "configure: " << ENVIRONMENT::get_configure_args( ENVIRONMENT::CONFIGURE_FULL ) << std::endl;
        return 0;
    }

    multi_mode = options->contains( "multi" );
    skip_setupdiag = options->contains( "skip-setup" );
    logfile_mode = options->contains( "logfile" );

    return -1;
}


/** @brief geometryオプションの解析
 *
 * @param[in] option_name オプション名
 * @param[in] value       解析前のオプション値
 * @param[in] has_value   オプション値を持っているか
 * @return 解析に成功したかどうか
 */
bool App::slot_option_geometry( const Glib::ustring& option_name, const Glib::ustring& value, bool has_value )
{
    if( ! has_value ) return false;

    JDLIB::Regex regex;
    constexpr size_t offset = 0;
    constexpr bool icase = false;
    constexpr bool newline = true;
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;
    const std::string query = "(([0-9]*)x([0-9]*))?\\-([0-9]*)\\+([0-9]*)";
    if( regex.exec( query, value, offset, icase, newline, usemigemo, wchar ) ) {

        if( regex.length( 2 ) > 0 ) init_w = std::atoi( regex.str( 2 ).c_str() );
        if( regex.length( 3 ) > 0 ) init_h = std::atoi( regex.str( 3 ).c_str() );
        if( regex.length( 4 ) > 0 ) init_x = std::atoi( regex.str( 4 ).c_str() );
        if( regex.length( 5 ) > 0 ) init_y = std::atoi( regex.str( 5 ).c_str() );
        return true;
    }
    return false;
}


/**
 * @brief Gtk::Application, Gio::Application の機能を使って排他制御を行わないためスタートアップは必ず実行される
 */
void App::slot_startup()
{
    if( ! m_with_gui ) return;

    // 全体設定ロード
    init = !( CONFIG::load_conf() );

    if( init ){

        // 設定ファイルが読み込めないときに存在するか確認
        const int exists = CACHE::file_exists( CACHE::path_conf() );
        if( exists == CACHE::EXIST_FILE || exists == CACHE::EXIST_DIR ){

            std::string msg = "JDimの設定ファイル(" + CACHE::path_conf()
                              + ")は存在しますが読み込むことが出来ませんでした。\n\n起動しますか？";
            Gtk::MessageDialog mdiag( msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            const int ret = mdiag.run();
            if( ret != Gtk::RESPONSE_YES ) return quit();
        }

        // 初回起動時にルートを作る
        CACHE::mkdir_root();
    }

    // メッセージをログファイルに出力
    if( logfile_mode && CACHE::mkdir_logroot() ){
        const std::string logfile = Glib::locale_from_utf8( CACHE::path_msglog() );
        m_redirect_stdout = std::freopen( logfile.c_str(), "ab", stdout );
        m_redirect_stderr = std::freopen( logfile.c_str(), "ab", stderr );

        if( m_redirect_stdout ) std::setbuf( m_redirect_stdout, nullptr );
        if( m_redirect_stderr ) std::setbuf( m_redirect_stderr, nullptr );
    }

    /*--- IOMonitor -------------------------------------------------*/
    iomonitor.init_connection();
}


/** @brief ロックファイル(FIFO)による排他処理を行う。
 *
 * @param[in] url 板やスレッドののURL、またはDATのファイルパス
 * @return ウインドウを表示せずプロセスを終了するときはfalseを返す
 */
bool App::setup_fifo( std::string url )
{
    // FIFOの状態をチェックする
    if( iomonitor.get_fifo_stat() == CORE::FIFO_OK )
    {
        // 引数にURLがある
        if( ! url.empty() )
        {
            // ローカルファイルかどうかチェック
            std::string url_real = CACHE::get_realpath( url );
            if( ! url_real.empty() ) url = std::move( url_real );

            // FIFOに書き込む
            iomonitor.send_command( url );

            // マルチモードでなく、メインプロセスでもない場合は終了
            if( ! multi_mode && ! iomonitor.is_main_process() ) return false;
        }
        // マルチモードではなく、メインプロセスでもない場合はメインプロセスのウインドウを最前面に表示して終了する
        // NOTE: "最前面に表示" はデスクトップ環境によって挙動が異なる可能性がある
        else if( ! multi_mode && ! iomonitor.is_main_process() )
        {
            // FIFOに書き込む
            iomonitor.send_command( CORE::kURL_WinMain );
            return false;
        }
    }
    // FIFOに問題がある(FATで作成出来ないなど)
    else
    {
        if( CONFIG::get_show_diag_fifo_error() )
        {
            Gtk::MessageDialog mdiag( CACHE::path_lock() + "の作成またはオープンに問題があります。このまま起動しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

            Gtk::CheckButton chk_button( "今後表示しない" );
            mdiag.get_content_area()->pack_start( chk_button, Gtk::PACK_SHRINK );
            chk_button.show();

            const int ret = mdiag.run();

            CONFIG::set_show_diag_fifo_error( ! chk_button.get_active() );

            if( ret != Gtk::RESPONSE_YES )
            {
                CONFIG::save_conf();
                return false;
            }
        }
    }
    return true;
}


/**
 * @brief 解析したコマンドラインオプションでウインドウを作成して Gtk::Application に追加する
 */
JDWinMain* App::create_window()
{
    assert( m_with_gui );
    auto win = new JDWinMain( init, skip_setupdiag, init_w, init_h, init_x, init_y );
    win->signal_hide().connect( sigc::mem_fun( *this, &App::slot_hide_window ) );
    win->show_all();
    Glib::RefPtr<Gtk::Application>::cast_dynamic( m_app )->add_window( *win );
    return win;
}


void App::slot_activate()
{
    if( ! m_with_gui ) return;

    if( ! setup_fifo( std::string{} ) ) return;

    // メインプロセスまたは多重起動のときはウインドウを作成して表示
    if( ! Win_Main ) {
        Win_Main = create_window();
    }
    Win_Main->present();
}


/** @brief URLやDATファイルを開く
 *
 * URLを複数渡して起動しても最初の一つのみ開く
 * @param[in] files URLやDATファイルのリスト
 */
void App::slot_open( const Gio::Application::type_vec_files& files, const Glib::ustring& )
{
    if( ! m_with_gui ) return;

    std::string url;
    for( auto& f : files ) {
        url = f->get_uri();
        if( ! url.empty() ) break;
    }

    if( ! setup_fifo( std::move( url ) ) ) return;

    // メインプロセスまたは多重起動のときはウインドウを作成して表示
    if( ! Win_Main ) {
        Win_Main = create_window();
    }
    Win_Main->present();
}


void App::slot_hide_window()
{
    // ウインドウを削除すると Gtk::Application から取り除かれる
    // 最後のウインドウが削除されるとアプリケーションは終了する
    delete Win_Main;
    Win_Main = nullptr;
}


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

    std::memset( &sigact, 0, sizeof(struct sigaction) );
    sigact.sa_handler = sig_handler;
    sigact.sa_mask = blockset;
    sigact.sa_flags = SA_RESETHAND;

    // シグナルハンドラ設定
    if( sigaction( SIGHUP, &sigact, nullptr ) != 0
        || sigaction( SIGINT, &sigact, nullptr ) != 0
        || sigaction( SIGQUIT, &sigact, nullptr ) != 0 ){
        std::cerr << "sigaction failed\n";
        std::exit( 1 );
    }
    // 接続が切れたソケットに書き込むと SIGPIPE が発生して強制終了するので無視するように設定する
    std::memset( &sigact, 0, sizeof(struct sigaction) );
    sigact.sa_handler = SIG_IGN;
    if( sigaction( SIGPIPE, &sigact, nullptr ) != 0 ) {
        std::cerr << "sigaction failed (SIGPIPE)\n";
        std::exit( 1 );
    }

    // XSMPによるセッション管理
#ifdef USE_XSMP

#ifdef _DEBUG
    std::cout << "USE_XSMP\n";
#endif

    XSMPDATA xsmpdata;
    std::memset( &xsmpdata, 0, sizeof(XSMPDATA) );
    xsmp_session_init( &xsmpdata );
#endif

    JDLIB::tlslib_init();

    App app;
    const int exit_status = app.run( argc, argv );

    JDLIB::tlslib_deinit();

#ifdef USE_XSMP
    xsmp_session_end( &xsmpdata );
#endif

    return exit_status;
}
