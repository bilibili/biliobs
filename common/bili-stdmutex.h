#ifndef BILI_STDMUTEX_H
#define BILI_STDMUTEX_H
 
#include "../third_party/w32-pthreads/pthread.h"
#include <windows.h>

namespace bili
{
    struct mutex_base
    {
        pthread_mutex_t mutex_;
        
        void lock()
        {
            pthread_mutex_lock(&mutex_);
        }
        
        void unlock()
        {
            pthread_mutex_unlock(&mutex_);
        }
    };
    
    struct mutex : public mutex_base
    {
        mutex()
        {
            pthread_mutex_init(&mutex_, 0);
        }
        ~mutex()
        {
            pthread_mutex_destroy(&mutex_);
        }
    };
    
    struct recursive_mutex : public mutex_base
    {
        recursive_mutex()
        {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mutex_, &attr);
        }
        ~recursive_mutex()
        {
			pthread_mutex_destroy(&mutex_);
        }
    };
    
    template<class T>
    class lock_guard
    {
        lock_guard(const lock_guard&) = delete;
        lock_guard& operator = (const lock_guard&) = delete;
        
        T& mutexObject_;
    public:
        lock_guard(T& mutexObject) : mutexObject_(mutexObject)
        {
            mutexObject_.lock();
        } 
        
        ~lock_guard()
        {
            mutexObject_.unlock();
        }
    };
    
    struct once_flag
    {
        volatile long processedFlag;
        
        once_flag()
        {
            processedFlag = 0;
        }
    };
    
    template<class Proc>
    inline void call_once(once_flag& flag, Proc&& proc)
    {
        if (InterlockedCompareExchange(&flag.processedFlag, 1, 0) == 0)
        {
            proc();
        }
    }
};

#endif
