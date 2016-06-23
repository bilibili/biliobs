
#ifndef WCS_WEAK_REF_H
#define WCS_WEAK_REF_H

#include "third_party/w32-pthreads/pthread.h"
#include <assert.h>
#include <qdebug.h>

namespace wcs {
    template <class T>
    class WeakRef {
    public:
        //static bool global_init() 
        //{
        //    if (pthread_mutex_init(&strong_lock_, 0))
        //        return false;

        //    code_valid_ = true;
        //    return true;
        //}
        //static bool global_release()
        //{
        //    pthread_mutex_destroy(&strong_lock_);
        //    code_valid_ = false;
        //}
        //static pthread_mutex_t strong_lock_;
        //static bool code_valid_;

    private:

        struct Ref {
            volatile T *ref;
            pthread_mutex_t ref_lock_;

            /*记录多少个对象对此ref进行引用*/
            volatile int ref_count_;
            pthread_mutex_t count_lock_;
        };
    private:
        /*保证任何存在的实例中，ref不为0*/
        Ref *ref;

    public:
        explicit WeakRef() : ref(0) {}
        bool init(T *r)
        {
            /*对于 无参构造的进行初始化,之前不能跨线程使用*/
            ref = new Ref();
            if (pthread_mutex_init(&ref->count_lock_, 0)) {
                delete ref;
                return false;
            }

            if (pthread_mutex_init(&ref->ref_lock_, 0)) {
                pthread_mutex_destroy(&ref->count_lock_);
                delete ref;
                return false;
            }

            ref->ref = r;
            ref->ref_count_ = 1;

            return true;
        }

        WeakRef(const WeakRef& src)
        {
            assert(src.ref);
            ref = src.ref;

            pthread_mutex_lock(&ref->count_lock_);
            ref->ref_count_++;
            pthread_mutex_unlock(&ref->count_lock_);
        }

        ~WeakRef()
        {
            assert(ref);
            bool no_ref;

            pthread_mutex_lock(&ref->count_lock_);
            ref->ref_count_--;

            if (!ref->ref_count_) {
                no_ref = true;
            } else
                no_ref = false;
            pthread_mutex_unlock(&ref->count_lock_);

            if (no_ref) {
                pthread_mutex_destroy(&ref->count_lock_);
                pthread_mutex_destroy(&ref->ref_lock_);

                delete ref;
            }

        }

        void invalid() 
        {
            pthread_mutex_lock(&ref->ref_lock_);
            ref->ref = 0;
            pthread_mutex_unlock(&ref->ref_lock_);
        }

        void* doOwnTask(void* (T::*task)(void))
        {
            void *ret;
            pthread_mutex_lock(&ref->ref_lock_);
            if (ref->ref) {
                ret = (((T*)(ref->ref))->*task)();
            } else {
                qDebug() << "weak_ref catched";
                ret = 0;
            }
            pthread_mutex_unlock(&ref->ref_lock_);
            return ret;
        }
        void doTask(void (*task)(T *own, void *in_arg, void *out_args))
        {
            pthread_mutex_lock(&ref->ref_lock_);
            if (ref->ref) {
                (ref->ref->*task)(own, in_arg, out_args);
            }
            pthread_mutex_unlock(&ref->ref_lock_);
        }

    };

    //template <class T>
    //bool WeakRef<T>::code_valid_ = false;
}

#endif