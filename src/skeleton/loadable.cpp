// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "loadable.h"

#include "jdlib/loader.h"
#include "jdlib/misctime.h"

#include "httpcode.h"

using namespace SKELETON;

Loadable::Loadable()
    : m_loader( NULL ),
      m_low_priority( false )
{
    clear_load_data();
}


Loadable::~Loadable()
{
    terminate_load(); // 一応デストラクタ内部でも実行しておく
}


// ロード強制停止
void Loadable::terminate_load()
{
#ifdef _DEBUG
    std::cout << "Loadable::terminate_load\n";
#endif

    // loadableを delete する前に terminate_load() を呼び出さないと
    // スレッド実行中にメモリなどが初期化されてしまうため落ちる時がある
    // デストラクタ内から terminate_load() しても落ちる時があるので
    // デストラクタの外から呼び出すこと
    set_dispatchable( false );

    delete_loader();
}


// 完全クリア
void Loadable::clear_load_data()
{
    m_code = HTTP_INIT;
    m_str_code = std::string();
    m_date_modified = std::string();
    m_cookies.clear();
    m_location = std::string();
    m_total_length = 0;
    m_current_length = 0;
}



const bool Loadable::is_loading() const
{
    if( ! m_loader ) return false;

    // m_loader != NULL ならローダ起動中ってこと
    return true;
}


//
// 更新時刻
//
const time_t Loadable::get_time_modified()
{
    time_t time_out;
    time_out = MISC::datetotime( m_date_modified );
    if( time_out == 0 ) time_out = time( NULL ) - 600;

#ifdef _DEBUG
    std::cout << "Loadable::get_time_modified " << m_date_modified << std::endl
              << " -> " << time_out << std::endl;
#endif

    return time_out; 
}




void Loadable::delete_loader()
{
    if( m_loader ) delete m_loader;
    m_loader = NULL;
}




//
// ロード開始
//
bool Loadable::start_load( const JDLIB::LOADERDATA& data )
{
    if( is_loading() ) return false;

#ifdef _DEBUG
    std::cout << "Loadable::start_load url = " << data.url << std::endl;
#endif

    assert( m_loader == NULL );
    m_loader = new JDLIB::Loader( m_low_priority );

    // 情報初期化
    // m_date_modified, m_cookie は初期化しない
    m_code = HTTP_INIT;
    m_str_code = std::string();
    m_location = std::string();
    m_total_length = 0;
    m_current_length = 0;

    if( !m_loader->run( this, data ) ){
        m_code = get_loader_code();
        m_str_code = get_loader_str_code();
        delete_loader();
        return false;
    }

    return true;
}



//
// ロード中止
//
void Loadable::stop_load()
{
    if( m_loader ) m_loader->stop();
}



// ローダーからコールバックされてコードなどを取得してから
// receive_data() を呼び出す
void Loadable::receive( const char* data, size_t size )
{
    m_code = get_loader_code();
    if( ! m_total_length && m_code != HTTP_INIT ) m_total_length = get_loader_length();
    m_current_length += size;

    receive_data( data, size );
}


// 別スレッドで動いているローダからfinish()を呼ばれたらディスパッチして
// メインスレッドに制御を戻してから callback_dispatch()を呼び出す。
// そうしないと色々不具合が生じる
void Loadable::finish()
{
#ifdef _DEBUG
    std::cout << "Loadable::finish\n";
#endif

    dispatch();
}


//
// ローダを削除してreceive_finish()をコール
//
void Loadable::callback_dispatch()
{
#ifdef _DEBUG
    std::cout << "Loadable::callback_dispatch\n";
#endif

    // ローダを削除する前に情報保存
    m_code = get_loader_code();
    if( ! get_loader_str_code().empty() ) m_str_code = get_loader_str_code();
    if( ! get_loader_contenttype().empty() ) m_contenttype = get_loader_contenttype();
    if( ! get_loader_modified().empty() ) m_date_modified = get_loader_modified();
    if( ! get_loader_cookies().empty() ) m_cookies = get_loader_cookies();
    if( ! get_loader_location().empty() ) m_location = get_loader_location();

#ifdef _DEBUG
    std::cout << "delete_loader\n";
#endif    

    delete_loader();

#ifdef _DEBUG
    std::cout << "code = " << m_code << std::endl;
    std::cout << "str_code = " << m_str_code << std::endl;
    std::cout << "contenttype = " << m_contenttype << std::endl;
    std::cout << "modified = " << m_date_modified << std::endl;
    std::cout << "location = " << m_location << std::endl;
    std::cout << "total_length = " << m_total_length << std::endl;
    std::cout << "current length = " << m_current_length << std::endl;
#endif

    receive_finish();
}



// ローダから各種情報の取得

const int Loadable::get_loader_code()
{
    if( ! m_loader ) return HTTP_INIT;

    return m_loader->data().code;
}


const std::string Loadable::get_loader_str_code()
{
    if( ! m_loader ) return std::string();

    return m_loader->data().str_code;
}


const std::string Loadable::get_loader_contenttype()
{
    if( ! m_loader ) return std::string();

    return m_loader->data().contenttype;
}


const std::string Loadable::get_loader_modified()
{
    if( ! m_loader ) return std::string();

    return m_loader->data().modified;
}


const std::list< std::string > Loadable::get_loader_cookies()
{
    if( ! m_loader ) return std::list< std::string >();

    return m_loader->data().list_cookies;
}


const std::string Loadable::get_loader_location()
{
    if( ! m_loader ) return std::string();

    return m_loader->data().location;
}


const size_t Loadable::get_loader_length()
{
    if( ! m_loader ) return 0;

    return m_loader->data().length;
}
