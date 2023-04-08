// ライセンス: GPL2


using namespace JDLIB;


void JDLIB::tlslib_init()
{
    gnutls_global_init();
}


void JDLIB::tlslib_deinit()
{
    gnutls_global_deinit();
}


/** @brief ネットワークホストに対して SSL/TLS ハンドシェイクする
 *
 * @note ホストに接続した後に呼び出す必要がある
 * @param[in] hostname 接続するホスト
 * @param[in] verify   true ならサーバー証明書を検証する
 * @retval true  ハンドシェイクに成功した
 * @retval false ハンドシェイクに失敗した
 */
bool Socket::tls_handshake( const std::string& hostname, const bool verify )
{
#ifdef _DEBUG
    std::cout << "Socket::tls_handshake[GnuTLS]" << std::endl;
#endif
    using namespace std::string_literals;

    bool ret = false;
    int err = 0;

    m_errmsg.clear();

    if( ! soc_is_valid( m_soc ) ) {
        m_errmsg = "invalid socket";
        return ret;
    }

    if( m_tls != nullptr ) {
        m_errmsg = "duplicate fuction call";
        return ret;
    }

    m_terminated = false;

    if( ret = gnutls_init( &m_tls, GNUTLS_CLIENT | GNUTLS_NO_SIGNAL ); ret != 0 ) {
        m_errmsg = "gnutls_init failed";
        return false;
    }

    if( err = gnutls_certificate_allocate_credentials( &m_cred ); err != GNUTLS_E_SUCCESS ) {
        m_errmsg = "allocate cred failed: "s + gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }
    if( verify && ( err = gnutls_certificate_set_x509_system_trust( m_cred ) ) < 0 ) {
        m_errmsg = "set trust file failed: "s + gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    // allow the use of private ciphersuites. Server Name Indication (SNI)
    if( ! is_ipaddress( hostname.c_str() ) ) {
         gnutls_server_name_set( m_tls, GNUTLS_NAME_DNS, hostname.c_str(), hostname.size() );
    }

    if( err = gnutls_set_default_priority( m_tls ); err != GNUTLS_E_SUCCESS ) {
        m_errmsg = "set default priority failed: "s + gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }
    if( err = gnutls_credentials_set( m_tls, GNUTLS_CRD_CERTIFICATE, m_cred ); err != GNUTLS_E_SUCCESS ) {
        m_errmsg = "cred set failed: "s + gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    if( verify ) {
        gnutls_session_set_verify_cert( m_tls, hostname.c_str(), 0 );
    }

    gnutls_transport_set_int( m_tls, m_soc );
    gnutls_handshake_set_timeout( m_tls, m_tout * 1000 );

    while( ( err = gnutls_handshake( m_tls ) ) != GNUTLS_E_SUCCESS ) {

        if( gnutls_error_is_fatal( err ) != 0 ) {
            m_errmsg = "handshake failed: "s + gnutls_strerror( err );
            goto EXIT_TLS_CONNECT;
        }

        // writefds 待ち
        if( ! wait_fds( WaitFor::recv ) ) {
            m_errmsg = "handshake timeout";
            goto EXIT_TLS_CONNECT;
        }
    }

#ifdef _DEBUG
    std::cout << "Socket::tls_handshake[GnuTLS]: "
              << gnutls_protocol_get_name( gnutls_protocol_get_version( m_tls ) ) << " "
              << gnutls_cipher_suite_get_name( gnutls_kx_get( m_tls ), gnutls_cipher_get( m_tls ),
                                               gnutls_mac_get( m_tls ) )
              << std::endl;
#endif

    ret = true;

EXIT_TLS_CONNECT:

    if( ! m_errmsg.empty() && m_cred ) {
        gnutls_certificate_free_credentials( m_cred );
        m_cred = nullptr;
    }

    return ret;
}


/**
 * @brief SSL/TLS 接続を閉じる
 */
void Socket::tls_close()
{
    int ret;

    while( ( ret = gnutls_bye( m_tls, GNUTLS_SHUT_RDWR ) ) != GNUTLS_E_SUCCESS ) {

        if( ret != GNUTLS_E_AGAIN && ret != GNUTLS_E_INTERRUPTED ) {
            if( ! m_terminated ) {
                MISC::ERRMSG( std::string( "TLS shutdown failed: " ) + gnutls_strerror( ret ) );
            }
            break;
        }

        // readfds 待ち
        if( ! wait_fds( WaitFor::send ) ) {
            MISC::ERRMSG( "TLS shutdown timeout" );
            break;
        }
    }

    if( m_cred ) {
        gnutls_certificate_free_credentials( m_cred );
        m_cred = nullptr;
    }

    gnutls_deinit( m_tls );
    m_terminated = false;
    m_tls = nullptr;
}


/** @brief SSL/TLS ソケットにメッセージを送信する
 *
 * @param[in] buf     送信するメッセージ (not null)
 * @param[in] bufsize メッセージのサイズ
 * @return 送信したサイズ
 * @retval "< 0" 送信に失敗した or 接続がタイムアウトした
 */
int Socket::tls_write( const char* buf, const std::size_t bufsize )
{
    const ssize_t tmpsize = gnutls_record_send( m_tls, buf, bufsize );

    if( tmpsize < 0 ) {
        if( tmpsize != GNUTLS_E_AGAIN ) {
            m_errmsg = "gnutls_record_send failed: ";
            m_errmsg += gnutls_strerror( tmpsize );
        }

        // readfds 待ち
        else if( ! wait_fds( WaitFor::send ) ) {
            m_errmsg = "send timeout";
        }
    }
    return tmpsize; // possibly narrow cast
}


/** @brief SSL/TLS ソケットからメッセージを受信する
 *
 * @param[out] buf     メッセージを格納するバッファ (not null)
 * @param[in]  bufsize バッファのサイズ
 * @return 受信したサイズ
 * @retval "< 0" 受信に失敗した or 接続がタイムアウトした
 */
int Socket::tls_read( char* buf, const std::size_t bufsize )
{
    ssize_t ret = 0;
    if( m_terminated ) return ret;

    ret = gnutls_record_recv( m_tls, buf, bufsize );
    if( ret == GNUTLS_E_PREMATURE_TERMINATION ) {
        ret = 0;
        m_terminated = true;
    }
    else if( ret < 0 ) {
        if( ret != GNUTLS_E_AGAIN ) {
            m_errmsg = "gnutls_record_recv failed: ";
            m_errmsg += gnutls_strerror( ret );
        }

        // readfds 待ち
        else if( ! wait_fds( WaitFor::recv ) ) {
            m_errmsg = "receive timeout";
        }
    }
    return ret; // possibly narrow cast
}
