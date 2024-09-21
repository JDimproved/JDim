// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_TIME
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "loader.h"

#include "jdsocket.h"
#include "miscmsg.h"
#include "miscutil.h"

#ifdef _DEBUG_TIME
#include "misctime.h"
#include <iostream>
#endif

#include "config/globalconf.h"

#include "skeleton/loadable.h"

#include "httpcode.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <mutex>
#include <sstream>
#include <system_error>


constexpr int MAX_LOADER = 10; // 最大スレッド数
constexpr int MAX_LOADER_SAMEHOST = 2; // 同一ホストに対して実行できる最大スレッド数
constexpr std::size_t LNG_BUF_MIN = 1 * 1024; // 読み込みバッファの最小値 (byte)
constexpr int TIMEOUT_MIN = 1; // タイムアウトの最小値 (秒)


namespace {

std::string get_error_message( int err_code )
{
    std::string result = " [Errno " + std::to_string( err_code ) + "] ";
    result.append( std::strerror( err_code ) );
    return result;
}

} // namespace


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


static std::mutex mutex_token;
static std::mutex mutex_queue;
std::list< JDLIB::Loader* > queue_loader; // スレッド起動待ちの Loader のキュー
int token_loader = 0;
std::vector< JDLIB::Loader* > vec_loader( MAX_LOADER );
bool disable_pop = false;


// トークン取得
bool JDLIB::get_token( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_token );

#ifdef _DEBUG
    std::cout << "JDLIB::get_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif

    if( token_loader >= MAX_LOADER ) return false;

    const std::string& host = loader->data().host;
    const auto compare_host = [&host]( const auto* ldr ) { return ldr && ldr->data().host == host; };
    const int count = std::count_if( vec_loader.cbegin(), vec_loader.cend(), compare_host );
#ifdef _DEBUG
    std::cout << "count = " << count << std::endl;
#endif

    const int max_loader = std::clamp( CONFIG::get_connection_num(), 1, MAX_LOADER_SAMEHOST );
    if( count >= max_loader ) return false;

#ifdef _DEBUG
    std::cout << "got token\n";
#endif

    ++token_loader;

    auto it = std::find( vec_loader.begin(), vec_loader.end(), nullptr );
    if( it != vec_loader.end() ) *it = loader;

    return true;
}


//　トークン返す
void JDLIB::return_token( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_token );

    --token_loader;
    assert( token_loader >= 0 );

    constexpr JDLIB::Loader* null_loader = nullptr;
    std::replace( vec_loader.begin(), vec_loader.end(), loader, null_loader );

#ifdef _DEBUG
    std::cout << "JDLIB::return_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif
}


// スレッド起動待ちキューに Loader を登録
void JDLIB::push_loader_queue( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_queue );

    if( ! loader ) return;

    if( loader->get_low_priority() ) queue_loader.push_back( loader );
    else{

        const auto pos = std::find_if( queue_loader.cbegin(), queue_loader.cend(),
                                       []( const JDLIB::Loader* p ) { return p && p->get_low_priority(); } );
        queue_loader.insert( pos, loader );
    }

#ifdef _DEBUG
    std::cout << "JDLIB::push_loader_queue url = " << loader->data().url << " size = " << queue_loader.size() << std::endl;
#endif    
}


// キューから Loader を取り除いたらtrueを返す
bool JDLIB::remove_loader_queue( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_queue );

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
    std::lock_guard< std::mutex > lock( mutex_queue );

    if( disable_pop ) return;
    if( ! queue_loader.size() ) return;

#ifdef _DEBUG
    std::cout << "JDLIB::pop_loader_queue size = " << queue_loader.size() << std::endl;
#endif

    auto it = std::find_if( queue_loader.begin(), queue_loader.end(), &JDLIB::get_token );
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
    std::lock_guard< std::mutex > lock( mutex_queue );

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
}



///////////////////////////////////////////////////

using namespace JDLIB;


/**
 * @brief デコーダーの状態を初期化する
 */
void ChunkedDecoder::clear()
{
    m_state = State::parse_size;
    m_lng_leftdata = 0;
    m_buf_sizepart.clear();
}


/** @brief chunked なデータを切りつめる
 *
 * @param[in,out] buf 生データ -> 切りつめられたデータ (not null)
 * @param[in,out] read_size 生データサイズ -> 出力データサイズ
 * @return 生データが壊れていて処理できないときはfalse
 */
bool ChunkedDecoder::decode( char* buf, std::size_t& read_size )
{
    assert( buf );
    std::size_t pos_chunk = 0;
    std::size_t pos_body_start = 0; // データ部の先頭位置
    std::size_t decoded_size = 0; // デコード済みのデータ長

    while(1) {

        // サイズ部
        if( m_state == State::parse_size ) {

            // サイズ部の途中でデータが途切れても再開できるように
            // \r\nが来るまで m_buf_sizepart に文字をコピーしていく
            for( ; pos_chunk < read_size; ++pos_chunk ) {

                m_buf_sizepart.push_back( buf[pos_chunk] );

                // \r\n が来たらデータ部のサイズを取得
                if( const auto size = m_buf_sizepart.size();
                        size >= 2 && m_buf_sizepart.compare( size - 2, 2, "\r\n" ) == 0 ) {

                    char* end;
                    m_lng_leftdata = std::strtoul( m_buf_sizepart.data(), &end, 16 );
                    // 非数字で解析しなかったときもstrtoul()の結果が0になるためendをチェックする
                    if( m_buf_sizepart.data() == end ) {
                        MISC::ERRMSG( "broken chunked data that size part is not hexadecimal." );
                        return false;
                    }
                    m_buf_sizepart.clear();

                    // 最後のチャンク(長さ0)に達したら終了してトレーラー部は無視する
                    if( m_lng_leftdata == 0 ) {
                        read_size = decoded_size;
                        m_state = State::completed;
                        return true;
                    }

                    pos_chunk += 1; // \n をスキップする
                    pos_body_start = pos_chunk;
                    m_state = State::format_body;
                    break;
                }
            }
        }

        // データ部
        if( m_state == State::format_body ) {

            // データを前に詰める
            if( m_lng_leftdata ){
                // 残りのチャンクデータと残りの入力バッファの少ないほう
                const std::size_t cut_size = (std::min)( m_lng_leftdata, read_size - pos_chunk );
                m_lng_leftdata -= cut_size;
                pos_chunk += cut_size;

                const std::size_t chunk_size = pos_chunk - pos_body_start;
                if( decoded_size != pos_body_start && chunk_size ) {
                    std::memmove( buf + decoded_size, buf + pos_body_start, chunk_size );
                }
                decoded_size += chunk_size;
            }

            // データを全部読み込んだらデータ部終わり
            if( m_lng_leftdata == 0 ) m_state = State::check_body_cr;
        }

        // データ部→サイズ部切り替え中("\r"の前)
        if( m_state == State::check_body_cr && pos_chunk != read_size ) {

            if( buf[pos_chunk] != '\r' ) {
                MISC::ERRMSG( "broken chunked data that is missing last CR." );
                return false;
            }
            ++pos_chunk;
            m_state = State::check_body_lf;
        }

        // データ部→サイズ部切り替え中("\n"の前: "\r" と "\n" の間でサーバからの入力が分かれる時がある)
        if( m_state == State::check_body_lf && pos_chunk != read_size ) {

            if( buf[pos_chunk] != '\n' ) {
                MISC::ERRMSG( "broken chunked data that is missing last LF." );
                return false;
            }
            ++pos_chunk;
            m_state = State::parse_size;
        }

        // バッファ終わり
        if( pos_chunk == read_size || m_state == State::completed ) {
            read_size = decoded_size;
            // 処理の途中または完了したので m_state は変更しない
            return true;
        }
    }
    return true;
}


/**
 * @brief デコーダーの状態を開放する
 */
void GzipDecoder::clear()
{
    m_buf_gzip_in.clear();
    m_buf_gzip_out.clear();
    m_lng_gzip_in = 0;
    m_lng_gzip_out = 0;

    if( m_use_gzip ) inflateEnd( &m_zstream );
    m_use_gzip = false;

    m_callback = nullptr;
}


/** @brief zlib 初期化
 *
 * @details `lng_buf` を基に入出力のバッファサイズを設定する
 * @param[in] lng_buf   受信のバッファサイズ
 * @param[in] callback  展開したデータを処理する関数ラッパー
 * @return `true`なら初期化に成功した
 */
bool GzipDecoder::setup( std::size_t lng_buf, CallbackWrapper callback )
{
#ifdef _DEBUG
    std::cout << "GzipDecoder::setup\n";
#endif

    m_use_gzip = true;
    m_lng_gzip_in = lng_buf * 2;
    m_lng_gzip_out = lng_buf * 10; // 小さいとパフォーマンスが落ちるので少し多めに10倍位
    m_callback = std::move( callback );

    // zlib 初期化
    m_zstream.zalloc = Z_NULL;
    m_zstream.zfree = Z_NULL;
    m_zstream.opaque = Z_NULL;
    m_zstream.next_in = Z_NULL;
    m_zstream.avail_in = 0;

    if( inflateInit2( &m_zstream, 15 + 32 ) != Z_OK ) // デフォルトの15に+32する( windowBits = 47 )と自動でヘッダ認識
    {
        MISC::ERRMSG( "GzipDecoder: inflateInit2 failed." );
        return false;
    }

    // オーバーフロー対策としてマージンを追加してメモリを確保する
    m_buf_gzip_in.resize( m_lng_gzip_in + kMargin );
    m_buf_gzip_out.resize( m_lng_gzip_out + kMargin );

    return true;
}


/** @brief gzip圧縮されたデータを展開してcallbackに渡す
 *
 * @param[in] gzip      gzip圧縮されたデータ
 * @param[in] gzip_size 圧縮データのバイトサイズ
 * @return 展開したデータのサイズ、または入力用バッファが不足したとき `std::nullopt` を返す
 */
std::optional<std::size_t> GzipDecoder::feed( const char* gzip, const std::size_t gzip_size )
{
    // オーバーフローのチェック
    if( ( m_zstream.avail_in + gzip_size ) > m_lng_gzip_in ) {
        MISC::ERRMSG( "GzipDecoder: Failed to decompress data due to short allocated buffer." );
        return std::nullopt;
    }
    // zlibの入力バッファに値セット
    std::memcpy( m_buf_gzip_in.data() + m_zstream.avail_in, gzip, gzip_size );
    m_zstream.avail_in += gzip_size;
    m_zstream.next_in = m_buf_gzip_in.data();

    std::size_t total_out = 0;
    std::size_t byte_out = 0;
    do {
        // 出力バッファセット
        m_zstream.next_out = m_buf_gzip_out.data();
        m_zstream.avail_out = m_lng_gzip_out;

        // 展開
        const int ret = inflate( &m_zstream, Z_NO_FLUSH );
        if( ret == Z_OK || ret == Z_STREAM_END ) {

            byte_out = m_lng_gzip_out - m_zstream.avail_out;
            total_out += byte_out;

#ifdef _DEBUG
            std::cout << "inflate ok byte = " << byte_out << std::endl;
#endif

            // コールバック呼び出し
            if( byte_out && m_callback ) {
                m_callback( reinterpret_cast<char*>( m_buf_gzip_out.data() ), byte_out );
            }
        }
        else return total_out;

    } while ( byte_out );

    // 入力バッファに使ってないデータが残っていたら前に移動
    if( const auto avail_in = m_zstream.avail_in; avail_in ) {
        std::memmove( m_buf_gzip_in.data(), m_buf_gzip_in.data() + ( gzip_size - avail_in ), avail_in );
    }

    return total_out;
}



//
// low_priority = true の時はスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
//
Loader::Loader( const bool low_priority )
    : m_low_priority( low_priority )
{
#ifdef _DEBUG
    std::cout << "Loader::Loader : loader was created.\n";
#endif

    clear();
}


Loader::~Loader()
{
#ifdef _DEBUG
    std::cout << "Loader::~Loader : url = " << m_data.url << std::endl;
#endif

    // m_thread.joinable() == true のときスレッドを破棄すると強制終了するため待機処理を入れる
    clear();
}


void Loader::clear()
{
#ifdef _DEBUG
    std::cout << "Loader::clear\n";
#endif

    stop();
    wait();

    m_loadable = nullptr;
    
    m_use_chunk = false;
    m_chunk_decoder.clear();

    m_buf.clear();

    m_gzip_decoder.clear();
}


void Loader::wait()
{
    if( m_thread.joinable() ) m_thread.join();
}


void Loader::stop()
{
    if( ! m_loading ) return;

#ifdef _DEBUG
    std::cout << "Loader::stop : url = " << m_data.url << std::endl;
#endif

    m_stop.store( true, std::memory_order_release );

    // スレッド起動待ち状態の時は SKELETON::Loadable にメッセージを送る
    if( JDLIB::remove_loader_queue( this ) ){

        m_data.code = HTTP_TIMEOUT;
        m_data.modified.clear();
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
// cookie_for_request
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
    m_stop.store( false, std::memory_order_release );

    // バッファサイズ設定
    m_data.size_buf = data_in.size_buf;
    m_lng_buf = (std::max)( LNG_BUF_MIN, m_data.size_buf * 1024 );
    
    // protocol と host と path 取得
    m_data.url = data_in.url;
    std::size_t i = m_data.url.find( "://" ); // "http://" とつけるのは呼び出し側の責任で
    if( i == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could not get protocol : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    i += 3;
    m_data.protocol = data_in.url.substr( 0, i );

    const std::size_t i2 = m_data.url.find( '/', i );
    if( i2 == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could not get hostname and path : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    
    m_data.host = m_data.url.substr( i, i2 - i );
    m_data.path = m_data.url.substr( i2 );
    // HTTPやURLの仕様に基づいて、リクエストで送信するパスからアンカー(`#`以降の部分)を取り除く
    if( const auto anchor = m_data.path.find( '#' ); anchor != std::string::npos ) {
        m_data.path.erase( anchor );
    }

    // ポートセット
    // ホスト名の後に指定されている
    if( i = m_data.host.find( ':' ); i != std::string::npos ) {
        m_data.port = std::atoi( m_data.host.c_str() + ( i + 1 ) );
        m_data.host = m_data.host.substr( 0, i );
    }

    // 明示的にポートを指定
    else if( data_in.port != 0 ) m_data.port = data_in.port;

    // プロトコルを見て自動決定
    else if( m_data.protocol == "https://" ) m_data.port = 443;

    else if( m_data.protocol == "http://" ) m_data.port = 80;

    // その他
    else {
        m_data.code = HTTP_ERR;
        m_data.str_code = "unknown protocol : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }

    // SSL/TLS使用の指定
    constexpr std::array<const char*, 3> domains{ ".5ch.net", ".2ch.net", ".bbspink.com" };
    const std::string& hostname = m_data.host;
    const auto has_domain = [&hostname]( const char* d ) { return hostname.find( d ) != std::string::npos; };

    // 明示的にSSL/TLS使用を指定、または
    // HACK: httpsから始まるURLで プロキシを使わない or 2ch系サイトでない 場合はSSL/TLSで送受信する
    if( data_in.use_ssl
        || ( m_data.protocol == "https://"
             && ( data_in.host_proxy.empty() || std::none_of( domains.cbegin(), domains.cend(), has_domain ) )
             )
        ) {
        m_data.port = 443;
        m_data.use_ssl = true;
    }

    // プロキシ
    m_data.host_proxy = data_in.host_proxy;
    Proxy proxy;
    if( i = m_data.host_proxy.find( "://" ); i != std::string::npos ) {
        const auto scheme = std::string_view{ data_in.host_proxy }.substr( 0, i );
        if( scheme == "http" ) proxy = Proxy::http;
        else if( scheme == "socks4" ) proxy = Proxy::socks4;
        else if( scheme == "socks4a" ) proxy = Proxy::socks4a;
        else {
            m_data.code = HTTP_ERR;
            m_data.str_code = "unknown proxy proxy : ";
            m_data.str_code.append( scheme );
            MISC::ERRMSG( m_data.str_code );
            return false;
        }
        m_data.host_proxy = m_data.host_proxy.substr( i + 3 );
    }
    else {
        proxy = Proxy::http;
    }
    m_data.protocol_proxy = static_cast<int>( proxy );

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
    m_data.origin = data_in.origin;
    m_data.referer = data_in.referer;
    m_data.accept = data_in.accept;
    m_data.cookie_for_request = data_in.cookie_for_request;
    m_data.timeout = (std::max)( TIMEOUT_MIN, data_in.timeout );
    m_data.ex_field = data_in.ex_field;
    m_data.basicauth = data_in.basicauth;

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
    std::cout << "cookie: " << m_data.cookie_for_request << std::endl;
    std::cout << "protocol of proxy: " << m_data.protocol_proxy << std::endl;
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
    if( m_thread.joinable() ) {
        MISC::ERRMSG( "Loader::create_thread : thread is already running" );
        return;
    }

    try {
        m_thread = std::thread( &Loader::run_main, this );
    }
    catch( std::system_error& ) {
        m_data.code = HTTP_ERR;
        m_data.str_code = "Loader::run : could not start thread";
        MISC::ERRMSG( m_data.str_code );
        finish_loading();
    }
}


//
// 実際の処理部
//
void Loader::run_main()
{
    // エラーメッセージ
    std::string errmsg;

    const bool use_proxy{ ! m_data.host_proxy.empty() };

    // 送信メッセージ作成
    const std::string msg_send = create_msg_send();

#ifdef _DEBUG
    std::cout << "Loader::run_main : start loading thread : " << m_data.url << std::endl;
    if( use_proxy ) std::cout << "use_proxy : " << m_data.host_proxy << std::endl;
    std::cout <<"send :----------\n" <<  msg_send << "\n---------------\n";
#endif

    const bool async{ ! m_data.use_ssl || CONFIG::get_tls_nonblocking() };
    JDLIB::Socket soc( m_stop, async );
    soc.set_timeout( m_data.timeout );

    // socket接続
    const bool use_ipv6 = CONFIG::get_use_ipv6();
    if( m_data.host_proxy.empty() ) {
        if( ! soc.connect( m_data.host, std::to_string( m_data.port ), use_ipv6 ) ) {
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }
    else {
        if( ! soc.connect( m_data.host_proxy, std::to_string( m_data.port_proxy ), use_ipv6 ) ) {
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // 受信用バッファの割り当て
    m_buf.resize( m_lng_buf );

    // Socksのハンドシェイク
    if( const Proxy proxy{ m_data.protocol_proxy }; use_proxy && proxy != Proxy::http ) {

        if( ! soc.socks_handshake( m_data.host, std::to_string( m_data.port ), proxy ) ) {
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // HTTP tunneling
    else if( use_proxy && proxy == Proxy::http && m_data.use_ssl ) {

        // CONNECT
        std::string msg = "CONNECT ";
        msg += m_data.host + ":" + std::to_string( m_data.port ) + " HTTP/1.1\r\n";
        msg += "Host: " + m_data.host + "\r\n";
        msg += "Proxy-Connection: keep-alive\r\n";
        if( ! m_data.agent.empty() ) msg += "User-Agent: " + m_data.agent + "\r\n";
        msg += "\r\n";

        if( soc.write( msg.c_str(), msg.size() ) < 0 ) {

            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }

        // 読み込み
        std::size_t read_size = 0;
        while( read_size < m_lng_buf && ! m_stop.load( std::memory_order_acquire ) ) {

            const int tmpsize = soc.read( m_buf.data() + read_size, m_lng_buf - read_size );
            if( tmpsize < 0 ) {
                m_data.code = HTTP_ERR;
                errmsg = soc.get_errmsg();
                goto EXIT_LOADING;
            }

            else if( tmpsize == 0 ) goto EXIT_LOADING;
            else read_size += tmpsize;

            const auto header_parse = receive_header( m_buf.data(), read_size );
            if( header_parse == HeaderParse::failure ) {
                m_data.code = HTTP_ERR;
                errmsg = "invalid proxy header : " + m_data.url;
                errmsg.append( get_error_message( errno ) );
                goto EXIT_LOADING;
            }
            else if( header_parse == HeaderParse::success ) {
                if( m_data.code < 200 || m_data.code >= 300 ) {
                    goto EXIT_LOADING;
                }
                break;
            }
        }
    }

    // SSL/TLSのハンドシェイク
    if( m_data.use_ssl ) {

        if( ! soc.tls_handshake( m_data.host, CONFIG::get_verify_cert() ) ) {
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // HTTPリクエストの送信
    if( soc.write( msg_send.c_str(), msg_send.size() ) < 0 ) {

        m_data.code = HTTP_ERR;
        errmsg = soc.get_errmsg();
        goto EXIT_LOADING;
    }

#ifdef _DEBUG_TIME
    MISC::start_measurement( 1 );
#endif

    // 受信開始
    bool receiving_header; // C++ does not allow goto jumping across a definition with initialization.
    receiving_header = true;
    m_data.length_current = 0;
    m_data.size_data = 0;
    do {
        // 読み込み
        std::size_t read_size = 0; // m_buf に読み込んだデータの長さ
        // m_buf に格納したデータの末尾に '\0' は追加されないため read_size を使う
        while( read_size < m_lng_buf && ! m_stop.load( std::memory_order_acquire ) ) {

            const int tmpsize = soc.read( m_buf.data() + read_size, m_lng_buf - read_size );
            if( tmpsize < 0 ) {
                m_data.code = HTTP_ERR;
                errmsg = soc.get_errmsg();
                goto EXIT_LOADING;
            }

            if( tmpsize == 0 ) break;
            if( tmpsize > 0 ) {

                read_size += tmpsize;

                // ヘッダ取得
                if( receiving_header ) {

                    const auto header_parse = receive_header( m_buf.data(), read_size );
                    if( header_parse == HeaderParse::failure ) {

                        m_data.code = HTTP_ERR;
                        errmsg = "invalid header : " + m_data.url;
                        goto EXIT_LOADING;
                    }
                    else if( header_parse == HeaderParse::success ) receiving_header = false;
                }

                if( m_data.length && m_data.length <= m_data.length_current + read_size ) break;
            }

        }

        // 停止指定
        if( m_stop.load( std::memory_order_acquire ) ) break;

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

            if( ! m_chunk_decoder.decode( m_buf.data(), read_size ) ) {

                m_data.code = HTTP_ERR;
                errmsg = "ChunkedDecoder::decode() failed: " + m_data.url;
                goto EXIT_LOADING;
            }
            if( ! read_size ) break;
        }

        // 圧縮されていない時はそのままコールバック呼び出し
        if( ! m_gzip_decoder.is_decoding() ) {

            m_data.size_data += read_size;

            // コールバック呼び出し
            if( m_loadable ) m_loadable->receive( m_buf.data(), read_size );
        }

        // 圧縮されているときは unzip してからコールバック呼び出し
        else if( auto expan_size = m_gzip_decoder.feed( m_buf.data(), read_size );
                 expan_size.has_value() ) {

            m_data.size_data += *expan_size;
        }

        else {
            m_data.code = HTTP_ERR;
            errmsg = "unzip() failed";
            goto EXIT_LOADING;
        }

        if( m_data.length && m_data.length <= m_data.length_current ) break;
        if( m_use_chunk && m_chunk_decoder.is_completed() ) break;

    } while( ! m_stop.load( std::memory_order_acquire ) );

#ifdef _DEBUG_TIME
    std::cout << "Loader::run_main loading time(ns) = " << MISC::measurement( 1 ) << std::endl;
#endif

    // 終了処理
EXIT_LOADING:

    // 強制停止した場合
    if( m_stop.load( std::memory_order_acquire ) ) {
#ifdef _DEBUG
        std::cout << "Loader::run_main : stop loading\n";
#endif
        m_data.code = HTTP_TIMEOUT;
        m_data.modified.clear();
        m_data.str_code = "stop loading";
    }
    // エラーあり
    else if( ! errmsg.empty() ){
        m_data.modified.clear();
        MISC::ERRMSG( errmsg );
        m_data.str_code = std::move( errmsg );
    }

    // ソケットクローズ
    soc.close();

    // トークン返す
    return_token( this );

    finish_loading();

#ifdef _DEBUG
    std::cout << "Loader::run_main : finish loading : " << m_data.url << std::endl;
    std::cout << "read size : " << m_data.length_current << " / " << m_data.length << std::endl;
    std::cout << "data size : " << m_data.size_data << std::endl;
    std::cout << "code : " << m_data.code << std::endl << std::endl;
#endif    
}


//
// 送信メッセージ作成
//
std::string Loader::create_msg_send() const
{
    const bool post_msg{ ! m_data.str_post.empty() && ! m_data.head };
    const bool use_proxy{ ! m_data.host_proxy.empty() && ! m_data.use_ssl };

    std::ostringstream msg;

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
    if( ! m_data.origin.empty() ) msg << "Origin: " << m_data.origin << "\r\n";
    if( ! m_data.referer.empty() ) msg << "Referer: " << m_data.referer << "\r\n";

    // basic認証
    if( ! m_data.basicauth.empty() ) msg << "Authorization: Basic " << MISC::base64( m_data.basicauth ) << "\r\n";

    // proxy basic認証
    if( use_proxy && ! m_data.basicauth_proxy.empty() ) msg << "Proxy-Authorization: Basic " << MISC::base64( m_data.basicauth_proxy ) << "\r\n";

    if( ! m_data.cookie_for_request.empty() ) msg << "Cookie: " << m_data.cookie_for_request << "\r\n";

    if( ! m_data.modified.empty() ) msg << "If-Modified-Since: " << m_data.modified << "\r\n";

    if( ! m_data.accept.empty() ) msg << "Accept: " << m_data.accept << "\r\n";
    else msg << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
    msg << "Accept-Language: ja,en-US;q=0.7,en;q=0.3\r\n";

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
    msg << "Upgrade-Insecure-Requests: 1\r\n";

    // POST する文字列
    if( post_msg ){
        
        msg << "Content-Length: " << m_data.str_post.length() << "\r\n";
        msg << "\r\n";
        msg << m_data.str_post;
    }
    msg << "\r\n";
    
    return msg.str();
}


/** @brief サーバから送られてきた生データからヘッダ取得
 *
 * @param[in,out] buf        (in) 生データ, (out) ヘッダーが取り除かれたデータ
 * @param[in,out] read_size  (in) 生データサイズ, (out) 出力データーサイズ
 * @return ヘッダーの解析状況
 * @retval HeaderParse::success      成功
 * @retval HeaderParse::failure      失敗
 * @retval HeaderParse::not_finished 解析が終わってない
 */
Loader::HeaderParse Loader::receive_header( char* buf, std::size_t& read_size )
{
#ifdef _DEBUG
    std::cout << "Loader::receive_header : read_size = " << read_size << std::endl;
#endif

    m_data.str_header.assign( buf, read_size );
    std::size_t lng_header = m_data.str_header.find( "\r\n\r\n" );
    if( lng_header != std::string::npos ) lng_header += 4;
    else{

        lng_header = m_data.str_header.find( "\n\n" );
        if( lng_header != std::string::npos ) lng_header +=2;
        else return HeaderParse::not_finished;
    }
        
    m_data.str_header.resize( lng_header ); 

#ifdef _DEBUG    
    std::cout << "header : size = " << lng_header << " byte\n";
    std::cout << m_data.str_header << std::endl;
#endif

    if( ! analyze_header() ) return HeaderParse::failure;
                
    // 残りのデータを前に移動
    read_size -= lng_header;
    if( read_size ) std::memmove( buf, buf + lng_header, read_size );

    return HeaderParse::success;
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

    m_data.code = std::atoi( str_tmp.c_str() );
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
        || m_data.code == HTTP_MOVED_PERM
        || m_data.code == HTTP_PERMANENT_REDIRECT ) {

        m_data.location = analyze_header_option( "Location: " );
        // ルート相対パスならurlの上位部分で補って絶対パスにする
        if( ! m_data.location.empty() && m_data.location.front() == '/' ) {
            m_data.location = MISC::get_hostname( m_data.url, true ) + m_data.location;
        }
    }
    else m_data.location.clear();

    // Content-Type
    m_data.contenttype = analyze_header_option( "Content-Type: " );

    // chunked か
    m_use_chunk = false;
    str_tmp = analyze_header_option( "Transfer-Encoding: " );
    if( str_tmp.find( "chunked" ) != std::string::npos ){
        
        m_use_chunk = true;
        m_chunk_decoder.clear();
    }

    // gzip か
    m_gzip_decoder.clear();
    str_tmp = analyze_header_option( "Content-Encoding: " );
    if( str_tmp.find( "gzip" ) != std::string::npos ){
        std::function callback = [p = m_loadable]( const char* buf, std::size_t size )
        {
            p->receive( buf, size );
        };
        if( ! m_gzip_decoder.setup( m_lng_buf, std::move( callback ) ) ) return false;
    }

#ifdef _DEBUG
    std::cout << "code = " << m_data.code << std::endl;
    std::cout << "length = " << m_data.length << std::endl;    
    std::cout << "date = " << m_data.date << std::endl;
    std::cout << "modified = " << m_data.modified << std::endl;

    for( const std::string& s : m_data.list_cookies ) std::cout << "cookie " << s << std::endl;

    std::cout << "location = " << m_data.location << std::endl;
    std::cout << "contenttype = " << m_data.contenttype << std::endl;
    if( m_use_chunk ) std::cout << "m_use_chunk = true\n";
    if( m_gzip_decoder.is_decoding() )  std::cout << "m_use_gzip = true\n";

    std::cout << "authenticate = " << analyze_header_option( "WWW-Authenticate: " ) << std::endl;

    std::cout << "\n";
#endif
    
    return true;
}


//
// analyze_header() から呼ばれるオプションの値を取り出す関数
//
std::string Loader::analyze_header_option( std::string_view option ) const
{
    const std::size_t i = MISC::ascii_ignore_case_find( m_data.str_header, option );
    if( i != std::string::npos ){
        const std::size_t option_length = option.length();
        std::size_t i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( '\n', i );
        if( i2 != std::string::npos ) return m_data.str_header.substr( i + option_length, i2 - ( i + option_length ) );
    }

    return std::string();
}



//
// analyze_header() から呼ばれるオプションの値を取り出す関数(リスト版)
//
std::list< std::string > Loader::analyze_header_option_list( std::string_view option ) const
{
    std::list< std::string > str_list;

    const std::size_t option_length = option.length();

    std::size_t i2 = 0;
    for(;;){

        const std::size_t i = MISC::ascii_ignore_case_find( m_data.str_header, option, i2 );
        if( i == std::string::npos ) break;

        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( '\n', i );
        if( i2 == std::string::npos ) break;

        str_list.push_back( m_data.str_header.substr( i + option_length, i2 - ( i + option_length ) ) );
    }

    return str_list;
}



//
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
