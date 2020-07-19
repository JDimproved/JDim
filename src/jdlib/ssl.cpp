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
    : m_session( nullptr ),
      m_cred( nullptr )
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
    : m_ctx( nullptr ),
      m_ssl( nullptr )
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


bool JDSSL::connect( const int soc, const char *host )
{
#ifdef _DEBUG
    std::cout << "JDSSL::connect(openssl)\n";
#endif

    if( soc < 0 ) return false;
    if( m_ctx ) return false;
    if( m_ssl ) return false;

    SSL_library_init();
    m_ctx = SSL_CTX_new( SSLv23_client_method() );
    if( ! m_ctx ){
        m_errmsg = "SSL_CTX_new failed";
        return false;
    }

    m_ssl = SSL_new( m_ctx );
    if( ! m_ssl ){
        m_errmsg = "SSL_new failed";
        return false;
    }

    if( SSL_set_fd( m_ssl, soc ) == 0 ){
        m_errmsg = "SSL_set_fd failed";
        return false;
    }

    SSL_set_tlsext_host_name( m_ssl, host ) ;
    if( SSL_connect( m_ssl ) != 1 ){
        m_errmsg = "SSL_connect failed";
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
        SSL_shutdown( m_ssl );
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
    int tmpsize = SSL_write( m_ssl, buf, bufsize );
    if( tmpsize < 0 ) m_errmsg = "SSL_write failed";

    return tmpsize;
}


int JDSSL::read( char* buf, const size_t bufsize )
{
    int tmpsize = SSL_read( m_ssl, buf, bufsize );
    if( tmpsize < 0 ) m_errmsg = "SSL_read failed";

    return tmpsize;
}

#endif
