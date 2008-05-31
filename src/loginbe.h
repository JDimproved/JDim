// ライセンス: GPL2

//
// BEログイン管理クラス
//
// セッション管理やログイン、パスワードの保存などを行う
//

#ifndef _LOGINBE_H
#define _LOGINBE_H

#include "skeleton/login.h"

namespace CORE
{
    class LoginBe : public SKELETON::Login
    {
      public:

        LoginBe();

        virtual void start_login();
        virtual void logout();
    };


    LoginBe* get_loginbe();
    void delete_loginbe();
}


#endif
