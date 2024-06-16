// SPDX-License-Identifier: GPL-2.0-or-later

//#define _DEBUG
#include "jddebug.h"

#include "loginacorn.h"

#include "command.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"

#include "config/globalconf.h"
#include "jdlib/cookiemanager.h"
#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"
#include "skeleton/msgdiag.h"


namespace CORE::ac {
constexpr std::size_t kSizeOfRawData = 64 * 1024;

constexpr const char* kHostDonguri = "donguri.5ch.net";
constexpr const char* kOriginDonguri = "https://donguri.5ch.net";
}


static CORE::LoginAcorn* instance_loginacorn = nullptr;


CORE::LoginAcorn* CORE::get_loginacorn()
{
    if( ! instance_loginacorn ) instance_loginacorn = new CORE::LoginAcorn();
    assert( instance_loginacorn );

    return instance_loginacorn;
}


void CORE::delete_loginacorn()
{
    if( instance_loginacorn ){
        instance_loginacorn->terminate_load();
        delete instance_loginacorn;
    }
    instance_loginacorn = nullptr;
}


using namespace CORE;


LoginAcorn::LoginAcorn()
    : SKELETON::Login( URL_LOGINACORN )
{
}


/**
 * @brief ログイン状態をリセットする
 */
void LoginAcorn::reset()
{
    SKELETON::Login::set_login_now( false );
    SKELETON::Login::set_sessionid( std::string() );
    SKELETON::Login::set_sessiondata( std::string() );
    SESSION::set_loginacorn( false );
}

/**
 * @brief ログアウト
 */
void LoginAcorn::logout()
{
#ifdef _DEBUG
    std::cout << "LoginAcorn::logout\n";
#endif
    if( is_loading() ) return;

    reset();

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = std::string{ ac::kOriginDonguri } + "/logout";
    data.referer = std::string{ ac::kOriginDonguri } + "/";
    data.agent = CONFIG::get_agent_for2ch();

    const JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
    data.cookie_for_request = cookie_manager->get_cookie_by_host( ac::kHostDonguri );

    m_rawdata.clear();
    m_operation = Operation::logout;

    start_load( data );
}


/**
 * @brief ログイン開始
 */
void LoginAcorn::start_login()
{
    if( is_loading() ) return;

    std::string login_url = std::string{ ac::kOriginDonguri } + "/login";
#ifdef _DEBUG
    std::cout << "LoginAcorn::start_login  url = " << login_url << std::endl;
#endif

    set_str_code( "" );

    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( nullptr, "オフラインです" );
        mdiag.run();
        return;
    }

    if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "メールアドレスまたはパスワードが設定されていません\n\n"
                                 "設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
        return;
    }

    reset();

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = std::move( login_url );
    data.origin = ac::kOriginDonguri;
    data.referer = std::string{ ac::kOriginDonguri } + "/";
    data.agent = CONFIG::get_agent_for2ch();

    data.contenttype = "application/x-www-form-urlencoded";
    data.str_post = "email=" + MISC::url_encode_plus( get_username() );
    data.str_post += "&pass=" + MISC::url_encode_plus( get_passwd() );

    if( m_rawdata.capacity() < ac::kSizeOfRawData ) m_rawdata.reserve( ac::kSizeOfRawData );
    m_rawdata.clear();
    m_operation = Operation::login;

    start_load( data );
}


/**
 * @brief データ受信
 */
void LoginAcorn::receive_data( std::string_view buf )
{
    m_rawdata.append( buf );
}


/**
 * @brief データ受信完了
 */
void LoginAcorn::receive_finish()
{
#ifdef _DEBUG
    std::cout << "LoginAcorn::receive_finish code = " << get_code() << std::endl;
    std::cout << "rawdata size = " << m_rawdata.size() << std::endl;
#endif

    // ログイン、ログアウトでクッキーを更新する
    if( get_code() == HTTP_REDIRECT ) {

        receive_finish_redirect();
    }
    // どんぐりベースからアカウント名とIDを取得する
    else if( get_code() == HTTP_OK ) {

        receive_finish_ok();
        return;
    }

    if( m_operation != Operation::login ) return;

    std::string str_err = "ログインに失敗しました。\n\nどんぐりシステムの"
                          "認証サーバのアドレスやメールアドレス、パスワード等を確認して下さい。\n\n";
    str_err += get_str_code();
    SKELETON::MsgDiag mdiag( nullptr, str_err );
    mdiag.run();
}


/**
 * @brief データ受信完了 (リダイレクト)
 *
 * @details ログインとログアウト時にHTTPクッキーを更新する。
 */
void LoginAcorn::receive_finish_redirect()
{
    std::string acorn;

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;
    constexpr bool icase = false;
    constexpr bool newline = true;
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    for( const std::string& cookie : cookies() ) {

        const std::string pattern = "acorn=([^;]*)";
        if( regex.exec( pattern, cookie, offset, icase, newline, usemigemo, wchar ) ) {
            acorn = regex.str( 1 );

            JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
            cookie_manager->feed( ac::kHostDonguri, MISC::utf8_trim( cookie ) );
        }
    }

    if( acorn.empty() ) return;

    set_login_now( true );
    set_sessiondata( acorn );
    SESSION::set_loginacorn( true );

    CORE::core_set_command( "loginacorn_finished", "" );

    // どんぐりベースにアクセスしてアカウント名とIDを取得する
    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = std::string{ ac::kOriginDonguri } + "/";
    data.referer = data.url;
    data.agent = CONFIG::get_agent_for2ch();

    const JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
    data.cookie_for_request = cookie_manager->get_cookie_by_host( ac::kHostDonguri );

    m_rawdata.clear();
    m_operation = Operation::status;

    start_load( data );
}


/**
 * @brief データ受信完了 (200 OK)
 *
 * @details どんぐりベースからアカウント名とIDを取得する。
 */
void LoginAcorn::receive_finish_ok()
{
    std::string name_and_id;

    const std::size_t offset = m_rawdata.find( "<div" );
    if( offset != std::string::npos ) {

        JDLIB::Regex regex;
        constexpr bool icase = false;
        constexpr bool newline = true;
        constexpr bool usemigemo = false;
        constexpr bool wchar = false;
        const std::string pattern = ">([^<]+)</div><div>(警備員[^<]+)</div>";
        if( regex.exec( pattern, m_rawdata, offset, icase, newline, usemigemo, wchar ) ) {
            name_and_id = regex.replace( "\\1 / \\2" );
        }
    }

    set_sessionid( name_and_id );
}
