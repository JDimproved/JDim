// ライセンス: GPL2

//
// 2chへのログイン管理クラス
//
// セッション管理やログイン、パスワードの保存などを行う
//

#ifndef _LOGIN2CH_H
#define _LOGIN2CH_H

#include "skeleton/login.h"

namespace CORE
{
    class Login2ch : public SKELETON::Login
    {
        char* m_rawdata;
        int m_lng_rawdata;

      public:

        Login2ch();
        virtual ~Login2ch();

        virtual void start_login();
        virtual void logout();

      private:

        virtual void receive_data( const char* , size_t );
        virtual void receive_finish();
    };


    Login2ch* get_login2ch();
    void delete_login2ch();
}


#endif
