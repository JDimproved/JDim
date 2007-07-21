// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_CHUNKED
#include "jddebug.h"

#include "loader.h"
#include "miscmsg.h"
#include "miscutil.h"
#include "miscthread.h"

#include "skeleton/loadable.h"

#include "httpcode.h"

#include <pthread.h>
#include <sstream>

#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#ifndef NOUSE_SSL
#include <openssl/ssl.h>
#endif

// 最大スレッド数
#define MAX_LOADER 6

// 読み込みバッファの最小値 (byte)
#define LNG_BUF_MIN ( 16 * 1024 )  

// タイムアウトの最小値 (秒)
#define TIMEOUT_MIN 10             


//
// トークン処理
//
// MAX_LOADER 個を越えるローダは作成できない 
//

int token_loader = 0;

int JDLIB::token()
{
#ifdef _DEBUG
    std::cout << "token : token = " << token_loader << std::endl;
#endif    

    return token_loader;
}

// トークン取得
void JDLIB::get_token()
{
#ifdef _DEBUG
    std::cout << "get_token : token = " << token_loader << std::endl;
#endif    
    ++token_loader;
}

//　トークン返す
void JDLIB::return_token()
{
    --token_loader;
    assert( token_loader >= 0 );

#ifdef _DEBUG
    std::cout << "return_token : token = " << token_loader << std::endl;
#endif    
}


//
// loader作成関数
//
// 基本的にこれを使ってloaderを作成すること
// loaderがたくさん動いてるときはNULLを返す
//
JDLIB::Loader* JDLIB::create_loader()
{
    if( JDLIB::token() >= MAX_LOADER ){
#ifdef _DEBUG
        std::cout << "JDLIB::create_loader : could not create loader\n";
#endif 
        return NULL;
    }

    return new JDLIB::Loader();
}


//
// mainの最後でローダが動いていないかチェックする関数
//
void JDLIB::check_loader_alive()
{
#ifdef _DEBUG
    std::cout << "JDLIB::check_loader_alive loader = " << token_loader << std::endl;
#endif    

    if( token_loader ){
        MISC::ERRMSG( "loaders are still moving." );
        assert( false );
    }
}


using namespace JDLIB;



///////////////////////////////////////////////////



Loader::Loader()
    : m_addrinfo( 0 ),
      m_loading( 0 ),
      m_thread( 0 ),
      m_buf( 0 ),
      m_buf_zlib_in ( 0 ),
      m_buf_zlib_out ( 0 ),
      m_use_zlib ( 0 )
{
#ifdef _DEBUG
    std::cout << "Loader::Loader : loader was created.\n";
#endif

    clear();
}


Loader::~Loader()
{
    clear();

#ifdef _DEBUG
    std::cout << "Loader::~Loader : url = " << m_data.url << std::endl;
#endif

    assert( ! m_loading );
}


void Loader::clear()
{
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
    if( m_thread ) pthread_join( m_thread, NULL );
    m_thread = 0;
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
        if( m_data.protocol.find( "http://" ) != std::string::npos ) m_data.port = 80;

#ifndef NOUSE_SSL
        // https
        else if( m_data.protocol.find( "https://" ) != std::string::npos ){
            m_data.use_ssl = true;
            m_data.async = false;
            m_data.port = 443;;
        }
#endif

        // その他
        else{

            m_data.code = HTTP_ERR;
            m_data.str_code = "unknown protocol : " + m_data.url;
            MISC::ERRMSG( m_data.str_code );
            return false;
        }
    }

#ifndef NOUSE_SSL
    // 明示的にssl使用指定
    if( data_in.use_ssl ){
        m_data.use_ssl = true;
        m_data.async = false;
        m_data.port = 443;
    }
#else
    m_data.use_ssl = false;
#endif    

    // その他
    m_data.head = data_in.head;
    m_data.str_post = data_in.str_post;
    m_data.modified = data_in.modified;
    m_data.byte_readfrom = data_in.byte_readfrom;    
    m_data.agent = data_in.agent;
    m_data.referer = data_in.referer;
    m_data.cookie_for_write = data_in.cookie_for_write;
    m_data.host_proxy = data_in.host_proxy;
    m_data.port_proxy = data_in.port_proxy;
    if( m_data.port_proxy == 0 ) m_data.port_proxy = 8080;
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
    std::cout << "agent: " << m_data.agent << std::endl;
    std::cout << "referer: " << m_data.referer << std::endl;
    std::cout << "cookie: " << m_data.cookie_for_write << std::endl;
    std::cout << "proxy: " << m_data.host_proxy << std::endl;
    std::cout << "port of proxy: " << m_data.port_proxy << std::endl;
    std::cout << "buffer size: " << m_lng_buf / 1024 << " kb" << std::endl;
    std::cout << "timeout : " << m_data.timeout << " sec" << std::endl;
    std::cout << "ex_field : " << m_data.ex_field << std::endl;
    std::cout << "basicauth : " << m_data.basicauth << std::endl;
    std::cout << "\n";
#endif

    // スレッドを起動して run_main() 実行
    const int stacksize = 64;
    int status;
    m_stop = false;
    if( ( status = MISC::thread_create( &m_thread, ( STARTFUNC ) launcher, ( void * ) this, stacksize ) )){

        m_data.code = HTTP_ERR;
        m_data.str_code = std::string( "Loader::run : could not start thread : " ) + strerror( status );
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    m_loading = true;

    return true;
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


//
// 実際の処理部
//
void Loader::run_main()
{
    // エラーメッセージ
    std::string errmsg;

    // トークン取得
    get_token();

    int soc = -1; // ソケットID
    bool use_proxy = ( ! m_data.host_proxy.empty() );

#ifndef NOUSE_SSL
    // ssl用
    SSL_CTX *ctx = NULL;
    SSL* ssl = NULL;
#else
    bool ssl = false;
#endif
    
    // 送信メッセージ作成
    std::string msg_send = create_msg_send();
    
#ifdef _DEBUG    
    std::cout << "Loader::run_main : start loading thread : " << m_data.url << std::endl;;
    if( use_proxy ) std::cout << "use_proxy : " << m_data.host_proxy << std::endl;
    std::cout <<"send :----------\n" <<  msg_send << "\n---------------\n";
#endif

    // addrinfo 取得
    if( m_data.host_proxy.empty() ) m_addrinfo = get_addrinfo( m_data.host, m_data.port );
    else m_addrinfo = get_addrinfo( m_data.host_proxy, m_data.port_proxy );
    if( ! m_addrinfo ){
        m_data.code = HTTP_ERR;
        errmsg = "getaddrinfo failed : " + m_data.url;
        goto EXIT_LOADING;
    }

    // ソケット作成
    soc = socket( m_addrinfo ->ai_family, m_addrinfo ->ai_socktype, m_addrinfo ->ai_protocol );
    if ( soc < 0 ){
        m_data.code = HTTP_ERR;
        errmsg = "socket failed : " + m_data.url;
        goto EXIT_LOADING;
    }

    // ソケットを非同期に設定
    if( m_data.async ){
        int flags;
        flags = fcntl( soc, F_GETFL, 0);
        if( flags == -1 || fcntl( soc, F_SETFL, flags | O_NONBLOCK ) < 0 ){
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
        if ( !( m_data.async && errno == EINPROGRESS ) ){

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
        if( getsockopt( soc, SOL_SOCKET, SO_ERROR, (void *)&optval, &optlen ) < 0 ){
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

#ifndef NOUSE_SSL
    // ssl 初期化
    if( m_data.use_ssl ){

        SSL_library_init();
        ctx = SSL_CTX_new( SSLv23_client_method() );
        if( !ctx ){
            m_data.code = HTTP_ERR;
            errmsg = "SSL_CTX_new failed : " + m_data.url;
            goto EXIT_LOADING;
        }

        ssl = SSL_new( ctx );
        if( !ssl ){
            m_data.code = HTTP_ERR;
            errmsg = "SSL_new failed : " + m_data.url;
            goto EXIT_LOADING;
        }

        if( SSL_set_fd( ssl, soc ) == 0 ){
            m_data.code = HTTP_ERR;
            errmsg = "SSL_set_fd : " + m_data.url;
            goto EXIT_LOADING;
        }

        if( SSL_connect( ssl ) != 1 ){
            m_data.code = HTTP_ERR;
            errmsg = "SSL_connect : " + m_data.url;
            goto EXIT_LOADING;
        }            

#ifdef _DEBUG
        std::cout << "init ssl ok\n";
#endif
    }
#endif

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
            ssize_t tmpsize = send( soc, msg_send.data(), send_size , MSG_NOSIGNAL );
            if( tmpsize == 0
                || ( tmpsize < 0 && !( errno == EWOULDBLOCK || errno == EINTR ) ) ){

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

#ifndef NOUSE_SSL
    // SSL使用
    else{ 

        if( SSL_write( ssl, msg_send.data(), strlen( msg_send.data() ) ) < 0 ){

            m_data.code = HTTP_ERR;
            errmsg = "send failed(SSL) : " + m_data.url;
            goto EXIT_LOADING;
        }
    }
#endif

    // 受信用バッファを作ってメッセージ受信
    size_t mrg;
    mrg = 64; // 一応オーバーフロー避けのおまじない
    assert( m_buf == NULL );
    m_buf = ( char* )malloc( m_lng_buf + mrg );

    bool receiving_header;

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

            }

#ifndef NOUSE_SSL
            // SSL
            else{

                tmpsize = SSL_read( ssl, m_buf + read_size, m_lng_buf - read_size - mrg );
                if( tmpsize < 0 ){
                    m_data.code = HTTP_ERR;         
                    errmsg = "SSL_read() failed";
                    goto EXIT_LOADING;
                }
            }
#endif

            if( tmpsize == 0 ) break;
            if( tmpsize > 0 ) read_size += tmpsize;
        }

        m_buf[ read_size + 1 ] = '\0';

        // 停止指定
        if( m_stop ) break;

        // サーバ側がcloseした
        if( read_size == 0 ){

            // ヘッダを取得する前にcloseした
            if( receiving_header && m_data.size_data == 0 ){
                m_data.code = HTTP_ERR;         
                errmsg = "no data";
                goto EXIT_LOADING;
            }

            break;
        }

        // ヘッダ取得
        if( receiving_header ){

            if( ! receive_header( m_buf, read_size ) ){

                m_data.code = HTTP_ERR;
                errmsg = "invalid header : " + m_data.url;
                goto EXIT_LOADING;
            }
            receiving_header = false;
            if( ! read_size ) continue;
        }

        m_data.length_current += read_size;
        
        //  chunkedな場合
        if( m_use_chunk ){
            
            if( !skip_chunk( m_buf, read_size ) ){

                m_data.code = HTTP_ERR;
                errmsg = "skip_chunk() failed : " + m_data.url;
                goto EXIT_LOADING;
            }
            if( ! read_size ) continue;
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
        
    } while( !m_stop );

    // 終了処理
EXIT_LOADING:

#ifndef NOUSE_SSL
    // ssl クローズ
    if( ssl ){
        SSL_shutdown( ssl );
        SSL_free( ssl );
    }
    if( ctx ) SSL_CTX_free( ctx );
#endif

    if( soc >= 0 ){

        // writefds待ち
        // 待たないとclose()したときにfinパケットが消える？
        if( ! wait_recv_send( soc, false ) ){

            // タイムアウト
            m_data.code = HTTP_TIMEOUT;         
            errmsg = "send timeout";
        }

        // 送信禁止
        shutdown( soc, SHUT_WR );
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
    if( soc >= 0 ) close( soc );

    // addrinfo開放
    if( m_addrinfo ) freeaddrinfo( m_addrinfo );
    m_addrinfo = NULL;

    // トークン返す
    return_token();

    // Loadable::finish()をコールバックして終わり
    if( m_loadable ) m_loadable->finish();

#ifdef _DEBUG
    std::cout << "Loader::run_main : finish loading : " << m_data.url << std::endl;;
    std::cout << "read size : " << m_data.length_current << std::endl;;    
    std::cout << "data size : " << m_data.size_data << std::endl;;
    std::cout << "code : " << m_data.code << std::endl << std::endl;
#endif    

    m_loading = false;
}



//
// addrinfo 取得
//
struct addrinfo* Loader::get_addrinfo( const std::string& hostname, int port )
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
std::string Loader::create_msg_send()
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
    if( ! m_data.agent.empty() ) msg << "User-Agent: " << m_data.agent << "\r\n";
    if( ! m_data.referer.empty() ) msg << "Referer: " << m_data.referer << "\r\n";

    // basic認証
    if( ! m_data.basicauth.empty() ) msg << "Authorization: Basic " << MISC::base64( m_data.basicauth ) << "\r\n";

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
// 処理を簡単にするためにヘッダはバッファ内にすべてあると仮定(バッファ超えたらエラー)
//
// 入力
// buf : 生データ
// readsize: 生データサイズ
//
// 出力
// buf : ヘッダが取り除かれたデータ
// readsize: 出力データサイズ
//
bool Loader::receive_header( char* buf, size_t& read_size )
{
    buf[ read_size ] = '\0';
    m_data.str_header = buf;

    size_t lng_header = m_data.str_header.find( "\r\n\r\n" );
    if( lng_header == std::string::npos ) lng_header = m_data.str_header.find( "\n\n" );
    if( lng_header == std::string::npos ){
        MISC::ERRMSG( "could not find HTML header" );
#ifdef _DEBUG
        std::cout << "Loader::receive_header : read_size = " << read_size << std::endl;
        std::cout << m_data.str_header << std::endl;
#endif
        return false;
    }
        
    lng_header += 2;
    m_data.str_header.resize( lng_header ); 

#ifdef _DEBUG    
    std::cout << "\nheader : " << lng_header << " byte\n";
    std::cout << m_data.str_header << std::endl;
#endif

    if( ! analyze_header() ) return false;
                
    // 残りのデータを前に移動
    lng_header += 2;
    read_size -= lng_header;
    if( read_size ){

        memmove( buf, buf+ lng_header, read_size );
        buf[ read_size ] = '\0';
    }

    return true;
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
    size_t i = str_tmp.find( " " );
    if( i == std::string::npos ){
        MISC::ERRMSG( "could not find HTTP/1.1" );
        return false;
    }
    m_data.code = atoi( str_tmp.substr( 0, i ).c_str() );

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
    if( m_data.code == HTTP_REDIRECT ) m_data.location = analyze_header_option( "Location: " );
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
std::string Loader::analyze_header_option( char* option )
{
    size_t i = 0, i2 = 0;
    i = m_data.str_header.find( option, 0 );    
    if( i != std::string::npos ){
        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( "\n", i );
        if( i2 != std::string::npos ) return m_data.str_header.substr( i + strlen( option ), i2 - ( i + strlen( option ) ) );
    }

    return std::string();
}



//
// analyze_header() から呼ばれるオプションの値を取り出す関数(リスト版)
//
std::list< std::string > Loader::analyze_header_option_list( char* option )
{
    std::list< std::string > str_list;
    
    size_t i = 0, i2 = 0;

    for(;;){

        i = m_data.str_header.find( option, i2 );    
        if( i == std::string::npos ) break;

        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( "\n", i );
        if( i2 == std::string::npos ) break;

        str_list.push_back( m_data.str_header.substr( i + strlen( option ), i2 - ( i + strlen( option ) ) ) );
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

        // データ部→サイズ部切り替え中( "\r" と "\n" の間でサーバからの入力が分かれる時がある)
        if( m_status_chunk == 2 ){

            unsigned char c = buf[ pos_chunk ];
            if( c != '\r' && c != '\n' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }

            // \r\nが来たらサイズ部に戻る
            if( c == '\r' ) ++pos_chunk; else break;      
            if( buf[ pos_chunk ] == '\n' ) ++pos_chunk; else break;
            
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
bool Loader::wait_recv_send( int fd, bool recv )
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
        std::cout << "Loader::wait_recv_send ret = " << ret << " timeout = " << count << std::endl;
#endif
    }
    
    return false;
}
