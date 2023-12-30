// ライセンス: GPL2

//
// 2chへのログイン管理クラス
//
// セッション管理やログイン、パスワードの保存などを行う
//

#ifndef _LOGIN2CH_H
#define _LOGIN2CH_H

#include "skeleton/login.h"

#include <string>
#include <string_view>


namespace CORE
{
    class Login2ch : public SKELETON::Login
    {
        std::string m_rawdata;

      public:

        Login2ch();
        ~Login2ch() override = default;

        void start_login() override;
        void logout() override;

      private:

        void receive_data( std::string_view buf ) override;
        void receive_finish() override;
    };


    Login2ch* get_login2ch();
    void delete_login2ch();
}


#endif
