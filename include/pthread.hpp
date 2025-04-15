#pragma once 

#include <iostream>
#include <functional>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

namespace PthreadModule
{
    enum class ThreadStatus
    {
        Running,
        Stop,
        Detached,
    };
    static int gcount = 0;
    using func_t = std::function<void(void)>;
    class mythread
    {
        static void* runtine(void* args)
        {
            mythread* newthis = static_cast<mythread*>(args);
            
            newthis->_status = ThreadStatus::Running;
            // std::cout << "线程函数开始了" << newthis->_name << std::endl;
            newthis->_func();
            // std::cout << "线程函数结束了" << newthis->_name << std::endl;
            newthis->_status = ThreadStatus::Stop;
            return nullptr;
        }
    public:
        template <typename Func,class ...Args>
        mythread(Func&& func,Args&&... args):
            _pid(::getpid()),
            _name("thread" + std::to_string(gcount++)),
            _status(ThreadStatus::Stop),
            _func(std::bind(std::forward<Func>(func),std::forward<Args>(args)...)),
            _joinable(false)
        {
            // 将我们函数初始化好
        }

        void Start()
        {
            // 将我们线程启动
            if(_status == ThreadStatus::Stop)
            {
                
                ::pthread_create(&_tid,nullptr,runtine,this);
            }
        }

        void Detach()
        {
            if(_status != ThreadStatus::Detached)
            {
                ::pthread_detach(_tid);
                _status = ThreadStatus::Detached;
            }
        }

        void Join()
        {
            if(_joinable == false)
            {
                ::pthread_join(_tid,nullptr);
                _joinable = true;
            }
            
        }

        void Cancel()
        {
            if(_status == ThreadStatus::Running){
                ::pthread_cancel(_tid);
                _status = ThreadStatus::Stop;
            }
        }        
    private:
        pid_t _pid;
        ::pthread_t _tid;
        std::string _name;
        func_t _func;
        ThreadStatus _status;
        bool _joinable;
    };

}

namespace StandardModule
{
    using thread = PthreadModule::mythread;
}



































// namespace mycode{
//     enum STATUS{
//         running,
//         stop,
//     };
//     static int num = 0;
//     template <class T>
//     class Thread{
//         using task_t = std::function<void*(T&)>;
//         static void* runtine(void* args)
//         {
//             Thread* pt = static_cast<Thread*>(args);
//             pt->_func(_data);
//             return nullptr;
//         }
//     public:
//         Thread(task_t func):
//             _tid(pthread_self()),_pid(::getpid()),_status(STATUS::stop),_func(func),_data(data)
//         {
//             _name = "thread-" + std::to_string(num++);
//         }

//         void Start()
//         {
//             if(_status != running)
//             {
//                 pthread_create(&_tid,runtine,static_cast<void*>(this));
//             }
//         }
//         void Detach()
//         {
//             if(_attached == false)
//             {
//                 pthread_detach(_tid);
//                 _attached = true;
//             }
//         }
//         bool Join()
//         {
//             int ret = pthread_join(_tid,nullptr);
//             if(ret != 0) return false;
//             return true;
//         }
//     private:
//         std::string _name;
//         pthread_t _tid;
//         pid_t _pid;
//         task_t _func;
//         enum STATUS _status;
//         bool _attached;
//         T _data;
//     };
    
// }