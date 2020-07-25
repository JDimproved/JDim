// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdthread.h"
#include "miscmsg.h"

#include <system_error>

using namespace JDLIB;


Thread::~Thread()
{
    join();
}


// スレッド作成
bool Thread::create( STARTFUNC func , void* arg, const bool detach, const int )
{
    if( m_thread.joinable() ){
        MISC::ERRMSG( "Thread::create : thread is already running" );
        return false;
    }

    try {
        m_thread = std::thread( func, arg );
    }
    catch( std::system_error& err ) {
        MISC::ERRMSG( err.what() );
        return false;
    }

    if( detach ) {
#ifdef _DEBUG
        std::cout << "Thread::create detach" << std::endl;
#endif
        m_thread.detach();
        assert( ! m_thread.joinable() );
    }

#ifdef _DEBUG
    std::cout << "Thread::create thread = " << m_thread.get_id() << std::endl;
#endif

    return true;
}


bool Thread::join()
{
#ifdef _DEBUG
    std::cout << "Thread::join thread = " << m_thread.get_id() << std::endl;
#endif

    if( m_thread.joinable() ) {
        m_thread.join();
    }
    return true;
}
