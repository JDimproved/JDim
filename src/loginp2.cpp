// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loginp2.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"
#include "command.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include <cstring>

enum
{
    SIZE_OF_RAWDATA = 64 * 1024
};

CORE::Loginp2* instance_loginp2 = nullptr;

CORE::Loginp2* CORE::get_loginp2()
{
    if( ! instance_loginp2 ) instance_loginp2 = new CORE::Loginp2();
    assert( instance_loginp2 );

    return instance_loginp2;
}


void CORE::delete_loginp2()
{
    if( instance_loginp2 ){
        instance_loginp2->terminate_load();
        delete instance_loginp2;
    }
    instance_loginp2 = nullptr;
}


using namespace CORE;


Loginp2::Loginp2()
    : SKELETON::Login( URL_LOGINP2 )
    , m_rawdata( nullptr ), m_lng_rawdata( 0 )
{
#ifdef _DEBUG
    std::cout << "Loginp2::Loginp2\n";
#endif
}


Loginp2::~Loginp2()
{
#ifdef _DEBUG
    std::cout << "Loginp2::~Loginp2\n";
#endif

    if( m_rawdata ) free( m_rawdata );
}



//
// ログアウト
//
void Loginp2::logout()
{
#ifdef _DEBUG
    std::cout << "Loginp2::logout\n";
#endif
    if( is_loading() ) return;
    
    SKELETON::Login::set_login_now( false );
    SKELETON::Login::set_sessionid( std::string() );
    SKELETON::Login::set_sessiondata( std::string() );
    SESSION::set_loginp2( false );
}


//
// ログイン開始
//
void Loginp2::start_login()
{
    if( is_loading() ) return;

#ifdef _DEBUG
    std::cout << "Loginp2::start_login url = " << CONFIG::get_url_loginp2() << std::endl;
#endif 

    // cid の取得に成功したら receive_finish() の中でもう一度ローダを起動して csrfid を取得する
    m_loading_csrfid = false;  

    set_str_code( "" );

    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( nullptr, "オフラインです" );
        mdiag.run();
        return;
    }

    if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "IDまたはパスワードが設定されていません\n\n設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
        return;
    }

    if( CONFIG::get_url_loginp2().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "p2の認証サーバのアドレスが指定されていません。" );
        mdiag.run();
        return;
    }

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = CONFIG::get_url_loginp2() + "?guid=ON";

    data.contenttype = "application/x-www-form-urlencoded";
    data.str_post = "form_login_id=";
    data.str_post += MISC::url_encode( get_username() );
    data.str_post += "&form_login_pass=";
    data.str_post += MISC::url_encode( get_passwd() );
    data.str_post += "&ctl_regist_cookie=1"; // p2(rep2)
    data.str_post += "&regist_cookie=1"; // p2(rep2)
    data.str_post += "&ctl_register_cookie=1"; // p2.2ch.net(2012/01/24修正)
    data.str_post += "&register_cookie=1"; // p2.2ch.net(2012/01/24修正)
//    data.str_post += "&submit_member=" + MISC::charset_url_encode( "ユーザログイン", "MS932" );  // 2009/12/20 仕様変更
    data.str_post += "&submit_userlogin=" + MISC::charset_url_encode( "ユーザログイン", "MS932" );

    logout();
    if( ! m_rawdata ) m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    memset( m_rawdata, 0, SIZE_OF_RAWDATA );
    m_lng_rawdata = 0;

    start_load( data );
}


//
// データ受信
//
void Loginp2::receive_data( const char* data, size_t size )
{
#ifdef _DEBUG
    std::cout << "Loginp2::receive_data\n";
#endif

    if( m_lng_rawdata + size < SIZE_OF_RAWDATA ){

        memcpy( m_rawdata + m_lng_rawdata , data, size );
        m_lng_rawdata += size;
    }
}


//
// データ受信完了
//
void Loginp2::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Loginp2::receive_finish code = " << get_code() << std::endl;
    std::cout << "lng_rawdata = " << m_lng_rawdata << std::endl;
    if( !m_loading_csrfid && m_rawdata ) std::cout << m_rawdata << std::endl;
#endif

    // csrfid 取得
    if( m_loading_csrfid ){

        std::string csrfid;

        if( m_rawdata && get_code() == HTTP_OK ){

            m_rawdata[ m_lng_rawdata ] = '\0';

            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::string query = "csrfid=([^\"]*)";

            if( regex.exec( query, m_rawdata, offset, icase, newline, usemigemo, wchar ) ) csrfid = regex.str( 1 );
        }

#ifdef _DEBUG
        std::cout << "csrfid = " << csrfid << std::endl;
#endif

        if( ! csrfid.empty() ){

            set_login_now( true );
            set_sessiondata( csrfid );
            SESSION::set_loginp2( true );

            CORE::core_set_command( "loginp2_finished", "" );
        }
        else{

            SKELETON::MsgDiag mdiag( nullptr, "csrfid の取得に失敗しました。p2のサーバが落ちていないか確認して下さい。" );
            mdiag.run();
        }

        return;
    }


    ///////////////////////////////////

    // p2.2ch.net -> w2.p2.2ch.netなどのリダイレクト

    if( get_code() == HTTP_REDIRECT ){
        std::string loc = location();

        if( ! loc.empty() ){
            // ログインサーバ書き換え
            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::string query = "^([^#&\?]*/)";
            if( regex.exec( query, loc, offset, icase, newline, usemigemo, wchar ) ){

                if( CONFIG::get_url_loginp2() != regex.str( 1 ) ){
                    std::string str_err = "p2のリダイレクト要求がされました。ログイン先を変更してよろしいですか？\n\n";
                    str_err += regex.str( 1 );
                    SKELETON::MsgDiag mdiag( nullptr, str_err, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
                    if( mdiag.run() != Gtk::RESPONSE_YES ) {
                        return;
                    }
                    CONFIG::set_url_loginp2( regex.str( 1 ) );
                }
            }else{
                return; // 不当なリダイレクト先
            }

            // Cookieが付いてきたら、そのままリダイレクト先へ投げる
            std::string cookie;

            std::list< std::string >::const_iterator it = cookies().begin();
            for( ; it != cookies().end(); ++it ){

                JDLIB::Regex regex;
                const size_t offset = 0;
                const bool icase = false;
                const bool newline = true;
                const bool usemigemo = false;
                const bool wchar = false;

                std::string query = "^([^=]*?=[^;]*?)";
                if( regex.exec( query, (*it), offset, icase, newline, usemigemo, wchar ) ){
                    if( cookie.empty() ){
                        cookie = regex.str( 1 );
                    }else{
                        cookie += "; " + regex.str( 1 );
                    }
                }
            }
#ifdef _DEBUG
            std::cout << "cookie = " << cookie << std::endl;
            std::cout << "redirect = " << loc << std::endl;
            std::cout << std::endl;
#endif

            // リダイレクト先をロード
            JDLIB::LOADERDATA data;
            data.init_for_data();
            data.url = loc;
            data.referer = CONFIG::get_url_loginp2(); // リファラセット
            data.cookie_for_write = cookie;

            start_load( data );
    	}
    }
    ///////////////////////////////////

    // cid 取得

    std::string cid;

    if( m_rawdata && get_code() == HTTP_OK ){

        std::list< std::string >::const_iterator it = cookies().begin();
        for( ; it != cookies().end(); ++it ){

#ifdef _DEBUG
            std::cout << ( *it ) << std::endl;
#endif

            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::string query = "cid=([^;]*)?";

            if( regex.exec( query, (*it), offset, icase, newline, usemigemo, wchar ) ){

                if( regex.str( 1 ) != "deleted" ){
                    cid = regex.str( 1 );
                    break;
                }
            }
        }

#ifdef _DEBUG
        std::cout << "cid = " << cid << std::endl;
#endif

        if( ! cid.empty() ){

            set_sessionid( cid );

            // ( menu.php から ) csrfid 取得
            JDLIB::LOADERDATA data;
            data.init_for_data();
            data.url = CONFIG::get_url_loginp2() + "menu.php";
            data.referer = CONFIG::get_url_loginp2(); // リファラセット
            data.cookie_for_write = "cid="+cid;

            m_loading_csrfid = true;
            start_load( data );
        }
        else{

            std::string str_err = "ログインに失敗しました。\n\np2の認証サーバのアドレスやID、パスワード等を確認して下さい。\n\n";
            str_err += get_str_code();
            SKELETON::MsgDiag mdiag( nullptr, str_err );
            mdiag.run();  
        }
    }
}
