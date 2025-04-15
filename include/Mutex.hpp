#pragma once
#include <iostream>
#include <pthread.h>

namespace MutexModule
{
    
    class Mutex{
    public:
        Mutex() { ::pthread_mutex_init(&_mtx,nullptr); }

        void Lock() { ::pthread_mutex_lock(&_mtx); }

        void Unlock() { ::pthread_mutex_unlock(&_mtx); }

        ~Mutex() { ::pthread_mutex_destroy(&_mtx); }

        pthread_mutex_t * MutexPtr() { return &_mtx; }
    private:
        pthread_mutex_t _mtx;
    };
    
    class LockGuard{
    public:
        LockGuard(Mutex& lock):_lock(lock){
            _lock.Lock();
        }
        ~LockGuard(){
            _lock.Unlock();
        }
    private:
        Mutex& _lock;
    };
}
namespace StandardModule
{
    using mutex = MutexModule::Mutex;
}