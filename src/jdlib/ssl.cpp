// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "ssl.h"

using namespace JDLIB;


#ifdef USE_GNUTLS

#include <cstring>

// gnutls 使用

void JDLIB::init_ssl()
{
#ifdef _DEBUG
    std::cout << "init_ssl(gnutls)\n";
#endif

    gnutls_global_init();
}

void JDLIB::deinit_ssl()
{
#ifdef _DEBUG
    std::cout << "deinit_ssl(gnutls)\n";
#endif

    gnutls_global_deinit();
}


JDSSL::JDSSL()
{
#ifdef _DEBUG
    std::cout << "JDSSL::JDSSL(gnutls)\n";
#endif
}


JDSSL::~JDSSL()
{
#ifdef _DEBUG
    std::cout << "JDSSL::~JDSSL(gnutls)\n";
#endif

    close();
}


bool JDSSL::connect( const int soc, const char *host )
{
#ifdef _DEBUG
    std::cout << "JDSSL::connect(gnutls)\n";
#endif

    if( soc < 0 ) return false;
    if( m_session ) return false;
    if( m_cred ) return false;

    int ret;

    ret = gnutls_init( &m_session, GNUTLS_CLIENT );
    if( ret != 0 ){
        m_errmsg = "gnutls_init failed";
        return false;
    }

    auto failure = [this]( int err ) { m_errmsg = gnutls_strerror( err ); return false; };

    ret = gnutls_certificate_allocate_credentials( &m_cred );
    if( ret != GNUTLS_E_SUCCESS ) return failure( ret );
    ret = gnutls_certificate_set_x509_system_trust( m_cred );
    if( ret < 0 ) return failure( ret );

    ret = gnutls_server_name_set( m_session, GNUTLS_NAME_DNS, host, strlen( host ) );
    if( ret != GNUTLS_E_SUCCESS ) return failure( ret );

    ret = gnutls_set_default_priority( m_session );
    if( ret != GNUTLS_E_SUCCESS ) return failure( ret );
    ret = gnutls_credentials_set( m_session, GNUTLS_CRD_CERTIFICATE, m_cred );
    if( ret != GNUTLS_E_SUCCESS ) return failure( ret );

    gnutls_session_set_verify_cert( m_session, host, 0 );

    gnutls_transport_set_int( m_session, soc );
    gnutls_handshake_set_timeout( m_session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT );

    do {
        ret = gnutls_handshake( m_session );
    } while( ret < 0 && gnutls_error_is_fatal( ret ) == 0 );

    if( ret < 0 ) {
        m_errmsg = "JDSSL::connect(gnutls) *** Handshake failed: ";
        m_errmsg += gnutls_strerror( ret );

        if( ret == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR ) {
            gnutls_certificate_type_t type = gnutls_certificate_type_get( m_session );
            const unsigned status = gnutls_session_get_verify_cert_status( m_session );
            gnutls_datum_t out;
            ret = gnutls_certificate_verification_status_print( status, type, &out, 0 );
            if( ret == GNUTLS_E_SUCCESS ) {
                m_errmsg += "\nJDSSL::connect(gnutls) - cert verify output: ";
                m_errmsg += reinterpret_cast< const char* >( out.data );
                gnutls_free( out.data );
            }
        }
        return false;
    }

#ifdef _DEBUG
    char* desc = gnutls_session_get_desc( m_session );
    assert( desc );
    std::cout << "JDSSL::connect(gnutls) - Session info: " << desc << std::endl;
    gnutls_free( desc );
#endif // _DEBUG

    return true;
}


bool JDSSL::close()
{
#ifdef _DEBUG
    std::cout << "JDSSL::close(gnutls)\n";
#endif

    if( m_session ){
        gnutls_bye( m_session, GNUTLS_SHUT_RDWR );
        gnutls_deinit( m_session );
        m_session = nullptr;
    }
    if( m_cred ){
        gnutls_certificate_free_credentials( m_cred );
        m_cred = nullptr;
    }

    return true;
}


int JDSSL::write( const char* buf, const size_t bufsize )
{
    int tmpsize;
    do {
        tmpsize = gnutls_record_send( m_session, buf, bufsize );
    } while( tmpsize == GNUTLS_E_AGAIN || tmpsize == GNUTLS_E_INTERRUPTED );

#ifdef _DEBUG
    std::cout << "JDSSL::write(gnutls) tmpsize = " << tmpsize << "; bufsize = " << bufsize << std::endl;
#endif

    if( tmpsize < 0 ) m_errmsg = "gnutls_record_send failed";

    return tmpsize;
}


int JDSSL::read( char* buf, const size_t bufsize )
{
    int tmpsize;
    do {
        tmpsize = gnutls_record_recv( m_session, buf, bufsize );
    } while( tmpsize == GNUTLS_E_AGAIN || tmpsize == GNUTLS_E_INTERRUPTED );

#ifdef _DEBUG
    std::cout << "JDSSL::read(gnutls) tmpsize = " << tmpsize << "; bufsize = " << bufsize << std::endl;
#endif

    if( tmpsize == GNUTLS_E_PREMATURE_TERMINATION || tmpsize == GNUTLS_E_INVALID_SESSION ) {
        // Transfer-Encoding: chuncked のときはデータの長さが分からない
        // そのため受信エラーをデータ無しに置き換えて受信終了を判断する
        return 0;
    }
    if( tmpsize < 0 ) m_errmsg = "gnutls_record_recv failed";

    return tmpsize;
}

#else ////////////////////////////////////////////////////////////////////

// OpenSSL 使用

#include "miscmsg.h"

#include <openssl/err.h>

#include <cstring>


void JDLIB::init_ssl()
{
#ifdef _DEBUG
    std::cout << "init_ssl(openssl)\n";
#endif
}

void JDLIB::deinit_ssl()
{
#ifdef _DEBUG
    std::cout << "deinit_ssl(openssl)\n";
#endif
}


JDSSL::JDSSL()
{
#ifdef _DEBUG
    std::cout << "JDSSL::JDSSL(openssl)\n";
#endif
}


JDSSL::~JDSSL()
{
#ifdef _DEBUG
    std::cout << "JDSSL::~JDSSL(openssl)\n";
#endif

    close();
}


/// SSL_get_error() の戻り値からエラーメッセージを返す
static const char* openssl_error_string( int error )
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


bool JDSSL::connect( const int soc, const char *host )
{
#ifdef _DEBUG
    std::cout << "JDSSL::connect(openssl)\n";
#endif
    constexpr const int success = 1;

    if( soc < 0 ) return false;
    if( m_ctx ) return false;
    if( m_ssl ) return false;

    auto failure = [this]( const char* err ) {
        m_errmsg.assign( err );
        m_errmsg.append( openssl_error_string( SSL_ERROR_SSL ) );
        return false;
    };

    if( m_ctx = SSL_CTX_new( TLS_client_method() ); ! m_ctx ) {
        return failure( "SSL_CTX_new failed: " );
    }
    if( SSL_CTX_set_min_proto_version( m_ctx, TLS1_2_VERSION ) != success ) {
        return failure( "SSL_CTX_set_min_proto_version(TLS1_2_VERSION) failed: " );
    }

    // load the trusted client CA certificate into context
    if( SSL_CTX_set_default_verify_paths( m_ctx ) != success ){
        return failure( "SSL_CTX_set_default_verify_paths failed: " );
    }
    SSL_CTX_set_verify( m_ctx, SSL_VERIFY_PEER, nullptr );

    if( m_ssl = SSL_new( m_ctx ); ! m_ssl ) {
        return failure( "SSL_new failed: " );
    }
    if( SSL_set_fd( m_ssl, soc ) != success ) {
        return failure( "SSL_set_fd failed: " );
    }
    if( SSL_set_tlsext_host_name( m_ssl, host ) != success ) {
        return failure( "SSL_set_tlsext_host_name failed: " );
    }

    if( const int err = SSL_connect( m_ssl ); err != success ) {
        m_errmsg.assign( "SSL_connect failed: " );
        m_errmsg.append( openssl_error_string( SSL_get_error( m_ssl, err ) ) );
        return false;
    }

#ifdef _DEBUG
    std::cout << "connect ok\n";
#endif

    return true;
}


bool JDSSL::close()
{
#ifdef _DEBUG
    std::cout << "JDSSL::close(openssl)\n";
#endif

    if( m_ssl ){
        int err;
        // SSL_shutdown() returning 0 does not indicate an error.
        if( ! SSL_in_init( m_ssl ) && ( err = SSL_shutdown( m_ssl ) ) < 0 ) {
            m_errmsg.assign( "SSL_shutdown failed: " );
            m_errmsg.append( openssl_error_string( SSL_get_error( m_ssl, err ) ) );
            MISC::ERRMSG( m_errmsg );
        }
        SSL_free( m_ssl );
        m_ssl = nullptr;
    }
    if( m_ctx ){
        SSL_CTX_free( m_ctx );
        m_ctx = nullptr;
    }

    return true;
}


int JDSSL::write( const char* buf, const size_t bufsize )
{
    int tmpsize;
    while(1) {
        tmpsize = SSL_write( m_ssl, buf, bufsize );
        if( tmpsize > 0 ) break;

        const int err = SSL_get_error( m_ssl, tmpsize );
        // TLSハンドシェイクの処理はプロトコル中いつでも発生する可能性がある
        if( err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ ) continue;

        m_errmsg.assign( "SSL_write failed: " );
        m_errmsg.append( openssl_error_string( err ) );
        break;
    }
    return tmpsize;
}


int JDSSL::read( char* buf, const size_t bufsize )
{
    int tmpsize;
    while(1) {
        tmpsize = SSL_read( m_ssl, buf, bufsize );
        if( tmpsize > 0 ) break;

        const int err = SSL_get_error( m_ssl, tmpsize );
        // TLSハンドシェイクの処理はプロトコル中いつでも発生する可能性がある
        if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ) continue;

        m_errmsg.assign( "SSL_read failed: " );
        m_errmsg.append( openssl_error_string( err ) );
        break;
    }
    return tmpsize;
}

#endif
