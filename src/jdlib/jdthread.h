// ライセンス: GPL2

// スレッドクラス

#ifndef _JDTHREAD_H
#define _JDTHREAD_H

#include <pthread.h>

#ifdef _WIN32
#define JDTH_ISRUNNING( pth ) ( ( pth ).p != NULL )
#define JDTH_CLEAR( pth ) ( ( pth ).p = NULL )
#else
#define JDTH_ISRUNNING( pth ) ( pth )
#define JDTH_CLEAR( pth ) ( pth = 0 )
#endif

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

        const bool is_running() const { return JDTH_ISRUNNING( m_thread ); }

        // スレッド作成
        const bool create( STARTFUNC func , void * arg, const bool detach, const int stack_kbyte = DEFAULT_STACKSIZE );

        const bool join();
    };
}

#endif
