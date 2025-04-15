#pragma once
#include <iostream>

#include "Mutex.hpp"

namespace CondModule
{
    using namespace MutexModule;

    class CondValue{
    public:
        CondValue(){
            ::pthread_cond_init(&_cond,nullptr);
        }

        void Wait(Mutex& lock){
            ::pthread_cond_wait(&_cond,lock.MutexPtr());
        }

        void Nodify() { 
            ::pthread_cond_signal(&_cond); // 唤醒一个线程
        }

        void NodefyAll()
        {
            ::pthread_cond_broadcast(&_cond); // 唤醒所有的线程
        }

        ~CondValue(){
            ::pthread_cond_destroy(&_cond);
        }
    private:
        pthread_cond_t _cond;
    };
}
namespace CondModule
{
    using cond_variable = CondModule::cond_variable;
}
