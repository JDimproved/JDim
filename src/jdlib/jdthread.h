// ライセンス: GPL2

// スレッドクラス

#ifndef _JDTHREAD_H
#define _JDTHREAD_H

#include <pthread.h>

typedef void* ( *STARTFUNC )( void * );

enum
{
    DEFAULT_STACKSIZE = 64
};

namespace JDLIB
{
    enum
    {
        DETACH = true,
        NODETACH = false
    };

    class Thread
    {
        pthread_t m_thread;

      public:

        Thread();
        virtual ~Thread();

        const bool is_running() const { return ( m_thread ); }

        // スレッド作成
        const bool create( STARTFUNC func , void * arg, const bool detach, const int stack_kbyte = DEFAULT_STACKSIZE );

        const bool join();
    };
}

#endif
