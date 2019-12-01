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

#ifdef _WIN32
#define FIFO_TIMEOUT_MILI 100
#endif

using namespace CORE;

#define COMMAND_MAX_LENGTH 1024


/*-------------------------------------------------------------------*/
// コンストラクタ
/*-------------------------------------------------------------------*/
IOMonitor::IOMonitor()
#ifndef _WIN32
    : m_fifo_fd( -1 )
    , m_iochannel( nullptr )
#else
    : m_slot_hd( INVALID_HANDLE_VALUE )
#endif // _WIN32
    , m_fifo_file( CACHE::path_lock() )
    , m_fifo_stat( FIFO_OK )
    , m_main_process( false )
{
#ifdef _WIN32
    // create path to mailslot depends JD cache root
    std::string slot_tmp( CACHE::path_root() );
    for( int i=slot_tmp.length()-1; i>=0; i-- ){
        if( slot_tmp[ i ] == '/' || slot_tmp[ i ] == ':' ){
            slot_tmp[ i ] = '_';
        }
    }
    m_slot_name = "\\\\.\\mailslot\\" + slot_tmp;
#endif // _WIN32
    init();
}


/*-------------------------------------------------------------------*/
// デストラクタ
/*-------------------------------------------------------------------*/
IOMonitor::~IOMonitor()
{
#ifndef _WIN32
    if( m_iochannel ) m_iochannel->close(); // close( m_fifo_fd );

    // メインプロセスの終了時にFIFOを消去する
    if( m_main_process ) delete_fifo();
#else
    if( m_slot_hd != INVALID_HANDLE_VALUE ){
        CloseHandle( m_slot_hd );
        m_slot_hd = INVALID_HANDLE_VALUE;
    }
    if( m_main_process ){
        m_thread.join();
    }
#endif // _WIN32
}


/*-------------------------------------------------------------------*/
// このクラスの初期化( FIFO作成、FIFOオープン、Glib::IOChannelの作成 )
/*-------------------------------------------------------------------*/
void IOMonitor::init()
{
    // 既にFIFOと同名のファイルが存在するか確認
    const int status = CACHE::file_exists( m_fifo_file );

#ifndef _WIN32
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
        Glib::signal_io().connect( sigc::mem_fun( this, &IOMonitor::slot_ioin ), m_fifo_fd, Glib::IO_IN );
        m_iochannel = Glib::IOChannel::create_from_fd( m_fifo_fd );
    }
#else // _WIN32
    if( status != CACHE::EXIST_ERROR )
    {
        // if lockfile exists, jd don't suggest working because cache isn't broken
        MISC::ERRMSG( "IOMonitor::init(): exists other lockfile." );
        m_fifo_stat = FIFO_CREATE_ERROR;
    }

#ifdef _DEBUG
    std::cerr << "Create slot name: " << m_slot_name << std::endl;
#endif
    m_slot_hd = CreateMailslot( to_locale_cstr( m_slot_name.c_str() ),
            0, FIFO_TIMEOUT_MILI, nullptr );
    if( m_slot_hd == INVALID_HANDLE_VALUE ){
        if( GetLastError() == ERROR_ALREADY_EXISTS ){
            // client
            m_slot_hd = CreateFile( to_locale_cstr( m_slot_name.c_str() ),
                    GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, OPEN_EXISTING, 0, nullptr );
            if( m_slot_hd == INVALID_HANDLE_VALUE ){
#ifdef _DEBUG
                std::cerr << "Open err: " << GetLastError() << std::endl;
#endif
                MISC::ERRMSG( "IOMonitor::init(): Open client failed." );
                m_fifo_stat = FIFO_OPEN_ERROR;
            }
        } else {
#ifdef _DEBUG
            std::cerr << "Create err: " << GetLastError() << std::endl;
#endif
            MISC::ERRMSG( "IOMonitor::init(): Create slot failed." );
            m_fifo_stat = FIFO_CREATE_ERROR;
        }
    } else {
        // server
        if( ! m_thread.create( monitor_launcher, ( void* )this, JDLIB::NODETACH ) ) {
            MISC::ERRMSG( "IOMonitor::init(): Could not start thread." );
            m_fifo_stat = FIFO_CREATE_ERROR;
            return;
        }
        m_main_process = true;
    }
#endif // _WIN32
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
bool IOMonitor::send_command( const char* command )
{
    if( ! command ) return false;

    const size_t command_length = strlen( command );

    // 異常に長かったら書き込まない
    if( command_length > COMMAND_MAX_LENGTH ) return false;

#ifndef _WIN32
    g_assert( m_fifo_fd >= 0 );

    int status = -1;
    status = write( m_fifo_fd, command, command_length );

    return ( (size_t)status == command_length );
#else
    g_assert( m_slot_hd != INVALID_HANDLE_VALUE );

    DWORD length = 0;
    BOOL rc = WriteFile( m_slot_hd, command, command_length, &length, nullptr );

    return rc != FALSE && ( size_t )length == command_length;
#endif // _WIN32
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
#ifndef _WIN32
    // 最大で COMMAND_MAX_LENGTH まで読み出す
    Glib::IOStatus io_status = m_iochannel->read( buffer, COMMAND_MAX_LENGTH );
    if( io_status == Glib::IO_STATUS_ERROR )
    {
        MISC::ERRMSG( "IOMonitor::slot_ioin(): read error." );
    }
#else
    char msg[ COMMAND_MAX_LENGTH ];
    memset( msg, 0, sizeof( msg ) );
    DWORD length = 0;

    BOOL rc = ReadFile( m_slot_hd, msg, COMMAND_MAX_LENGTH - 1, &length, nullptr );
    if( rc == FALSE ){
        DWORD error = GetLastError();
        if( error == ERROR_SEM_TIMEOUT ){
            return true;
        }
#ifdef _DEBUG
        std::cout << "read err: " << error << std::endl;
#endif
        MISC::ERRMSG( "IOMonitor::slot_ioin(): read error." );
        return false;
    }
    if( length == 0 ){
        return true;
    }
    buffer += msg;
#endif // _WIN32

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


#ifdef _WIN32
/*-------------------------------------------------------------------*/
// monitor_launcher
/*-------------------------------------------------------------------*/
// static
void* IOMonitor::monitor_launcher( void* dat )
{
    CORE::IOMonitor* iom = ( CORE::IOMonitor* )dat;
    while( iom->m_slot_hd != INVALID_HANDLE_VALUE ){
        if( ! iom->slot_ioin( Glib::IO_IN ) ){
            break;
        }
    }
    return 0;
}
#endif // _WIN32
