// ライセンス: 最新のGPL

//
// 2chなどへのログイン管理クラス
//
// セッション管理やログイン、パスワードの保存などを行う
//

#ifndef _LOGIN_H
#define _LOGIN_H

#include "loadable.h"

namespace SKELETON
{
    class Login : public SKELETON::Loadable
    {
        std::string m_url;
        bool m_login_now;
        bool m_save_info;

        std::string m_username;
        std::string m_passwd;

        std::string m_sessionid;

      public:

        Login( const std::string& url );
        virtual ~Login();

        const bool login_now() const { return m_login_now; }
        void set_login_now( bool login_now ){ m_login_now = login_now; }

        const std::string& get_url() const { return m_url; }

        const std::string& get_username() const { return m_username; }
        void set_username( const std::string& username );

        const std::string& get_passwd() const { return m_passwd; }
        void set_passwd( const std::string& passwd );

        const std::string& get_sessionid() const { return m_sessionid; }
        void set_sessionid( const std::string& sessionid ){ m_sessionid = sessionid; }

        virtual void start_login(){}
        virtual void logout(){}

      private:
        void read_info();
        void save_info();
    };
}


#endif
