// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdthread.h"
#include "miscmsg.h"

#include <limits.h>
#include <cstring>

using namespace JDLIB;


Thread::Thread()
{
    JDTH_CLEAR( m_thread );
}


Thread::~Thread()
{
#ifdef _DEBUG
    std::cout << "Thread::~Thread\n";
    assert( ! is_running() );
#endif

    join();
}


// スレッド作成
const bool Thread::create( STARTFUNC func , void* arg, const bool detach, const int stack_kbyte )
{
    if( JDTH_ISRUNNING( m_thread ) ){
        MISC::ERRMSG( "Thread::create : thread is already running" );
        return false;
    }

#ifdef _DEBUG
    std::cout << "Thread::create\n";
#endif

    int status;
    pthread_attr_t attr;
    size_t stacksize = PTHREAD_STACK_MIN + stack_kbyte * 1024;
    pthread_attr_init( &attr );
    pthread_attr_setstacksize( &attr, stacksize );
    status = pthread_create( &m_thread, &attr, func, arg );
    pthread_attr_destroy( &attr );

    if( status ){
        MISC::ERRMSG( std::string( "Thread::create : " ) + strerror( status ) );
        return false;
    }
    else{

        if( detach ){
#ifdef _DEBUG
            std::cout << "detach\n";
#endif
            pthread_detach( m_thread );
            JDTH_CLEAR( m_thread );
        }
    }

#ifdef _DEBUG
    std::cout << "thread = " << m_thread << std::endl;
#endif

    return true;
}


const bool Thread::join()
{
    if( ! JDTH_ISRUNNING( m_thread ) ) return true;

#ifdef _DEBUG
    std::cout << "Thread:join thread = " << m_thread << std::endl;
#endif

    int status = pthread_join( m_thread, NULL );
    JDTH_CLEAR( m_thread );
    if( status ){
        MISC::ERRMSG( std::string( "Thread::join : " ) + strerror( status ) );
        return false;
    }

    return true;
}
