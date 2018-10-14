// ライセンス: GPL2

// ミューテックス

#ifndef JDMUTEX_H
#define JDMUTEX_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WITH_STD_THREAD
#include <mutex>
#else
#include <glibmm/thread.h>
#endif


namespace JDLIB
{
#ifdef WITH_STD_THREAD
    using Mutex = std::mutex;
    using StaticMutex = std::mutex;

    using LockGuard = std::lock_guard< std::mutex >;
    using UniqueLock = std::unique_lock< std::mutex >;
#else
    using Mutex = Glib::Mutex;
    using StaticMutex = Glib::StaticMutex;

    using LockGuard = Glib::Mutex::Lock;
    using UniqueLock = Glib::Mutex::Lock;
#endif

    static inline void unique_unlock( UniqueLock& lock )
    {
#ifdef WITH_STD_THREAD
        lock.unlock();
#else
        lock.release();
#endif
    }
}

#ifdef WITH_STD_THREAD
// std::mutexは初期化子を必要としないがglibmmに合わせる
#define JDLIB_STATIC_MUTEX_INIT {}
#else
#define JDLIB_STATIC_MUTEX_INIT GLIBMM_STATIC_MUTEX_INIT
#endif

#endif
