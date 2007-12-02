// ライセンス: GPL2

// スレッド関係の関数

#ifndef _MISCTHREAD_H
#define _MISCTHREAD_H

#include <pthread.h>

typedef void* ( *STARTFUNC )( void * );

typedef pthread_t THREAD_T;

enum
{
    DEFAULT_STACKSIZE = 64
};

namespace MISC
{
    enum
    {
        DETACH = true,
        NODETACH = false
    };

    // スレッド作成
    bool thread_create( THREAD_T& thread, STARTFUNC func , void * arg, const bool detach, const int stack_kbyte = DEFAULT_STACKSIZE );

    bool thread_join( const THREAD_T thread );
}

#endif
