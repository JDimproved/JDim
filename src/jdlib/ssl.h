// ライセンス: GPL2

//
// SSL ローダ
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_GNUTLS
#include <gnutls/gnutls.h>
#else
#include <openssl/ssl.h>
#endif

#include <string>

namespace JDLIB
{
    class JDSSL
    {
        std::string m_errmsg;

#ifdef USE_GNUTLS
        gnutls_session_t m_session;
        gnutls_certificate_credentials  m_cred;
#else
        SSL_CTX *m_ctx;
        SSL* m_ssl;
#endif

      public:

        JDSSL();
        virtual ~JDSSL();

        const std::string& get_errmsg(){ return m_errmsg; }

        const bool connect( const int soc );
        const bool close();

        const int write( const char* buf, const size_t bufsize );
        const int read( char* buf, const size_t bufsize );
    };

    void init_ssl();
    void deinit_ssl();
}
