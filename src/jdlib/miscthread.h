// ライセンス: GPL2

// スレッド関係の関数

#ifndef _MISCTHREAD_H
#define _MISCTHREAD_H

#include <pthread.h>

typedef void* ( *STARTFUNC )( void * );

namespace MISC
{
    // スレッド作成
    int thread_create( pthread_t * thread, STARTFUNC func , void * arg, int stack_kbyte );
}

#endif
