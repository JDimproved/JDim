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

#define SIZE_OF_RAWDATA ( 64 * 1024 )

LOGIN::Login2ch* instance_login2ch = NULL;

LOGIN::Login2ch* LOGIN::get_login2ch()
{
    if( ! instance_login2ch ) instance_login2ch = new LOGIN::Login2ch();
    assert( instance_login2ch );

    return instance_login2ch;
}


void LOGIN::delete_login2ch()
{
    if( instance_login2ch ) delete instance_login2ch;
    instance_login2ch = NULL;
}


using namespace LOGIN;


Login2ch::Login2ch()
    : SKELETON::Login( URL_LOGIN2CH )
    , m_rawdata( 0 ), m_lng_rawdata( 0 )
{
#ifdef _DEBUG
    std::cout << "Login2ch::Login2ch\n";
#endif
}


Login2ch::~Login2ch()
{
#ifdef _DEBUG
    std::cout << "Login2ch::~Login2ch\n";
#endif

    if( m_rawdata ) free( m_rawdata );
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

    if( ! SESSION::is_online() ){

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();
        return;
    }
    if( get_username().empty() || get_passwd().empty() ){

        finish();
        return;
    }

    JDLIB::LOADERDATA data;    
    data.url = CONFIG::get_url_login2ch();
    data.agent = "DOLIB/1.00";
    data.ex_field = "X-2ch-UA: " + CONFIG::get_x_2ch_ua() + "\r\n";
    if( CONFIG::get_use_proxy_for_data() ) data.host_proxy = CONFIG::get_proxy_for_data();
    data.port_proxy = CONFIG::get_proxy_port_for_data();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

    data.str_post = "ID=";
    data.str_post += get_username();
    data.str_post += "&PW=";
    data.str_post += get_passwd();

    logout();
    if( ! m_rawdata ) m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    memset( m_rawdata, 0, SIZE_OF_RAWDATA );
    m_lng_rawdata = 0;

    start_load( data );
}


//
// データ受信
//
void Login2ch::receive_data( const char* data, size_t size )
{
#ifdef _DEBUG
    std::cout << "Login2ch::receive_data\n";
#endif

    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
    assert( m_lng_rawdata < SIZE_OF_RAWDATA );
}


//
// データ受信完了
//
void Login2ch::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Login2ch::receive_finish code = " << get_code() << " lng_rawdata = " << m_lng_rawdata << std::endl;
//    if( m_rawdata ) std::cout << m_rawdata << std::endl;
#endif

    std::string sid;
    bool show_err = true;

    if( m_rawdata && get_code() == HTTP_OK ){

        // 末尾のLFを除去
        char *pos_lf = strchr( m_rawdata, '\n' );
        if( pos_lf ){

            *pos_lf= '\0';

#ifdef _DEBUG
            std::cout << "removed LF\n";
#endif
        }

        // SID 取得
        sid = std::string( m_rawdata );

        if( sid.find( "SESSION-ID=" ) == 0 ){

            sid = sid.substr( strlen( "SESSION-ID=" ) );

#ifdef _DEBUG
//            std::cout << "sid = " << sid << std::endl;
#endif
            if( sid.find( "ERROR" ) != 0 ){
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
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
    }
    else if( get_username().empty() || get_passwd().empty() ){
        SKELETON::MsgDiag mdiag( NULL, "IDまたはパスワードが設定されていません\n\n設定→ネットワーク→パスワードで設定してください" );
        mdiag.run();
    }
    else if( show_err ){
        std::string str_err = "ログインに失敗しました。\n";
        str_err += get_str_code();
        SKELETON::MsgDiag mdiag( NULL, str_err );
        mdiag.run();  
    }

    // コアに受信完了を知らせる
    CORE::core_set_command( "login2ch_finished", "" );
}
