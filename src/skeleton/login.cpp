// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "login.h"

#include "cache.h"

#include "jdlib/confloader.h"

#include <sstream>
#include <cstring>
#include <sys/types.h> // chmod
#include <sys/stat.h>

using namespace SKELETON;

Login::Login( const std::string& url )
    : m_url( url ), m_login_now( 0 ), m_save_info( 0 )
{
#ifdef _DEBUG
    std::cout << "Login::Login " << get_url() << std::endl;
#endif

    read_info();
}


Login::~Login()
{
#ifdef _DEBUG
    std::cout << "Login::~Login " << get_url() << std::endl;
#endif
    if( m_save_info ) save_info();
}


void Login::set_username( const std::string& username )
{
    if( username != m_username ){
        m_username = username;
        m_save_info = true;
    }
}


void Login::set_passwd( const std::string& passwd )
{
    if( passwd != m_passwd ){
        m_passwd = passwd;
        m_save_info = true;
    }
}



//
// パスワードやユーザー名読み込み
//
void Login::read_info()
{
    std::string path = CACHE::path_passwd( get_url().substr( strlen( "jdlogin://" ) ) );

#ifdef _DEBUG
    std::cout << "Login::read_info path = " << path << std::endl;
#endif    

    JDLIB::ConfLoader cf( path, std::string() );

    m_username = cf.get_option_str( "username", "" );
    m_passwd = cf.get_option_str( "passwd", "" );

#ifdef _DEBUG
    std::cout << "user = " << m_username << " ,passwd = " << m_passwd << std::endl;
#endif
}


//
// パスワードやユーザー名書き込み
//
void Login::save_info()
{
    std::string path = CACHE::path_passwd( get_url().substr( strlen( "jdlogin://" ) ) );

#ifdef _DEBUG
    std::cout << "Login::save_info path = " << path << std::endl;
#endif    

    std::ostringstream oss;
    oss << "username = " << m_username<< std::endl
        << "passwd = " << m_passwd << std::endl;

    CACHE::save_rawdata( path, oss.str() );
    chmod( path.c_str(), S_IWUSR | S_IRUSR );
}
