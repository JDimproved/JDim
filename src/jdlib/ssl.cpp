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
    : m_session( 0 ),
      m_cred( 0 )
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

#if GNUTLS_VERSION_NUMBER >= 0x020108
    // gnutls >= 2.1.7 (unreleased)
    gnutls_priority_set_direct( m_session, "NORMAL:%COMPAT", NULL );
#else // GNUTLS_VERSION_NUMBER >= 0x020108
    static const int priority_prot[] = { GNUTLS_SSL3, 0 };
    // DEPRECATED (gnutls >= 2.1.4 gnutls =< 2.1.6)
    // UNDEPRECATED (gnutls >= 2.1.7)
    gnutls_set_default_priority( m_session );
    // _GNUTLS_GCC_ATTR_DEPRECATE (gnutls >= 2.12.0)
    gnutls_protocol_set_priority( m_session, priority_prot );
#endif // GNUTLS_VERSION_NUMBER >= 0x020108

    gnutls_transport_set_ptr( m_session, (gnutls_transport_ptr_t)(long) soc );
    gnutls_certificate_allocate_credentials( &m_cred );
    gnutls_credentials_set( m_session, GNUTLS_CRD_CERTIFICATE, m_cred );
    gnutls_server_name_set( m_session, GNUTLS_NAME_DNS, host, strlen( host ) );

    while ( ( ret = gnutls_handshake( m_session ) ) != GNUTLS_E_SUCCESS )
    {
        if ( gnutls_error_is_fatal( ret ) != 0 )
        {
            m_errmsg = "gnutls_handshake failed";
            return false;
        }
    }

#ifdef _DEBUG
    std::cout << "connect ok\n";
#endif

    return true;
}


bool JDSSL::close()
{
#ifdef _DEBUG
    std::cout << "JDSSL::close(gnutlsl)\n";
#endif

    if( m_session ){
        gnutls_bye( m_session, GNUTLS_SHUT_RDWR );
        gnutls_deinit( m_session );
        m_session = 0;
    }
    if( m_cred ){
        gnutls_certificate_free_credentials( m_cred );
        m_cred = 0;
    }

    return true;
}


int JDSSL::write( const char* buf, const size_t bufsize )
{
    int tmpsize = gnutls_record_send( m_session, buf, bufsize );
    if( tmpsize < 0 ) m_errmsg = "gnutls_record_send failed";

    return tmpsize;
}


int JDSSL::read( char* buf, const size_t bufsize )
{
    int tmpsize = gnutls_record_recv( m_session, buf, bufsize );
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
    : m_ctx( NULL ),
      m_ssl( NULL )
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
        m_ssl = NULL;
    }
    if( m_ctx ){
        SSL_CTX_free( m_ctx );
        m_ctx = NULL;
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
