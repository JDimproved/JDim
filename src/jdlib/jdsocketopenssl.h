// ライセンス: GPL2
/** @file jdsocketopenssl.h
 *
 * @note 関数のドキュメンテーションは jdsocketgnutls.h を参照
 */

#include <openssl/err.h>
#include <openssl/conf.h>

#include <utility> // std::move


using namespace JDLIB;


void JDLIB::tlslib_init()
{
    OPENSSL_init_ssl( 0, nullptr );
}


void JDLIB::tlslib_deinit()
{
    // OPENSSL_cleanup() の呼び出しは自動的に処理される
}


static inline const char* ssllib_errstr()
{
    return ERR_reason_error_string( ERR_get_error() );
}


static std::string tls_get_errstr( const int error )
{
    switch( error ) {
        case SSL_ERROR_NONE:
            return "SSL_ERROR_NONE";
        case SSL_ERROR_ZERO_RETURN:
            return "SSL_ERROR_ZERO_RETURN";
        case SSL_ERROR_WANT_READ:
            return "SSL_ERROR_WANT_READ";
        case SSL_ERROR_WANT_WRITE:
            return "SSL_ERROR_WANT_WRITE";
        case SSL_ERROR_WANT_CONNECT:
            return "SSL_ERROR_WANT_CONNECT";
        case SSL_ERROR_WANT_ACCEPT:
            return "SSL_ERROR_WANT_ACCEPT";
        case SSL_ERROR_WANT_X509_LOOKUP:
            return "SSL_ERROR_WANT_X509_LOOKUP";
        case SSL_ERROR_SYSCALL:
            if( const char* reason = std::strerror( errno ); reason ) {
                return reason;
            }
            return "SSL_ERROR_SYSCALL";
        case SSL_ERROR_SSL:
            if( const char* reason = ERR_reason_error_string( ERR_get_error() ); reason ) {
                return reason;
            }
            return "SSL_ERROR_SSL";
    }
    return "unrecognized error";
}


bool Socket::tls_handshake( const std::string& hostname, const bool verify )
{
#ifdef _DEBUG
    std::cout << "Socket::tls_handshake[OpenSSL]" << std::endl;
#endif
    using namespace std::string_literals;

    auto failure = [this] ( std::string errmsg ) {
        m_errmsg = std::move( errmsg );
        return false;
    };

    m_errmsg.clear();

    if( ! soc_is_valid( m_soc ) ) {
        return failure( "invalid socket" );
    }

    if( m_tls != nullptr ) {
        return failure( "duplicate fuction call" );
    }

    if( m_ctx = SSL_CTX_new( TLS_client_method() ); ! m_ctx ) {
        return failure( "SSL_CTX_new failed: "s + ssllib_errstr() );
    }

    if( ! SSL_CTX_set_min_proto_version( m_ctx, TLS1_2_VERSION ) ) {
        return failure( "SSL_CTX_set_min_proto_version(TLS1_2_VERSION) failed: "s + ssllib_errstr() );
    }

    if( verify ) {
        // load the trusted client CA certificate into context
        if( SSL_CTX_set_default_verify_paths( m_ctx ) != 1 ) {
            return failure( "SSL_CTX_set_default_verify_paths failed: "s + ssllib_errstr() );
        }

        SSL_CTX_set_verify( m_ctx, SSL_VERIFY_PEER, nullptr );
        SSL_CTX_set_verify_depth( m_ctx, 4 );
    }

    if( ! m_tout ) SSL_CTX_set_timeout( m_ctx, m_tout );

    if( m_tls = SSL_new( m_ctx ); ! m_tls ) {
        return failure( "SSL_CTX_set_default_verify_paths failed: "s + ssllib_errstr() );
    }

    int err = 0;
    if( ! is_ipaddress( hostname.c_str() ) &&
            ( err = SSL_set_tlsext_host_name( m_tls, hostname.c_str() ) ) != 1 ) {
        return failure( "SSL_set_tlsext_host_name failed: "s + ssllib_errstr() );
    }

    if( err = SSL_set_fd( m_tls, m_soc ); ! err ) {
        return failure( "SSL_set_fd failed: "s + ssllib_errstr() );
    }

    SSL_set_connect_state( m_tls );

    while( ( err = SSL_do_handshake( m_tls ) ) != 1 ) {
        WaitFor want_read;

        err = SSL_get_error( m_tls, err );
        if( err == SSL_ERROR_WANT_READ ) want_read = WaitFor::recv;
        else if( err == SSL_ERROR_WANT_WRITE ) want_read = WaitFor::send;
        else {
            return failure( "SSL_do_handshake failed: " + tls_get_errstr( err ) );
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ) {
            return failure( "SSL_do_handshake timeout" );
        }
    }

#ifdef _DEBUG
    std::cout << "Socket::tls_handshake[OpenSSL]: " << SSL_get_version( m_tls ) << " "
              << SSL_get_cipher_name( m_tls ) << std::endl;;
#endif
    return true;
}


void Socket::tls_close()
{
    int ret;

    while( ( ret = SSL_shutdown( m_tls ) ) != 1 ) {
        WaitFor want_read = WaitFor::recv;

        if( ret != 0 ) {
            ret = SSL_get_error( m_tls, ret );
            if( ret == 0 || ret == SSL_ERROR_WANT_READ ) want_read = WaitFor::recv;
            else if( ret == SSL_ERROR_WANT_WRITE ) want_read = WaitFor::send;
            else {
                if( ret != SSL_ERROR_SYSCALL || errno != 0 ) {
                    MISC::ERRMSG( "SSL_shutdown failed: " + tls_get_errstr( ret ) );
                }
                break;
            }
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ) {
            MISC::ERRMSG( "SSL_shutdown timeout" );
            break;
        }
    }
    SSL_free( m_tls );
    SSL_CTX_free( m_ctx );

    m_tls = nullptr;
}


int Socket::tls_write( const char* buf, const std::size_t bufsize )
{
    const int tmpsize = SSL_write( m_tls, buf, bufsize );

    if( tmpsize < 0 ) {
        WaitFor want_read;

        const int ret = SSL_get_error( m_tls, tmpsize );
        if( ret == SSL_ERROR_WANT_READ ) want_read = WaitFor::recv;
        else if( ret == SSL_ERROR_WANT_WRITE ) want_read = WaitFor::send;
        else {
            m_errmsg = "SSL_write failed: " + tls_get_errstr( ret );
            return tmpsize;
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ) {
            m_errmsg = "SSL_write timeout";
        }
    }
    return tmpsize;
}


int Socket::tls_read( char* buf, const std::size_t bufsize )
{
    const int tmpsize = SSL_read( m_tls, buf, bufsize );

    if( tmpsize < 0 ) {
        WaitFor want_read;

        const int ret = SSL_get_error( m_tls, tmpsize );
        if( ret == SSL_ERROR_WANT_READ ) want_read = WaitFor::recv;
        else if( ret == SSL_ERROR_WANT_WRITE ) want_read = WaitFor::send;
        else {
            m_errmsg = "SSL_read failed: " + tls_get_errstr( ret );
            return tmpsize;
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ) {
            m_errmsg = "SSL_read timeout";
        }
    }
    return tmpsize;
}
