// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscthread.h"

#include <limits.h>

// スレッド作成
int MISC::thread_create( pthread_t* thread, STARTFUNC func , void* arg, int stack_kbyte )
{
    int status;
    pthread_attr_t attr;
    size_t stacksize = PTHREAD_STACK_MIN + stack_kbyte * 1024;
    pthread_attr_init( &attr );
    pthread_attr_setstacksize( &attr, stacksize );
//    status = pthread_create( thread, &attr, func, arg );
    status = pthread_create( thread, NULL, func, arg );
    pthread_attr_destroy( &attr );

    return status;
}
