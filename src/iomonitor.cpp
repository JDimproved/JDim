// License: GPL2
//
// FIFOを使ったプロセス間通信を行うクラス
//

//#define _DEBUG
#include "jddebug.h"

#include "iomonitor.h"
#include "command.h"
#include "cache.h"
#include "jdlib/miscmsg.h"

#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <sys/stat.h>

using namespace CORE;

#define COMMAND_MAX_LENGTH 1024


/*-------------------------------------------------------------------*/
// コンストラクタ
/*-------------------------------------------------------------------*/
IOMonitor::IOMonitor()
  : m_fifo_fd( -1 ),
    m_fifo_file( CACHE::path_lock() ),
    m_iochannel( NULL ),
    m_main_process( false )
{
    init();
}


/*-------------------------------------------------------------------*/
// デストラクタ
/*-------------------------------------------------------------------*/
IOMonitor::~IOMonitor()
{
    if( m_iochannel ) m_iochannel->close(); // close( m_fifo_fd );

    // メインプロセスの終了時にFIFOを消去する
    if( m_main_process ) delete_fifo();
}


/*-------------------------------------------------------------------*/
// このクラスの初期化( FIFO作成、FIFOオープン、Glib::IOChannelの作成 )
/*-------------------------------------------------------------------*/
void IOMonitor::init()
{
    // 既にFIFOと同名のファイルが存在するか確認
    const int status = CACHE::file_exists( m_fifo_file );

    // 同名のファイルがFIFOでなければ削除する
    if( status != CACHE::EXIST_ERROR && status != CACHE::EXIST_FIFO )
    {
        delete_fifo();
    }

    // FIFOを作成
    int mkfifo_status = -1;

    do_makefifo:

    mkfifo_status = mkfifo( m_fifo_file.c_str(), O_RDWR | S_IRUSR | S_IWUSR );

    // FIFO作成でエラーになった( 基本的に既にメインプロセスがある )
    if( mkfifo_status != 0 )
    {
        // FIFOが存在しない
        if( errno != EEXIST )
        {
            MISC::ERRMSG( "IOMonitor::init(): fifo create failed." );

            return;
        }

        // FIFOを書き込み専用モードでオープン( ノンブロック )
        if( ( m_fifo_fd = open( m_fifo_file.c_str(), O_WRONLY | O_NONBLOCK ) ) == -1 )
        {
            // 反対側が既にオープンされていない( 異常終了などでメインプロセスがない )
            if( ( errno & ENXIO ) != 0 )
            {
                // 残っているFIFOを消す
                delete_fifo();

                // 最初からやり直す
                goto do_makefifo;
            }
            // その他のエラー
            else
            {
#ifdef _DEBUG
                std::cerr << "IOMonitor::init(): " << strerror( errno ) << std::endl;
#endif
            }
        }
    }
    // メインプロセス
    else
    {
        // FIFOを読み込み専用モードでオープン( ノンブロック )
        if( ( m_fifo_fd = open( m_fifo_file.c_str(), O_RDWR | O_NONBLOCK ) ) == -1 )
        {
            // エラーなのでFIFOを消す
            delete_fifo();

#ifdef _DEBUG
            std::cerr << "IOMonitor::init(): " << strerror( errno );
#endif // _DEBUG
        }

        // メインプロセスである
        m_main_process = true;

        // Glib::IOChannel
        Glib::signal_io().connect( sigc::mem_fun( this, &IOMonitor::slot_ioin ), m_fifo_fd, Glib::IO_IN );
        m_iochannel = Glib::IOChannel::create_from_fd( m_fifo_fd );
    }
}


/*-------------------------------------------------------------------*/
// FIFOを削除する
/*-------------------------------------------------------------------*/
void IOMonitor::delete_fifo()
{
    if( unlink( m_fifo_file.c_str() ) < 0 )
    {
        MISC::ERRMSG( "IOMonitor::init(): fifo unlink failed." );
    }
}


/*-------------------------------------------------------------------*/
// FIFOに書き込む
//
// 引数 1: 書き込む文字列
//
// 戻り値: 全て書き込まれたか否か
/*-------------------------------------------------------------------*/
bool IOMonitor::send_command( const char* command )
{
    if( ! command ) return false;

    const size_t command_length = strlen( command );

    // 異常に長かったら書き込まない
    if( command_length > COMMAND_MAX_LENGTH ) return false;

    g_assert( m_fifo_fd >= 0 );

    int status = -1;
    status = write( m_fifo_fd, command, command_length );

    return ( (size_t)status == command_length );
}


/*-------------------------------------------------------------------*/
// FIFOに書き込まれたら呼び出される( Glib::signal_io() )
//
// 引数 1: Glib::IOCondition
//
// 戻り値: true
/*-------------------------------------------------------------------*/
bool IOMonitor::slot_ioin( Glib::IOCondition io_condition )
{
    if( ( io_condition & ( Glib::IO_IN | Glib::IO_PRI ) ) == 0 )
    {
        MISC::ERRMSG( "IOMonitor::slot_ioin(): Invalid fifo response." );

        return false;
    }

    Glib::ustring buffer;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    Glib::IOStatus io_status;

    // 最大で COMMAND_MAX_LENGTH まで読み出す
    io_status = m_iochannel->read( buffer, COMMAND_MAX_LENGTH );

    if( io_status == Glib::IO_STATUS_ERROR )
    {
        MISC::ERRMSG( "IOMonitor::slot_ioin(): read error." );
    }

#else
    // GLIBMM_EXCEPTIONS_ENABLED が未定義の環境では未テストなので
    // 問題があるかも知れない
    std::auto_ptr< Glib::Error > ex;
    m_iochannel->read( buffer, COMMAND_MAX_LENGTH, ex );

    if( ex.get() )
    {
        MISC::ERRMSG( std::string( "IOMonitor::slot_ioin(): read_error. " ) + ex->what() );
    }

#endif // GLIBMM_EXCEPTIONS_ENABLED

#ifdef _DEBUG
    std::cout << "入力文字: " << buffer << std::endl;

    if( buffer == "Q" ) Gtk::Main::quit();
#endif // _DEBUG

    // FIFOに書き込まれたURLを開く
    // "現在のタブ/新しいタブ"など、開き方を選ぶ必要があるかも知れない
    //core_set_command( "open_article", buffer, "left", "auto" );
    core_set_command( "open_url", buffer );

    return true;
}

