// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "login2ch.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"
#include "command.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"

#include <cstring>


namespace CORE::ch {
constexpr std::size_t kSizeOfRawData = 64 * 1024;
}


CORE::Login2ch* instance_login2ch = nullptr;

CORE::Login2ch* CORE::get_login2ch()
{
    if( ! instance_login2ch ) instance_login2ch = new CORE::Login2ch();
    assert( instance_login2ch );

    return instance_login2ch;
}


void CORE::delete_login2ch()
{
    if( instance_login2ch ){
        instance_login2ch->terminate_load();
        delete instance_login2ch;
    }
    instance_login2ch = nullptr;
}


using namespace CORE;


Login2ch::Login2ch()
    : SKELETON::Login( URL_LOGIN2CH )
{
#ifdef _DEBUG
    std::cout << "Login2ch::Login2ch\n";
#endif
}


//
// ログアウト
//
void Login2ch::logout()
{
#ifdef _DEBUG
    std::cout << "Login2ch::logout\n";
#endif
    if( is_loading() ) return;
    
    SKELETON::Login::set_login_now( false );
    SKELETON::Login::set_sessionid( std::string() );
    SESSION::set_login2ch( false );
}


//
// ログイン開始
//
void Login2ch::start_login()
{
    if( is_loading() ) return;

#ifdef _DEBUG
    std::cout << "Login2ch::start_login url = " << CONFIG::get_url_login2ch() << std::endl;
#endif 

    set_str_code( "" );

    finish();
#if 0 // サイトの更新に対応していないためサポートを中止
    if( ! SESSION::is_online() ){

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();
        return;
    }
    if( CONFIG::get_url_login2ch().empty() || get_username().empty() || get_passwd().empty() ){

        finish();
        return;
    }

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = CONFIG::get_url_login2ch();
    data.agent = "DOLIB/1.00";
    data.ex_field = "X-2ch-UA: " + CONFIG::get_x_2ch_ua() + "\r\n";

    data.str_post = "ID=";
    data.str_post += get_username();
    data.str_post += "&PW=";
    data.str_post += get_passwd();

    logout();
    if( m_rawdata.capacity() < ch::kSizeOfRawData ) m_rawdata.reserve( ch::kSizeOfRawData );
    m_rawdata.clear();

    start_load( data );
#endif
}


//
// データ受信
//
void Login2ch::receive_data( std::string_view buf )
{
#ifdef _DEBUG
    std::cout << "Login2ch::receive_data\n";
#endif

    m_rawdata.append( buf );
}


//
// データ受信完了
//
void Login2ch::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Login2ch::receive_finish code = " << get_code()
              << " rawdata size = " << m_rawdata.size() << std::endl;
#endif

#if 0 // サイトの更新に対応していないためサポートを中止
    bool show_err = true;

    if( ! m_rawdata.empty() && get_code() == HTTP_OK ){

        // 末尾のLFを除去
        const std::size_t pos_lf = m_rawdata.find( '\n' );
        if( pos_lf != std::string::npos ) {

            m_rawdata.erase( pos_lf );
#ifdef _DEBUG
            std::cout << "removed LF\n";
#endif
        }

        // SID 取得
        std::string sid = m_rawdata;

        if( sid.rfind( "SESSION-ID=", 0 ) == 0 ){

            sid = sid.substr( strlen( "SESSION-ID=" ) );

#ifdef _DEBUG
//            std::cout << "sid = " << sid << std::endl;
#endif
            if( sid.rfind( "ERROR", 0 ) != 0 ){
                SKELETON::Login::set_login_now( true );
                SKELETON::Login::set_sessionid( sid );
                show_err = false;
                SESSION::set_login2ch( true );
            }
            else{
                MISC::ERRMSG( "2chログイン失敗 : sid = " + sid );
                set_str_code( get_str_code() + "\nIDとパスワードを確認して下さい" );
            }
        }
        else set_str_code( get_str_code() + "\n認証サーバーのURLを確認して下さい" );
    }

    // エラー表示
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( nullptr, "オフラインです" );
        mdiag.run();
    }
    else if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "IDまたはパスワードが設定されていません\n\n設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
    }
    else if( CONFIG::get_url_login2ch().empty() ){
        SKELETON::MsgDiag mdiag( nullptr, "2chの認証サーバのURLが指定されていません。" );
        mdiag.run();
    }
    else if( show_err ){
        std::string str_err = "ログインに失敗しました。\n";
        str_err += get_str_code();
        SKELETON::MsgDiag mdiag( nullptr, str_err );
        mdiag.run();  
    }
#endif
    SKELETON::MsgDiag mdiag( nullptr, "2chのログインは現在サポート中止しています。" );
    mdiag.run();

    // コアに受信完了を知らせる
    CORE::core_set_command( "login2ch_finished", "" );
}
