// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loginbe.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"
#include "command.h"

#include "config/globalconf.h"

#include "skeleton/msgdiag.h"

#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"


namespace CORE::be {
constexpr std::size_t kSizeOfRawData = 64 * 1024;
}


CORE::LoginBe* instance_loginbe = nullptr;

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
    instance_loginbe = nullptr;
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
    if( is_loading() ) return;

    SKELETON::Login::set_login_now( false );
    SKELETON::Login::set_sessionid( std::string() );
    SKELETON::Login::set_sessiondata( std::string() );
    SESSION::set_loginbe( false );
}


//
// ログイン開始
//
void LoginBe::start_login()
{
    if( is_loading() ) return;

#ifdef _DEBUG
    std::cout << "LoginBe::start_login  url = " << CONFIG::get_url_loginbe() << std::endl;
#endif 

    set_str_code( "" );

    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( nullptr, "オフラインです" );
        mdiag.run();
        return;
    }

    if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "メールアドレスまたはパスワードが設定されていません\n\n設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
        return;
    }

    if( CONFIG::get_url_loginbe().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "BEの認証サーバのアドレスが指定されていません。" );
        mdiag.run();
        return;
    }

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = CONFIG::get_url_loginbe();

    data.contenttype = "application/x-www-form-urlencoded";
    data.str_post = "m=" + MISC::url_encode_plus( get_username() );
    data.str_post += "&p=" + MISC::url_encode_plus( get_passwd() );
    data.str_post += "&submit=" + MISC::url_encode_plus( "登録", Encoding::eucjp );

    logout();
    if( m_rawdata.capacity() < be::kSizeOfRawData ) m_rawdata.reserve( be::kSizeOfRawData );
    m_rawdata.clear();

    start_load( data );
}


//
// データ受信
//
void LoginBe::receive_data( std::string_view buf )
{
#ifdef _DEBUG
    std::cout << "LoginBe::receive_data\n";
#endif

    m_rawdata.append( buf );
}


//
// データ受信完了
//
void LoginBe::receive_finish()
{
#ifdef _DEBUG
    std::cout << "LoginBe::receive_finish code = " << get_code() << std::endl;
    std::cout << "rawdata size = " << m_rawdata.size() << std::endl;
    std::cout << m_rawdata << std::endl;
#endif

    std::string dmdm;
    std::string mdmd;

    if( get_code() == HTTP_OK ){

        for( const std::string& cookie : cookies() ) {

#ifdef _DEBUG
            std::cout << cookie << std::endl;
#endif

            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::string query = "DMDM=([^;]*)";
            if( regex.exec( query, cookie, offset, icase, newline, usemigemo, wchar ) ) dmdm = regex.str( 1 );

            query = "MDMD=([^;]*)";
            if( regex.exec( query, cookie, offset, icase, newline, usemigemo, wchar ) ) mdmd = regex.str( 1 );
        }
    }

#ifdef _DEBUG
    std::cout << "dmdm = " << dmdm << " mdmd = " << mdmd << std::endl;
#endif

    if( ! dmdm.empty() && ! mdmd.empty() ){
        set_login_now( true );
        set_sessionid( dmdm );
        set_sessiondata( mdmd );
        SESSION::set_loginbe( true );

        CORE::core_set_command( "loginbe_finished", "" );
    }
    else{

        std::string str_err = "ログインに失敗しました。\n\nBEの認証サーバのアドレスやメールアドレス、パスワード等を確認して下さい。\n\n";
        str_err += get_str_code();
        SKELETON::MsgDiag mdiag( nullptr, str_err );
        mdiag.run();  
    }
}
