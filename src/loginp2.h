// ライセンス: GPL2

//
// p2へのログイン管理クラス
//
// セッション管理やログイン、パスワードの保存などを行う
//

#ifndef _LOGINP2_H
#define _LOGINP2_H

#include "skeleton/login.h"

namespace CORE
{
    class Loginp2 : public SKELETON::Login
    {
        char* m_rawdata;
        int m_lng_rawdata;

        bool m_loading_csrfid;  // ( menu.php から ) csrfid 取得中

      public:

        Loginp2();
        virtual ~Loginp2();

        virtual void start_login();
        virtual void logout();

      private:

        virtual void receive_data( const char* , size_t );
        virtual void receive_finish();
    };


    Loginp2* get_loginp2();
    void delete_loginp2();
}


#endif
