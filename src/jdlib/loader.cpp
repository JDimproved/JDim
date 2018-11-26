// ライセンス: GPL2

#ifdef _WIN32
// require Windows XP or Server 2003 for getaddrinfo
#define WINVER 0x0501
#endif

//#define _DEBUG
//#define _DEBUG_CHUNKED
//#define _DEBUG_TIME
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "loader.h"
#include "miscmsg.h"
#include "miscutil.h"
#include "ssl.h"

#ifdef _DEBUG_TIME
#include "misctime.h"
#endif

#include "config/globalconf.h"

#include "skeleton/loadable.h"

#include "httpcode.h"

#include <sstream>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#ifdef _WIN32
#include <process.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
#include <signal.h>

#include <glibmm.h>

#include "jdmutex.h"

#ifdef _WIN32
// _soc : SOCKET (unsigned int)
#define SOC_ISVALID(_soc) ( (_soc) != INVALID_SOCKET )
#else
// _soc : int
#define SOC_ISVALID(_soc) ( (_soc) >= 0 )
#endif

enum
{
    MAX_LOADER = 10, // 最大スレッド数
    MAX_LOADER_SAMEHOST = 2, // 同一ホストに対して実行できる最大スレッド数
    LNG_BUF_MIN = 1 * 1024, // 読み込みバッファの最小値 (byte)
    TIMEOUT_MIN = 1 // タイムアウトの最小値 (秒)
};


#ifdef _WIN32
bool initialized_loader = false;
#endif


//
// トークンとスレッド起動待ちキュー
//
// 起動しているローダが MAX_LOADER 個を越えたらローダをスレッド待ちキューに入れる
//

namespace JDLIB
{
    bool get_token( JDLIB::Loader* loader );
    void return_token( JDLIB::Loader* loader );

    void push_loader_queue( JDLIB::Loader* loader );
    bool remove_loader_queue( JDLIB::Loader* loader );
    void pop_loader_queue();
}


static JDLIB::StaticMutex mutex_token = JDLIB_STATIC_MUTEX_INIT;
static JDLIB::StaticMutex mutex_queue = JDLIB_STATIC_MUTEX_INIT;
std::list< JDLIB::Loader* > queue_loader; // スレッド起動待ちの Loader のキュー
int token_loader = 0;
std::vector< JDLIB::Loader* > vec_loader( MAX_LOADER );
bool disable_pop = false;


// トークン取得
bool JDLIB::get_token( JDLIB::Loader* loader )
{
    JDLIB::LockGuard lock( mutex_token );

#ifdef _DEBUG
    std::cout << "JDLIB::get_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif

    if( token_loader >= MAX_LOADER ) return false;

    int count = 0;
    std::vector< JDLIB::Loader* >::iterator it = vec_loader.begin();
    for( ; it != vec_loader.end(); ++it ) if( ( *it ) && ( *it )->data().host == loader->data().host ) ++count;
#ifdef _DEBUG
    std::cout << "count = " << count << std::endl;
#endif

    const int max_loader = MIN( MAX_LOADER_SAMEHOST, MAX( 1, CONFIG::get_connection_num() ) );
    if( count >= max_loader ) return false;

#ifdef _DEBUG
    std::cout << "got token\n";
#endif

    ++token_loader;

    it = vec_loader.begin();
    for( ; it != vec_loader.end(); ++it ) if( ! ( *it ) ){ ( *it ) = loader; break; }

    return true;
}


//　トークン返す
void JDLIB::return_token( JDLIB::Loader* loader )
{
    JDLIB::LockGuard lock( mutex_token );

    --token_loader;
    assert( token_loader >= 0 );

    std::vector< JDLIB::Loader* >::iterator it = vec_loader.begin();
    for( ; it != vec_loader.end(); ++it ) if( ( *it ) == loader ) ( *it ) = NULL;

#ifdef _DEBUG
    std::cout << "JDLIB::return_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif
}


// スレッド起動待ちキューに Loader を登録
void JDLIB::push_loader_queue( JDLIB::Loader* loader )
{
    JDLIB::LockGuard lock( mutex_queue );

    if( ! loader ) return;

    if( loader->get_low_priority() ) queue_loader.push_back( loader );
    else{

        std::list< JDLIB::Loader* >::iterator pos = queue_loader.begin();
        for( ; pos != queue_loader.end(); ++pos ) if( ( *pos )->get_low_priority() ) break;
        queue_loader.insert( pos, loader );
    }

#ifdef _DEBUG
    std::cout << "JDLIB::push_loader_queue url = " << loader->data().url << " size = " << queue_loader.size() << std::endl;
#endif    
}


// キューから Loader を取り除いたらtrueを返す
bool JDLIB::remove_loader_queue( JDLIB::Loader* loader )
{
    JDLIB::LockGuard lock( mutex_queue );

    if( ! queue_loader.size() ) return false;
    if( std::find( queue_loader.begin(), queue_loader.end(), loader ) == queue_loader.end() ) return false;

    queue_loader.remove( loader );

#ifdef _DEBUG
    std::cout << "JDLIB::remove_loader_queue url = " << loader->data().url << " size = " << queue_loader.size() << std::endl;
#endif    

    return true;
}


// キューに登録されたスレッドを起動する
void JDLIB::pop_loader_queue()
{
    JDLIB::LockGuard lock( mutex_queue );

    if( disable_pop ) return;
    if( ! queue_loader.size() ) return;

#ifdef _DEBUG
    std::cout << "JDLIB::pop_loader_queue size = " << queue_loader.size() << std::endl;
#endif    

    std::list< JDLIB::Loader* >::iterator it = queue_loader.begin();
    for( ; it != queue_loader.end(); ++it ) if( JDLIB::get_token( *it ) ) break;
    if( it == queue_loader.end() ) return;

    JDLIB::Loader* loader = *it;
    queue_loader.remove( loader );

#ifdef _DEBUG
    std::cout << "pop " << loader->data().url << std::endl;
#endif

    loader->create_thread();
}


//
// ローダの起動待ちキューにあるスレッドを実行しない
// 
// アプリ終了時にこの関数を呼び出さないとキューに登録されたスレッドが起動してしまうので注意
//
void JDLIB::disable_pop_loader_queue()
{
    JDLIB::LockGuard lock( mutex_queue );

#ifdef _DEBUG
    std::cout << "JDLIB::disable_pop_loader_queue\n";
#endif    

    disable_pop = true;
}


//
// mainの最後でローダが動いていないかチェックする関数
//
void JDLIB::check_loader_alive()
{
#ifdef _DEBUG
    std::cout << "JDLIB::check_loader_alive loader = " << token_loader
              << " queue = " << queue_loader.size() << std::endl;
#endif    

    if( token_loader ){
        MISC::ERRMSG( "loaders are still moving." );
        assert( false );
    }

    if( queue_loader.size() ){
        MISC::ERRMSG( "queue of loaders are not empty." );
        assert( false );
    }

#ifdef _WIN32
    if ( initialized_loader ){
        WSACleanup();
    }
#endif
}



///////////////////////////////////////////////////

using namespace JDLIB;


//
// low_priority = true の時はスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
//
Loader::Loader( const bool low_priority )
    : m_addrinfo( 0 ),
      m_stop( false ),
      m_loading( false ),
      m_low_priority( low_priority ),
      m_buf( 0 ),
      m_buf_zlib_in ( 0 ),
      m_buf_zlib_out ( 0 ),
      m_use_zlib ( 0 )
{
#ifdef _DEBUG
    std::cout << "Loader::Loader : loader was created.\n";
#endif

#ifdef _WIN32
    if ( !initialized_loader ){
        WSADATA wsaData;
        if ( WSAStartup(MAKEWORD(2,0), &wsaData) != 0 ){
            MISC::ERRMSG( "could not startup winsock2" );
        }
        initialized_loader = true;
    }
#endif

    clear();
}


Loader::~Loader()
{
#ifdef _DEBUG
    std::cout << "Loader::~Loader : url = " << m_data.url << std::endl;
#endif

    clear();

//    assert( ! m_loading );
}


void Loader::clear()
{
#ifdef _DEBUG
    std::cout << "Loader::clear\n";
#endif

    stop();
    wait();

    m_loadable = NULL;
    
    m_use_chunk = false;

    if( m_buf ) free( m_buf );
    m_buf = NULL;

    if( m_buf_zlib_in ) free( m_buf_zlib_in );
    m_buf_zlib_in = NULL;

    if( m_buf_zlib_out ) free( m_buf_zlib_out );
    m_buf_zlib_out = NULL;
    
    if( m_use_zlib ) inflateEnd( &m_zstream );
    m_use_zlib = false;
}


void Loader::wait()
{
    m_thread.join();
}


void Loader::stop()
{
    if( ! m_loading ) return;

#ifdef _DEBUG
    std::cout << "Loader::stop : url = " << m_data.url << std::endl;
#endif

    m_stop = true;

    // スレッド起動待ち状態の時は SKELETON::Loadable にメッセージを送る
    if( JDLIB::remove_loader_queue( this ) ){

        m_data.code = HTTP_TIMEOUT;
        m_data.modified = std::string();
        m_data.str_code = "stop loading";
        finish_loading();
    }
}


//
// ダウンロード開始
//
// data_in でセットする必要がある項目 ( url は必須 )
//
// url
// head ( true なら HEAD 送信 )
// port ( == 0 ならプロトコルを見て自動認識 )
// str_post( != empty なら POST する。UTF-8であること )
// modified
// byte_readfrom ( != 0 ならその位置からレジューム)
// agent
// referer
// cookie_for_write
// host_proxy ( != empty ならproxy使用 )
// port_proxy ( == 0 なら 8080 )
// basicauth_proxy
// size_buf ( バッファサイズ, k 単位で指定。 == 0 ならデフォルトサイズ(LNG_BUF_MIN)使用)
// timeout ( タイムアウト秒。==0 ならデフォルト( TIMEOUT )使用  )
// basicauth
//
bool Loader::run( SKELETON::Loadable* cb, const LOADERDATA& data_in )
{
    assert( ! data_in.url.empty() );

#ifdef _DEBUG
    std::cout << "Loader::run : url = " << data_in.url << std::endl;
#endif

    if( m_loading ){
        MISC::ERRMSG( "now loading : " + data_in.url );
        return false;
    }

    clear();
    m_loadable = cb;
    m_data.init();
    m_stop = false;
    
    // バッファサイズ設定
    m_data.size_buf = data_in.size_buf;
    m_lng_buf = MAX( LNG_BUF_MIN, m_data.size_buf * 1024 );
    m_lng_buf_zlib_in = m_lng_buf * 2;
    m_lng_buf_zlib_out = m_lng_buf * 10; // 小さいとパフォーマンスが落ちるので少し多めに10倍位
    
    // protocol と host と path 取得
    m_data.url = data_in.url;
    size_t i = m_data.url.find( "://", 0 );  // "http://" とつけるのは呼び出し側の責任で
    if( i == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could nod get protocol : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    i += 3;
    m_data.protocol = data_in.url.substr( 0, i );

    size_t i2 = m_data.url.find( "/", i );
    if( i2 == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could not get hostname and path : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    
    m_data.host = m_data.url.substr( i, i2 - i );
    i = i2; m_data.path = m_data.url.substr( i2 );

    // ポートセット

    // ホスト名の後に指定されている
    if( ( i = m_data.host.find( ":" ) ) != std::string::npos ){
        m_data.port = atoi( m_data.host.substr( i+1 ).c_str() );
        m_data.host = m_data.host.substr( 0, i );
    }

    // 明示的に指定
    else if( data_in.port != 0 ) m_data.port = data_in.port;

    // プロトコルを見て自動決定
    else{

        // http
        if( m_data.protocol.find( "http://" ) != std::string::npos )
            m_data.port = data_in.use_ssl ? 443 : 80;

        // https
        else if( m_data.protocol.find( "https://" ) != std::string::npos ){
            m_data.port = 443;
        }

        // その他
        else{

            m_data.code = HTTP_ERR;
            m_data.str_code = "unknown protocol : " + m_data.url;
            MISC::ERRMSG( m_data.str_code );
            return false;
        }
    }

    // ssl使用指定
    // HACK: don't use SSL to access 2ch via a scraping proxy
    if( data_in.use_ssl
        || ( m_data.protocol.find( "https://" ) != std::string::npos
             && m_data.host.find( ".2ch.net" ) == std::string::npos
             && m_data.host.find( ".5ch.net" ) == std::string::npos
             && m_data.host.find( ".bbspink.com" ) == std::string::npos
           )
      ){
        m_data.use_ssl = true;
        m_data.async = false;
    }

    // プロキシ
    m_data.host_proxy = data_in.host_proxy;

    // 先頭に *tp:// が付いていたら取り除く
    if( ! m_data.host_proxy.empty() && m_data.host_proxy.find( "tp://" ) != std::string::npos ){
        const bool protocol = false;
        m_data.host_proxy = MISC::get_hostname( m_data.host_proxy , protocol );
    }
    if( ! m_data.host_proxy.empty() ){
        m_data.port_proxy = data_in.port_proxy;
        if( m_data.port_proxy == 0 ) m_data.port_proxy = 8080;
        m_data.basicauth_proxy = data_in.basicauth_proxy;
    }

    // その他
    m_data.head = data_in.head;
    m_data.str_post = data_in.str_post;
    m_data.modified = data_in.modified;
    m_data.byte_readfrom = data_in.byte_readfrom;    
    m_data.contenttype = data_in.contenttype;
    m_data.agent = data_in.agent;
    m_data.referer = data_in.referer;
    m_data.cookie_for_write = data_in.cookie_for_write;
    m_data.timeout = MAX( TIMEOUT_MIN, data_in.timeout );
    m_data.ex_field = data_in.ex_field;
    m_data.basicauth = data_in.basicauth;
    m_data.use_ipv6 = data_in.use_ipv6;

#ifdef _DEBUG    
    std::cout << "host: " << m_data.host << std::endl;
    std::cout << "protocol: " << m_data.protocol << std::endl;
    std::cout << "path: " << m_data.path << std::endl;
    std::cout << "port: " << m_data.port << std::endl;
    std::cout << "modified: " << m_data.modified << std::endl;
    std::cout << "byte_readfrom: " << m_data.byte_readfrom << std::endl;
    std::cout << "contenttype: " << m_data.contenttype << std::endl;
    std::cout << "agent: " << m_data.agent << std::endl;
    std::cout << "referer: " << m_data.referer << std::endl;
    std::cout << "cookie: " << m_data.cookie_for_write << std::endl;
    std::cout << "proxy: " << m_data.host_proxy << std::endl;
    std::cout << "port of proxy: " << m_data.port_proxy << std::endl;
    std::cout << "proxy basicauth : " << m_data.basicauth_proxy << std::endl;
    std::cout << "buffer size: " << m_lng_buf / 1024 << " Kb" << std::endl;
    std::cout << "timeout : " << m_data.timeout << " sec" << std::endl;
    std::cout << "ex_field : " << m_data.ex_field << std::endl;
    std::cout << "basicauth : " << m_data.basicauth << std::endl;
    std::cout << "\n";
#endif

    m_loading = true;

    // トークンを取得出来なかったら、他のスレッドが終了した時に
    // 改めて create_thread() を呼び出す
    if( get_token( this ) ) create_thread();
    else JDLIB::push_loader_queue( this );

    return true;
}


//
// スレッド起動
//
void Loader::create_thread()
{
#ifdef _DEBUG
    std::cout << "Loader::create_thread :  url = " << m_data.url << std::endl;
#endif

    if( ! m_thread.create( ( STARTFUNC ) launcher, ( void * ) this, JDLIB::NODETACH ) ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "Loader::run : could not start thread";
        MISC::ERRMSG( m_data.str_code );
        finish_loading();
        return;
    }
}


//
// スレッドのランチャ (static)
//
void* Loader::launcher( void* dat )
{
    Loader* tt = ( Loader * ) dat;
    tt->run_main();
    return 0;
}


bool Loader::send_connect( const int soc, std::string& errmsg )
{
    std::string authority;
    std::string msg_send;

    authority = m_data.host + ":" + std::to_string( m_data.port );
    msg_send = "CONNECT " + authority + " HTTP/1.1\r\nHost: " + authority + "\r\n\r\n";
    size_t send_size = strlen( msg_send.data() );
    while( send_size > 0 && !m_stop ){
        if( ! wait_recv_send( soc, false ) ){
            m_data.code = HTTP_TIMEOUT;
            errmsg = "send timeout";
            return false;
        }

#ifdef _WIN32
        ssize_t tmpsize = send( soc, msg_send.data(), send_size,0);
        int lastError = WSAGetLastError();
#else
#ifdef MSG_NOSIGNAL
        ssize_t tmpsize = send( soc, msg_send.data(), send_size, MSG_NOSIGNAL );
#else
        // SolarisにはMSG_NOSIGNALが無いのでSIGPIPEをIGNOREする (FreeBSD4.11Rにもなかった)
        signal( SIGPIPE , SIG_IGN ); /* シグナルを無視する */
        ssize_t tmpsize = send( soc, msg_send.data(), send_size,0);
        signal(SIGPIPE,SIG_DFL); /* 念のため戻す */
#endif // MSG_NOSIGNAL
#endif // _WIN32

#ifdef _WIN32
        if( tmpsize == 0
            || ( tmpsize < 0 && !( lastError == WSAEWOULDBLOCK || errno == WSAEINTR ) ) ){
#else
        if( tmpsize == 0
            || ( tmpsize < 0 && !( errno == EWOULDBLOCK || errno == EINTR ) ) ){
#endif

            m_data.code = HTTP_ERR;
            errmsg = "send failed : " + m_data.url;
            return false;
        }

        if( tmpsize > 0 ) send_size -= tmpsize;
    }

    char rbuf[256];
    size_t read_size = 0;
    while( read_size < sizeof(rbuf) && !m_stop ){

        ssize_t tmpsize;

        if( !wait_recv_send( soc, true ) ){
            m_data.code = HTTP_TIMEOUT;
            errmsg = "CONNECT: read timeout in";
            return false;
        }

        tmpsize = recv( soc, rbuf + read_size, sizeof(rbuf) - read_size, 0 );
        if( tmpsize < 0 && errno != EINTR ){
            m_data.code = HTTP_ERR;
            errmsg = "CONNECT: recv() failed";
            return false;
        }

        if( tmpsize == 0 ) break;
        if( tmpsize > 0 ){
            read_size += tmpsize;

            const int ret = receive_header( rbuf, read_size );
            if( ret == HTTP_ERR ){

                m_data.code = HTTP_ERR;
                errmsg = "CONNECT: invalid header : " + m_data.url;
                return false;
            }
            else if( ret == HTTP_OK ) return true;
        }
    }
    return false;
}

//
// 実際の処理部
//
void Loader::run_main()
{
    // エラーメッセージ
    std::string errmsg;

#ifdef _WIN32
    SOCKET soc = INVALID_SOCKET; // ソケットID
#else
    int soc = -1; // ソケットID
#endif
    bool use_proxy = ( ! m_data.host_proxy.empty() );

    JDLIB::JDSSL* ssl = NULL;
    
    // 送信メッセージ作成
    const std::string msg_send = create_msg_send();
    
#ifdef _DEBUG    
    std::cout << "Loader::run_main : start loading thread : " << m_data.url << std::endl;;
    if( use_proxy ) std::cout << "use_proxy : " << m_data.host_proxy << std::endl;
    std::cout <<"send :----------\n" <<  msg_send << "\n---------------\n";
#endif

    // addrinfo 取得
    if( m_data.host_proxy.empty() ){
        m_addrinfo = get_addrinfo( m_data.host, m_data.port );
        if( ! m_addrinfo ){
            m_data.code = HTTP_ERR;
            errmsg = "getaddrinfo failed : " + m_data.url;
            goto EXIT_LOADING;
        }
    }
    else{
        m_addrinfo = get_addrinfo( m_data.host_proxy, m_data.port_proxy );
        if( ! m_addrinfo ){
            m_data.code = HTTP_ERR;
            errmsg = "getaddrinfo failed : " + m_data.host_proxy;
            goto EXIT_LOADING;
        }
    }

    // ソケット作成
    soc = socket( m_addrinfo ->ai_family, m_addrinfo ->ai_socktype, m_addrinfo ->ai_protocol );
    if( ! SOC_ISVALID( soc ) ){
        m_data.code = HTTP_ERR;
        errmsg = "socket failed : " + m_data.url;
        goto EXIT_LOADING;
    }

    // ソケットを非同期に設定
    if( m_data.async ){
#ifdef _WIN32
        u_long flags = 0;
        if ( ioctlsocket( soc, FIONBIO, &flags) != 0 ){
#else
        int flags;
        flags = fcntl( soc, F_GETFL, 0);
        if( flags == -1 || fcntl( soc, F_SETFL, flags | O_NONBLOCK ) < 0 ){
#endif
            m_data.code = HTTP_ERR;
            errmsg = "fcntl failed";
            goto EXIT_LOADING;
        }
    }
    
    // サーバにconnect
    int ret;
    ret = connect( soc, m_addrinfo ->ai_addr, m_addrinfo ->ai_addrlen );
    if( ret != 0 ){

        // ノンブロックでまだ接続中
#ifdef _WIN32
        if ( !( m_data.async && WSAGetLastError() == WSAEWOULDBLOCK ) ){
#else
        if ( !( m_data.async && errno == EINPROGRESS ) ){
#endif

            m_data.code = HTTP_ERR;
            if( ! use_proxy ) errmsg = "connect failed : " + m_data.host;
            else errmsg = "connect failed : " + m_data.host_proxy;
            goto EXIT_LOADING;
        }
    }

    // connect待ち
    if( m_data.async ){

        if( ! wait_recv_send( soc, false ) ){

            // タイムアウト
            m_data.code = HTTP_TIMEOUT;
            errmsg = "connect timeout";
            goto EXIT_LOADING;
        }

        // connectが成功したかチェック
        int optval;
        socklen_t optlen = sizeof( int );
#ifdef _WIN32
        if( getsockopt( soc, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen ) != 0 ){
#else
        if( getsockopt( soc, SOL_SOCKET, SO_ERROR, (void *)&optval, &optlen ) < 0 ){
#endif
            m_data.code = HTTP_ERR;
            errmsg = "getsockopt failed";
            goto EXIT_LOADING;
        }

        if( optval != 0 ){
            m_data.code = HTTP_ERR;
            errmsg = "connect(getsockopt) failed";
            goto EXIT_LOADING;
        }

#ifdef _DEBUG
        std::cout << "connect ok\n";
#endif
    }

    // ssl 初期化とコネクト
    if( m_data.use_ssl ){

        if ( use_proxy ) {
            if ( ! send_connect( soc, errmsg ) )
                goto EXIT_LOADING;
        }
        ssl = new JDLIB::JDSSL();
        if( ! ssl->connect( soc, m_data.host.c_str() ) ){
            m_data.code = HTTP_ERR;
            errmsg = ssl->get_errmsg() + " : " + m_data.url;
            goto EXIT_LOADING;
        }
    }

    // SEND 又は POST

    // 通常
    if( !ssl ){

        size_t send_size = strlen( msg_send.data() );
        while( send_size > 0 && !m_stop ){

            // writefds 待ち
            if( ! wait_recv_send( soc, false ) ){

                // タイムアウト
                m_data.code = HTTP_TIMEOUT;         
                errmsg = "send timeout";
                goto EXIT_LOADING;
            }

            // SEND 又は POST
#ifdef _WIN32
            ssize_t tmpsize = send( soc, msg_send.data(), send_size,0);
            int lastError = WSAGetLastError();
#else
#ifdef MSG_NOSIGNAL
            ssize_t tmpsize = send( soc, msg_send.data(), send_size, MSG_NOSIGNAL );
#else
            // SolarisにはMSG_NOSIGNALが無いのでSIGPIPEをIGNOREする (FreeBSD4.11Rにもなかった)
            signal( SIGPIPE , SIG_IGN ); /* シグナルを無視する */
            ssize_t tmpsize = send( soc, msg_send.data(), send_size,0);
            signal(SIGPIPE,SIG_DFL); /* 念のため戻す */
#endif // MSG_NOSIGNAL
#endif // _WIN32

#ifdef _WIN32
            if( tmpsize == 0
                || ( tmpsize < 0 && !( lastError == WSAEWOULDBLOCK || errno == WSAEINTR ) ) ){
#else
            if( tmpsize == 0
                || ( tmpsize < 0 && !( errno == EWOULDBLOCK || errno == EINTR ) ) ){
#endif

                m_data.code = HTTP_ERR;
                errmsg = "send failed : " + m_data.url;
                goto EXIT_LOADING;
            }

            if( tmpsize > 0 ) send_size -= tmpsize;
        }

        if( m_stop ) goto EXIT_LOADING;

#ifdef _DEBUG
        std::cout << "send ok\n";
#endif
    }

    // SSL使用
    else{ 

        if( ssl->write( msg_send.data(), strlen( msg_send.data() ) ) < 0 ){

            m_data.code = HTTP_ERR;
            errmsg = ssl->get_errmsg() + " : " + m_data.url;
            goto EXIT_LOADING;
        }
    }

    // 受信用バッファを作ってメッセージ受信
    size_t mrg;
    mrg = 64; // 一応オーバーフロー避けのおまじない
    assert( m_buf == NULL );
    m_buf = ( char* )malloc( m_lng_buf + mrg );

    bool receiving_header;

#ifdef _DEBUG_TIME
    MISC::start_measurement( 1 );
#endif

    // 受信開始
    receiving_header = true;
    m_data.length_current = 0;
    m_data.size_data = 0;    
    do{
        // 読み込み
        size_t read_size = 0;
        while( read_size < m_lng_buf - mrg && !m_stop ){

            ssize_t tmpsize;

            // 通常
            if( !ssl ){

#ifdef _DEBUG_TIME
                MISC::start_measurement( 0 );
#endif

                // readfds 待ち
                if( !wait_recv_send( soc, true ) ){
                    // タイムアウト
                    m_data.code = HTTP_TIMEOUT;         
                    errmsg = "read timeout";
                    goto EXIT_LOADING;
                }

                tmpsize = recv( soc, m_buf + read_size, m_lng_buf - read_size - mrg, 0 );
                if( tmpsize < 0 && errno != EINTR ){
                    m_data.code = HTTP_ERR;         
                    errmsg = "recv() failed";
                    goto EXIT_LOADING;
                }

#ifdef _DEBUG_TIME
                std::cout << "size = " << tmpsize << " time = " << MISC::measurement( 0 ) << std::endl;
#endif
            }

            // SSL
            else{

                tmpsize = ssl->read(  m_buf + read_size, m_lng_buf - read_size - mrg );
                if( tmpsize < 0 ){
                    m_data.code = HTTP_ERR;         
                    errmsg = ssl->get_errmsg() + " : " + m_data.url;
                    goto EXIT_LOADING;
                }
            }

            if( tmpsize == 0 ) break;
            if( tmpsize > 0 ){

                read_size += tmpsize;

                // ヘッダ取得
                if( receiving_header ){

                    const int ret = receive_header( m_buf, read_size );
                    if( ret == HTTP_ERR ){

                        m_data.code = HTTP_ERR;
                        errmsg = "invalid header : " + m_data.url;
                        goto EXIT_LOADING;
                    }
                    else if( ret == HTTP_OK ) receiving_header = false;
                }

                if( m_data.length && m_data.length <= m_data.length_current + read_size ) break;
            }

        }

        m_buf[ read_size ] = '\0';

        // 停止指定
        if( m_stop ) break;

        // サーバ側がcloseした
        if( read_size == 0 ){

            // ヘッダを取得する前にcloseしたらエラー
            if( receiving_header && m_data.size_data == 0 ){
                m_data.code = HTTP_ERR;         
                errmsg = "no data";
                goto EXIT_LOADING;
            }

            // コード304等の場合は終了
            break;
        }

        // ヘッダ取得失敗
        if( receiving_header ){
            m_data.code = HTTP_ERR;         
            errmsg = "no http header";
            goto EXIT_LOADING;
        }

        m_data.length_current += read_size;
        
        //  chunkedな場合
        if( m_use_chunk ){
            
            if( !skip_chunk( m_buf, read_size ) ){

                m_data.code = HTTP_ERR;
                errmsg = "skip_chunk() failed : " + m_data.url;
                goto EXIT_LOADING;
            }
            if( ! read_size ) break;
        }

        // 圧縮されていない時はそのままコールバック呼び出し
        if( !m_use_zlib ) {

            m_data.size_data += read_size;

            // コールバック呼び出し
            if( m_loadable ) m_loadable->receive( m_buf, read_size );
        }
        
        // 圧縮されているときは unzip してからコールバック呼び出し
        else if( !unzip( m_buf, read_size ) ){
            
            m_data.code = HTTP_ERR;
            errmsg = "unzip() failed : " + m_data.url;
            goto EXIT_LOADING;
        }

        if( m_data.length && m_data.length <= m_data.length_current ) break;
        
    } while( !m_stop );

#ifdef _DEBUG_TIME
    std::cout << "loadingl time = " << MISC::measurement( 1 ) << std::endl;
#endif

    // 終了処理
EXIT_LOADING:

    // ssl クローズ
    if( ssl ){
        ssl->close();
        delete ssl;
        ssl = NULL;
    }

    if( SOC_ISVALID( soc ) ){

        // writefds待ち
        // 待たないとclose()したときにfinパケットが消える？
        if( ! wait_recv_send( soc, false ) ){

            // タイムアウト
            m_data.code = HTTP_TIMEOUT;         
            errmsg = "send timeout";
        }

        // 送信禁止
#ifdef _WIN32
        shutdown( soc, SD_SEND );
#else
        shutdown( soc, SHUT_WR );
#endif
    }

    // 強制停止した場合
    if( m_stop ){
#ifdef _DEBUG
        std::cout << "Loader::run_main : stop loading\n";
#endif
        m_data.code = HTTP_TIMEOUT;
        m_data.modified = std::string();
        m_data.str_code = "stop loading";
    }
    // エラーあり
    else if( ! errmsg.empty() ){
        m_data.modified = std::string();
        MISC::ERRMSG( errmsg );
        m_data.str_code = errmsg;
    }

    // ソケットクローズ
    if( SOC_ISVALID( soc ) ){
#ifdef _WIN32
        closesocket( soc );
#else
        close( soc );
#endif
    }

    // addrinfo開放
    if( m_addrinfo ) freeaddrinfo( m_addrinfo );
    m_addrinfo = NULL;

    // トークン返す
    return_token( this );

    finish_loading();

#ifdef _DEBUG
    std::cout << "Loader::run_main : finish loading : " << m_data.url << std::endl;;
    std::cout << "read size : " << m_data.length_current << " / " << m_data.length << std::endl;;    
    std::cout << "data size : " << m_data.size_data << std::endl;;
    std::cout << "code : " << m_data.code << std::endl << std::endl;
#endif    
}



//
// addrinfo 取得
//
struct addrinfo* Loader::get_addrinfo( const std::string& hostname, const int port )
{
    if( port < 0 || port > 65535 ) return NULL;
    if( hostname.empty() ) return NULL;

    int ret;
    struct addrinfo hints, *res;
    const int poststrlng = 256;
    char port_str[ poststrlng ];
    memset( &hints, 0, sizeof( addrinfo ) );
    if( m_data.use_ipv6 ) hints.ai_family = AF_UNSPEC;
    else hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    snprintf( port_str, poststrlng, "%d", port );
    ret = getaddrinfo( hostname.c_str(), port_str, &hints, &res );
    if( ret ) {
        MISC::ERRMSG( m_data.str_code );
        return NULL;
    }
    
#ifdef _DEBUG    
    std::cout << "host = " << hostname
              << " ipv6 = " << m_data.use_ipv6
              << ", ip =" << inet_ntoa( (  ( sockaddr_in* )( res->ai_addr ) )->sin_addr ) << std::endl;
#endif

    return res;
}



//
// 送信メッセージ作成
//
const std::string Loader::create_msg_send()
{
    bool post_msg = ( !m_data.str_post.empty() && !m_data.head );
    bool use_proxy = ( ! m_data.host_proxy.empty() );

    std::ostringstream msg;
    msg.clear();

    if( m_data.head ) msg << "HEAD ";
    else if( ! post_msg ) msg << "GET ";
    else msg << "POST ";
    
    if( ! use_proxy ) msg << m_data.path << " HTTP/1.1\r\n";
    else {
        msg << m_data.protocol << m_data.host << ":" << m_data.port << m_data.path << " HTTP/1.1\r\n";
    }

    msg << "Host: " << m_data.host << "\r\n";
    if( ! m_data.contenttype.empty() ) msg << "Content-Type: " << m_data.contenttype << "\r\n";
    if( ! m_data.agent.empty() ) msg << "User-Agent: " << m_data.agent << "\r\n";
    if( ! m_data.referer.empty() ) msg << "Referer: " << m_data.referer << "\r\n";

    // basic認証
    if( ! m_data.basicauth.empty() ) msg << "Authorization: Basic " << MISC::base64( m_data.basicauth ) << "\r\n";

    // proxy basic認証
    if( use_proxy && ! m_data.basicauth_proxy.empty() ) msg << "Proxy-Authorization: Basic " << MISC::base64( m_data.basicauth_proxy ) << "\r\n";

    if( ! m_data.cookie_for_write.empty() ) msg << "Cookie: " << m_data.cookie_for_write << "\r\n";

    if( ! m_data.modified.empty() ) msg << "If-Modified-Since: " << m_data.modified << "\r\n";

    // レジュームするときは gzip は受け取らない
    if( m_data.byte_readfrom ){

        msg << "Range: bytes=" << m_data.byte_readfrom << "-\r\n";

        // プロキシ使う場合は no-cache 指定 
        if( use_proxy ) msg << "Cache-Control: no-cache\r\n";
    }
    else msg << "Accept-Encoding: gzip\r\n"; // レジュームしないなら gzip 受け取り可能

    // その他のフィールド
    if( ! m_data.ex_field.empty() ) msg << m_data.ex_field;

    msg << "Connection: close\r\n";

    // POST する文字列
    if( post_msg ){
        
        msg << "Content-Length: " << m_data.str_post.length() << "\r\n";
        msg << "\r\n";
        msg << m_data.str_post;
        msg << "\r\n";
    }
    msg << "\r\n";
    
    return msg.str();
}


//
// サーバから送られてきた生データからヘッダ取得
//
// 戻り値 : 成功 HTTP_OK、失敗 HTTP_ERR、未処理 HTTP_INIT
//
// 入力
// buf : 生データ
// readsize: 生データサイズ
//
// 出力
// buf : ヘッダが取り除かれたデータ
// readsize: 出力データサイズ
//
int Loader::receive_header( char* buf, size_t& read_size )
{
#ifdef _DEBUG
    std::cout << "Loader::receive_header : read_size = " << read_size << std::endl;
#endif

    buf[ read_size ] = '\0';
    m_data.str_header = buf;
    size_t lng_header = m_data.str_header.find( "\r\n\r\n" );
    if( lng_header != std::string::npos ) lng_header += 4;
    else{

        lng_header = m_data.str_header.find( "\n\n" );
        if( lng_header != std::string::npos ) lng_header +=2;
        else return HTTP_INIT;
    }
        
    m_data.str_header.resize( lng_header ); 

#ifdef _DEBUG    
    std::cout << "header : size = " << lng_header << " byte\n";
    std::cout << m_data.str_header << std::endl;
#endif

    if( ! analyze_header() ) return HTTP_ERR;
                
    // 残りのデータを前に移動
    read_size -= lng_header;
    if( read_size ){

        memmove( buf, buf+ lng_header, read_size );
        buf[ read_size ] = '\0';
    }

    return HTTP_OK;
}


//
// ヘッダ解析
//
bool Loader::analyze_header()
{
    // コード
    std::string str_tmp = analyze_header_option( "HTTP/1.1 " );
    if( ! str_tmp.empty() ){
        m_data.str_code = "HTTP/1.1 "  + str_tmp;
    }
    else{
        str_tmp = analyze_header_option( "HTTP/1.0 " );
        if( ! str_tmp.empty() ) m_data.str_code = "HTTP/1.0 "  + str_tmp;
    }

    if( str_tmp.empty() ){
        MISC::ERRMSG( "could not find HTTP/1.1" );
        return false;
    }

    size_t i = str_tmp.find( " " );
    if( i == std::string::npos ) m_data.code = atoi( str_tmp.c_str() );
    else m_data.code = atoi( str_tmp.substr( 0, i ).c_str() );

    if( m_data.code == 0 ){
        MISC::ERRMSG( "could not get http status code" );
        return false;
    }

    // サイズ
    m_data.length = 0;
    str_tmp = analyze_header_option( "Content-Length: " );
    if( ! str_tmp.empty() ) m_data.length = atoi( str_tmp.c_str() );
    
    // date
    m_data.date = analyze_header_option( "Date: " );

    // modified
    m_data.modified = analyze_header_option( "Last-Modified: " );
    
    // cookie
    m_data.list_cookies = analyze_header_option_list( "Set-Cookie: " );

    // Location
    if( m_data.code == HTTP_REDIRECT
        || m_data.code == HTTP_MOVED_PERM ) m_data.location = analyze_header_option( "Location: " );
    else m_data.location = std::string();

    // Content-Type
    m_data.contenttype = analyze_header_option( "Content-Type: " );
    
    // chunked か
    m_use_chunk = false;
    str_tmp = analyze_header_option( "Transfer-Encoding: " );
    if( str_tmp.find( "chunked" ) != std::string::npos ){
        
        m_use_chunk = true;
        m_status_chunk = 0;
        m_pos_sizepart = m_str_sizepart;
    }

    // gzip か
    m_use_zlib = false;
    str_tmp = analyze_header_option( "Content-Encoding: " );
    if( str_tmp.find( "gzip" ) != std::string::npos ){
        if( !init_unzip() ) return false;
    }

#ifdef _DEBUG
    std::cout << "code = " << m_data.code << std::endl;
    std::cout << "length = " << m_data.length << std::endl;    
    std::cout << "date = " << m_data.date << std::endl;
    std::cout << "modified = " << m_data.modified << std::endl;

    std::list< std::string >::iterator it = m_data.list_cookies.begin();
    for( ; it != m_data.list_cookies.end(); ++it ) std::cout << "cookie = " << (*it) << std::endl;

    std::cout << "location = " << m_data.location << std::endl;
    std::cout << "contenttype = " << m_data.contenttype<< std::endl;            
    if( m_use_chunk ) std::cout << "m_use_chunk = true\n";
    if( m_use_zlib )  std::cout << "m_use_zlib = true\n";

    std::cout << "authenticate = " << analyze_header_option( "WWW-Authenticate: " ) << std::endl;

    std::cout << "\n";
#endif
    
    return true;
}


//
// analyze_header() から呼ばれるオプションの値を取り出す関数
//
std::string Loader::analyze_header_option( const std::string& option )
{
    size_t i = 0, i2 = 0;
    i = m_data.str_header.find( option, 0 );    
    if( i != std::string::npos ){
        const size_t option_length = option.length();
        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( "\n", i );
        if( i2 != std::string::npos ) return m_data.str_header.substr( i + option_length, i2 - ( i + option_length ) );
    }

    return std::string();
}



//
// analyze_header() から呼ばれるオプションの値を取り出す関数(リスト版)
//
std::list< std::string > Loader::analyze_header_option_list( const std::string& option )
{
    std::list< std::string > str_list;
    
    size_t i = 0, i2 = 0;
    const size_t option_length = option.length();

    for(;;){

        i = m_data.str_header.find( option, i2 );    
        if( i == std::string::npos ) break;

        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( "\n", i );
        if( i2 == std::string::npos ) break;

        str_list.push_back( m_data.str_header.substr( i + option_length, i2 - ( i + option_length ) ) );
    }

    return str_list;
}




//
// chunked なデータを切りつめる関数
//
// 入力
// buf : 生データ
// readsize: 生データサイズ
//
// 出力
// buf : 切りつめられたデータ
// readsize: 出力データサイズ
//
bool Loader::skip_chunk( char* buf, size_t& read_size )
{
#ifdef _DEBUG_CHUNKED    
    std::cout << "\n[[ skip_chunk : start read_size = " << read_size << " ]]\n";
#endif    
    
    size_t pos_chunk = 0;
    size_t pos_data_chunk_start = 0;
    size_t buf_size = 0;
    
    for(;;){

        // サイズ部
        if( m_status_chunk == 0 ){  
        
            // \nが来るまで m_str_sizepart[] に文字をコピーしていく
            for( ; pos_chunk < read_size; ++pos_chunk, ++m_pos_sizepart ){

                // バッファオーバーフローのチェック
                if( ( long )( m_pos_sizepart - m_str_sizepart ) >= 64 ){
                    MISC::ERRMSG( "buffer over flow at skip_chunk" );
                    return false;
                }
                
                *( m_pos_sizepart ) =  buf[ pos_chunk ];

                // \n が来たらデータ部のサイズを取得
                if( buf[ pos_chunk ] == '\n' ){

                    ++pos_chunk;
                    
                    *( m_pos_sizepart ) = '\0';
                    if( *( m_pos_sizepart -1 ) == '\r' ) *( m_pos_sizepart -1 ) = '\0'; // "\r\n"の場合
                    m_lng_leftdata = strtol( m_str_sizepart, NULL, 16 );
                    m_pos_sizepart = m_str_sizepart;
                    
#ifdef _DEBUG_CHUNKED
                    std::cout << "[[ skip_chunk : size chunk finished : str = 0x" << m_str_sizepart << " ]]\n";                    
                    std::cout << "[[ skip_chunk : enter the data chunk, data size = " << m_lng_leftdata << " ]]\n";
#endif
                    pos_data_chunk_start = pos_chunk;
                    m_status_chunk = 1;
                    
                    break;
                }
            }
        }

        // データ部
        if( m_status_chunk == 1 ){ 

            // データを前に詰める
            if( m_lng_leftdata ){
                for( ; m_lng_leftdata > 0 && pos_chunk < read_size; --m_lng_leftdata, ++pos_chunk );
                size_t buf_size_tmp = pos_chunk - pos_data_chunk_start;
                if( buf_size != pos_data_chunk_start && buf_size_tmp ) memmove( buf + buf_size , buf + pos_data_chunk_start,  buf_size_tmp );
                buf_size +=  buf_size_tmp;
            }

            // データを全部読み込んだらデータ部終わり
            if( m_lng_leftdata == 0 ) m_status_chunk = 2;
        }

        // データ部→サイズ部切り替え中("\r"の前)
        if( m_status_chunk == 2 && pos_chunk != read_size ){

            if( buf[ pos_chunk++ ] != '\r' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }

            m_status_chunk = 3;
        }

        // データ部→サイズ部切り替え中("\n"の前: "\r" と "\n" の間でサーバからの入力が分かれる時がある)
        if( m_status_chunk == 3 && pos_chunk != read_size ){

            if( buf[ pos_chunk++ ] != '\n' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }

#ifdef _DEBUG_CHUNKED
            std::cout << "[[ skip_chunk : data chunk finished. ]]\n";
#endif
            m_status_chunk = 0;
        }

        // バッファ終わり
        if( pos_chunk == read_size ){
            
            read_size = buf_size;
            buf[ read_size ] = '\0';
            
#ifdef _DEBUG_CHUNKED
            std::cout << "[[ skip_chunk : output = " << read_size << " ]]\n\n";
#endif
            return true;
        }
    }

    return true;
}



//
// zlib 初期化
//
bool Loader::init_unzip()
{
#ifdef _DEBUG
    std::cout << "Loader::init_unzip\n";
#endif

    m_use_zlib = true;
        
#ifdef USE_OLDZLIB
    m_check_gzheader = true;
#endif

    // zlib 初期化
    m_zstream.zalloc = Z_NULL;
    m_zstream.zfree = Z_NULL;
    m_zstream.opaque = Z_NULL;
    m_zstream.next_in = Z_NULL;
    m_zstream.avail_in = 0;

#ifdef USE_OLDZLIB
#ifdef _DEBUG
    std::cout << "use old zlib\n";
#endif
    if ( inflateInit2( &m_zstream, -MAX_WBITS ) != Z_OK )
#else
#ifdef _DEBUG
    std::cout << "use zlib12\n";
#endif
    if ( inflateInit2( &m_zstream, 15 + 32 ) != Z_OK ) // デフォルトの15に+32する( windowBits = 47 )と自動でヘッダ認識
#endif
    {  

        MISC::ERRMSG( "inflateInit2 failed." );
        return false;
    }

    assert( m_buf_zlib_in == NULL );
    assert( m_buf_zlib_out == NULL );
    m_buf_zlib_in = ( Bytef* )malloc( m_lng_buf_zlib_in + 64 );
    m_buf_zlib_out = ( Bytef* )malloc( m_lng_buf_zlib_out + 64 );

    return true;
}



#ifdef USE_OLDZLIB
//
// gzip のヘッダチェック
//
// 戻り値 : ヘッダのサイズ(エラーなら-1)
//
// TODO: size がヘッダサイズよりも小さいと必ず失敗する
//
int Loader::check_gzheader( char* buf, size_t& size )
{
#ifdef _DEBUG
    std::cout << "Loader::check_gzheader\n";
#endif    

    m_check_gzheader = false;

    unsigned int pos = 0;

    if( ( ( unsigned char* )buf )[ pos++ ] != 0x1f || ( ( unsigned char* )buf )[ pos++ ] != 0x8b ) return -1;

    // CM(1byte)
    if( buf[ pos++ ] != 0x08 ) return false;

    // FLG(1byte)
    unsigned char flag =( ( unsigned char* )buf )[ pos++ ];
#ifdef _DEBUG
    std::cout << "flag = " << std::hex << ( unsigned int )flag << std::dec << std::endl;
#endif
    if( flag & 0xe0 ) return -1;

    // MTIME(4byte), XFL(1byte), OS(1byte)を飛ばす
    pos += 6;

    // XLEN(2byte) + 拡張フィールド
    if( flag & 4 ){
        unsigned short lngext;
        memcpy( &lngext, buf + pos, sizeof( unsigned short ) );
#ifdef _DEBUG
        std::cout << "FEXTRA lng = " << lngext << std::endl;
#endif
        pos += sizeof( unsigned short ) + lngext;
    }

    // NAME
    if( flag & 8 ){
        int i = 0;
        char tmp_str[ 256 ];
        while( pos < size && i < 256  && buf[ pos ] != '\0' ) tmp_str[ i++ ] = buf[ pos++ ];
        tmp_str[ i ] = buf[ pos++ ];
        if( tmp_str[ i ] != '\0' ) return -1;
#ifdef _DEBUG
        std::cout << "NAME = " << tmp_str << std::endl;
#endif
    }

    // COMMENT
    if( flag & 16 ){
        int i = 0;
        char tmp_str[ 256 ];
        while( pos < size && i < 256  && buf[ pos ] != '\0' ) tmp_str[ i++ ] = buf[ pos++ ];
        tmp_str[ i ] = buf[ pos++ ];
        if( tmp_str[ i ] != '\0' ) return -1;
#ifdef _DEBUG
        std::cout << "COMMENT = " << tmp_str << std::endl;
#endif
    }

    // CRC16
    if(flag & 2 ){
        pos += 2;
#ifdef _DEBUG
        std::cout << "CRC16\n";
#endif
    }

    if( pos >= size ) return -1;

#ifdef _DEBUG
    std::cout << "header size = " << pos << std::endl;
#endif

    return pos;
}
#endif



//
// unzipしてコールバック呼び出し
//
bool Loader::unzip( char* buf, size_t& read_size )
{
#ifdef USE_OLDZLIB
    // gzip のヘッダを飛ばす
    if( m_check_gzheader ){
        int headsize = check_gzheader( buf, read_size );
        if( headsize < 0 ){

            MISC::ERRMSG( "invalid gzip header : " + m_data.url );
            return false;
        }
        buf += headsize;
        read_size -= headsize;
    }
#endif    

    // zlibの入力バッファに値セット
    if( m_zstream.avail_in + read_size > m_lng_buf_zlib_in ){ // オーバーフローのチェック

        MISC::ERRMSG( "buffer over flow at zstream_in : " + m_data.url );
        return false;
    }
    memcpy( m_buf_zlib_in + m_zstream.avail_in , buf, read_size );
    m_zstream.avail_in += read_size;
    m_zstream.next_in = m_buf_zlib_in;
            
    size_t byte_out = 0;
    do{

        // 出力バッファセット
        m_zstream.next_out = m_buf_zlib_out;
        m_zstream.avail_out = m_lng_buf_zlib_out;
        byte_out = 0;

        // 解凍
        int ret = inflate( &m_zstream, Z_NO_FLUSH );
        if( ret == Z_OK || ret == Z_STREAM_END ){
            
            byte_out = m_lng_buf_zlib_out - m_zstream.avail_out;
            m_buf_zlib_out[ byte_out ] = '\0';
            m_data.size_data += byte_out;
            
#ifdef _DEBUG
            std::cout << "inflate ok byte = " << byte_out << std::endl;
#endif
            
            // コールバック呼び出し
            if( byte_out && m_loadable ) m_loadable->receive( ( char* )m_buf_zlib_out, byte_out );

#ifdef USE_OLDZLIB
            if( ret == Z_STREAM_END ){
                m_zstream.avail_in = 0; // CRC32(4byte)とISIZE(4byte)の分を引く
                return true;
            }
#endif            
        }
        else return true;
                
    } while ( byte_out );

    // 入力バッファに使ってないデータが残っていたら前に移動
    if( m_zstream.avail_in ) memmove( m_buf_zlib_in, m_buf_zlib_in + ( read_size - m_zstream.avail_in ),  m_zstream.avail_in );

    return true;
}



//
// sent, recv待ち
//
bool Loader::wait_recv_send( const int fd, const bool recv )
{
    if( !fd ) return true;

    // 同期している場合は何もしない
    if( !m_data.async ) return true;

    int count = 0;
    for(;;){

        errno = 0;

        int ret;
        fd_set fdset;
        FD_ZERO( &fdset );
        FD_SET( fd , &fdset );

        timeval timeout;
        memset( &timeout, 0, sizeof( timeval ) );
        timeout.tv_sec = 1;

        if( recv ) ret = select( fd+1 , &fdset , NULL , NULL , &timeout );
        else ret = select( fd+1 , NULL, &fdset , NULL , &timeout );

#ifdef _DEBUG
        if( errno == EINTR && ret < 0 ) std::cout << "Loader::wait_recv_send : errno = EINTR " << errno << std::endl;
#endif
        if( errno != EINTR && ret < 0 ){
#ifdef _DEBUG
            std::cout << "Loader::wait_recv_send : errno = " << errno << std::endl;
#endif
            MISC::ERRMSG( "select failed" );
            break;
        }

        if( errno != EINTR && FD_ISSET( fd, &fdset ) ) return true;
        if( m_stop ) break;
        if( ++count >= m_data.timeout ) break;
#ifdef _DEBUG
        std::cout << "Loader::wait_recv_send ret = " << ret << " errno = " << errno << " timeout = " << count << std::endl;
#endif
    }
    
    return false;
}


//
// ローディング終了処理
//
void Loader::finish_loading()
{
#ifdef _DEBUG
    std::cout << "Loader::finish_loading : url = " << m_data.url << std::endl;
#endif

    // SKELETON::Loadable に終了を知らせる
    if( m_loadable ) m_loadable->finish();

    m_loading = false;

    JDLIB::pop_loader_queue();
}
