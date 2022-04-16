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

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


using namespace CORE;

#define COMMAND_MAX_LENGTH 1024


/*-------------------------------------------------------------------*/
// コンストラクタ
/*-------------------------------------------------------------------*/
IOMonitor::IOMonitor()
    : m_fifo_fd( -1 )
    , m_fifo_file( CACHE::path_lock() )
    , m_fifo_stat( FIFO_OK )
{
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
void IOMonitor::init_connection()
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
        // FIFOが存在する
        if( errno == EEXIST )
        {
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
#ifdef _DEBUG
                std::cerr << "IOMonitor::init(): " << strerror( errno ) << std::endl;
#endif
                m_fifo_stat = FIFO_OPEN_ERROR;
            }
        }
        // 何らかの問題で作成出来なかった
        else
        {
            MISC::ERRMSG( "IOMonitor::init(): fifo create failed." );
            m_fifo_stat = FIFO_CREATE_ERROR;
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
            m_fifo_stat = FIFO_OPEN_ERROR;
            return;
        }

        // メインプロセスである
        m_main_process = true;

        // Glib::IOChannel
        Glib::signal_io().connect( sigc::mem_fun( *this, &IOMonitor::slot_ioin ), m_fifo_fd, Glib::IO_IN );
        m_iochannel = Glib::IOChannel::create_from_fd( m_fifo_fd );
    }
}


/*-------------------------------------------------------------------*/
// FIFOを削除する
/*-------------------------------------------------------------------*/
void IOMonitor::delete_fifo()
{
    int del_stat = 0;

    if( ( del_stat = unlink( m_fifo_file.c_str() ) ) < 0 )
    {
        MISC::ERRMSG( "IOMonitor::init(): fifo unlink failed." );
    }

    g_assert( del_stat >= 0 );
}


/*-------------------------------------------------------------------*/
// FIFOに書き込む
//
// 引数 1: 書き込む文字列
//
// 戻り値: 全て書き込まれたか否か
/*-------------------------------------------------------------------*/
bool IOMonitor::send_command( const std::string& command )
{
    // 異常に長かったら書き込まない
    if( command.size() > COMMAND_MAX_LENGTH ) return false;

    g_assert( m_fifo_fd >= 0 );

    const ssize_t status = write( m_fifo_fd, command.c_str(), command.size() );
    return static_cast<std::size_t>( status ) == command.size();
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
    // 最大で COMMAND_MAX_LENGTH まで読み出す
    Glib::IOStatus io_status = m_iochannel->read( buffer, COMMAND_MAX_LENGTH );
    if( io_status == Glib::IO_STATUS_ERROR )
    {
        MISC::ERRMSG( "IOMonitor::slot_ioin(): read error." );
    }

    // FIFOに書き込まれたURLを開く
    // "現在のタブ/新しいタブ"など、開き方を選ぶ必要があるかも知れない
    //core_set_command( "open_article", buffer, "left", "auto" );
    core_set_command( "open_url", buffer );

    return true;
}
