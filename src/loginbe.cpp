// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loginbe.h"
#include "global.h"
#include "session.h"

#include "skeleton/msgdiag.h"

CORE::LoginBe* instance_loginbe = NULL;

CORE::LoginBe* CORE::get_loginbe()
{
    if( ! instance_loginbe ) instance_loginbe = new CORE::LoginBe();
    assert( instance_loginbe );

    return instance_loginbe;
}


void CORE::delete_loginbe()
{
    if( instance_loginbe ){
        instance_loginbe->terminate_load();
        delete instance_loginbe;
    }
    instance_loginbe = NULL;
}


using namespace CORE;


LoginBe::LoginBe()
    : SKELETON::Login( URL_LOGINBE )
{
#ifdef _DEBUG
    std::cout << "LoginBe::LoginBe\n";
#endif
}


//
// ログアウト
//
void LoginBe::logout()
{
#ifdef _DEBUG
    std::cout << "LoginBe::logout\n";
#endif

    SKELETON::Login::set_login_now( false );
    SESSION::set_loginbe( false );
}


//
// ログイン開始
//
void LoginBe::start_login()
{
    if( login_now() ) return;

#ifdef _DEBUG
    std::cout << "LoginBe::start_login\n";
#endif 

    if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( NULL, "メールアドレスまたは認証コード設定されていません\n\n設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
        return;
    }

    set_login_now( true );
    SESSION::set_loginbe( true );
}
