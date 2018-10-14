// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdthread.h"
#include "miscmsg.h"

#include <limits.h>
#include <cstring>
#ifdef WITH_STD_THREAD
#include <system_error>
#endif

using namespace JDLIB;


Thread::Thread()
{
    JDTH_CLEAR( m_thread );
#ifdef USE_GTHREAD
    if( !Glib::thread_supported() ) Glib::thread_init();
#endif
}


Thread::~Thread()
{
#ifdef _DEBUG
    std::cout << "Thread::~Thread\n";
    assert( ! is_running() );
#endif

    join();
}


#ifdef USE_GTHREAD
void Thread::slot_wrapper( STARTFUNC func, void* arg )
{
    func( arg );
}
#endif


// スレッド作成
const bool Thread::create( STARTFUNC func , void* arg, const bool detach, const int stack_kbyte )
{
    if( JDTH_ISRUNNING( m_thread ) ){
        MISC::ERRMSG( "Thread::create : thread is already running" );
        return false;
    }

#if defined( WITH_STD_THREAD ) // std::thread 使用
    static_cast< void >( stack_kbyte );

#ifdef _DEBUG
    std::cout << "Thread::create (stdthread)\n";
#endif
    try {
        m_thread = std::thread( func, arg );
    }
    catch( std::system_error& err ) {
        MISC::ERRMSG( err.what() );
        return false;
    }

    if( detach ) {
#ifdef _DEBUG
        std::cout << "detach\n";
#endif
        m_thread.detach();
        assert( m_thread.get_id() == std::thread::id() );
    }

#elif defined( USE_GTHREAD ) // gthread 使用

#ifdef _DEBUG
    std::cout << "Thread::create (gthread)\n";
#endif

    try {
        m_thread = Glib::Thread::create(
            sigc::bind( sigc::ptr_fun( Thread::slot_wrapper ), func, arg ),
            stack_kbyte * 1024, ! detach, true, Glib::THREAD_PRIORITY_NORMAL );
    }
    catch( Glib::ThreadError& err )
    {
        MISC::ERRMSG( err.what() );
        return false;
    }

    if( detach ){
#ifdef _DEBUG
        std::cout << "detach\n";
#endif
        JDTH_CLEAR( m_thread );
    }

#else // pthread 使用

#ifdef _DEBUG
    std::cout << "Thread::create (pthread)\n";
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

    if( detach ){
#ifdef _DEBUG
        std::cout << "detach\n";
#endif
        pthread_detach( m_thread );
        JDTH_CLEAR( m_thread );
    }

#endif // USE_GTHREAD

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

#if defined( WITH_STD_THREAD ) // std::thread 使用
    if( m_thread.joinable() ) {
        m_thread.join();
    }
    JDTH_CLEAR( m_thread );

#elif defined( USE_GTHREAD ) // gthread 使用

    if( m_thread->joinable() ){
        m_thread->join();
    }
    JDTH_CLEAR( m_thread );

#else // pthread 使用

    int status = pthread_join( m_thread, NULL );
    JDTH_CLEAR( m_thread );
    if( status ){
        MISC::ERRMSG( std::string( "Thread::join : " ) + strerror( status ) );
        return false;
    }

#endif // USE_GTHREAD

    return true;
}
