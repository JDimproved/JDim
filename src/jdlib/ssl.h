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

// gdkmm/device.h で定義される set_key マクロと衝突する
#ifdef set_key
#undef set_key
#endif

#endif

#include <string>

namespace JDLIB
{
    class JDSSL
    {
        std::string m_errmsg;

#ifdef USE_GNUTLS
        gnutls_session_t m_session;

#if GNUTLS_VERSION_NUMBER >= 0x020000
        gnutls_certificate_credentials_t m_cred;
#else // GNUTLS_VERSION_NUMBER >= 0x020000
        // DEPRECATED (gnutls >= 2.x)
        gnutls_certificate_credentials m_cred;
#endif // GNUTLS_VERSION_NUMBER >= 0x020000

#else // USE_GNUTLS
        SSL_CTX *m_ctx;
        SSL* m_ssl;
#endif // USE_GNUTLS

      public:

        JDSSL();
        virtual ~JDSSL();

        const std::string& get_errmsg(){ return m_errmsg; }

        bool connect( const int soc, const char* host );
        bool close();

        int write( const char* buf, const size_t bufsize );
        int read( char* buf, const size_t bufsize );
    };

    void init_ssl();
    void deinit_ssl();
}
