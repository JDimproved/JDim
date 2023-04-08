// ライセンス: GPL2


#ifndef JDSOCKET_H
#define JDSOCKET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(USE_GNUTLS)
#include <gnutls/gnutls.h>
#elif defined(USE_OPENSSL)
#include <openssl/ssl.h>
#else
#error "No TLS library specified."
#endif

#include <atomic>
#include <string>


namespace JDLIB
{
    /// @brief プロキシ プロトコルの種類
    enum class Proxy
    {
        none = 0,
        http,
        socks4,
        socks4a,
    };

    /** @brief ソケットの通信や SSL/TLS の処理を行うクラス
     *
     * @details JDLIB::Loader クラスの通信を行うスレッドの中で使う。
     * @note Socketの生存期間中に参照で保持する変数(m_stop)の寿命が尽きないように注意する
     */
    class Socket
    {
        /// @brief 非同期通信の待機するモード
        enum class WaitFor
        {
            send, ///< 送信を待つ
            recv, ///< 受信を待つ
        };

        std::string m_errmsg;
        long m_tout{}; ///< タイムアウト時間 (単位は秒)
        int m_soc;     ///< ソケットのファイルディスクリプター
        const std::atomic<bool>& m_stop; ///< 通信を行うスレッドの中断をチェックする (読み取り専用)
        bool m_async;  ///< true なら非同期通信を行う

#if defined(USE_GNUTLS)
        gnutls_session_t m_tls{};
        gnutls_certificate_credentials_t m_cred{};
        bool m_terminated{};
#elif defined(USE_OPENSSL)
        SSL_CTX* m_ctx{};
        SSL* m_tls{};
#endif

    public:

        explicit Socket( const std::atomic<bool>& stop, const bool async = true );
        ~Socket();

        std::string& get_errmsg() { return m_errmsg; }
        /// @brief タイムアウト時間 (単位は秒) を設定する
        void set_timeout( long timeout ) { m_tout = timeout; }

        bool connect( const std::string& host, const std::string& port, const bool use_ipv6 );
        void close();

        int write( const char* buf, const std::size_t bufsize );
        int read( char* buf, const std::size_t bufsize );

        bool tls_handshake( const std::string& hostname, const bool verify );
        bool socks_handshake( const std::string& hostname, const std::string& port, const Proxy protocol );

    private:

        int tls_write( const char* buf, const std::size_t bufsize );
        int tls_read( char* buf, const std::size_t bufsize );
        void tls_close();

        bool wait_fds( const WaitFor operation );
    };

    /// @brief Initialize SSL/TLS library
    void tlslib_init();
    /// @brief Deinitialize SSL/TLS library
    void tlslib_deinit();
}

#endif
