// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscthread.h"
#include "miscmsg.h"

#include <limits.h>

// スレッド作成
bool MISC::thread_create( THREAD_T& thread, STARTFUNC func , void* arg, const bool detach, const int stack_kbyte )
{
#ifdef _DEBUG
    std::cout << "MISC::thread_create\n";
#endif

    int status;
    pthread_attr_t attr;
    size_t stacksize = PTHREAD_STACK_MIN + stack_kbyte * 1024;
    pthread_attr_init( &attr );
    pthread_attr_setstacksize( &attr, stacksize );
    status = pthread_create( &thread, &attr, func, arg );
    pthread_attr_destroy( &attr );

    if( status ){
        MISC::ERRMSG( std::string( "MISC::thread_create : " ) + strerror( status ) );
        return false;
    }
    else{

        if( detach ){
#ifdef _DEBUG
            std::cout << "detach\n";
#endif
            pthread_detach( thread );
        }
    }

#ifdef _DEBUG
    std::cout << "thread = " << thread << std::endl;
#endif

    return true;
}


bool MISC::thread_join( const THREAD_T thread )
{
#ifdef _DEBUG
    std::cout << "MISC::thread_join thread = " << thread << std::endl;
#endif

    if( ! thread ) return true;

    int status = pthread_join( thread, NULL );
    if( status ){
        MISC::ERRMSG( std::string( "MISC::thread_join : " ) + strerror( status ) );
        return false;
    }

    return true;
}
