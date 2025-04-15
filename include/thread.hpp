#pragma once
#include <iostream>
#include <functional>
#include <pthread.h>
#include <unistd.h>
namespace ThreadModule{
    // enum 
    // {
    //     running,
    //     stop,
    //     detached,
    // };

    // class pthread{
    //     static void* runtine(void* args)
    //     {
    //         pthread* newthis = static_cast<pthread*>(args);
            
    //     }
    // public:
    //     // template <class ...Args>
    //     // using func_t = std::function<void(Args&&...)>; // 这里使用的是可变参数列表, 因为我也不熟练, 这里暂时只是尝试
    //     template <class _Func,class ...Args>
    //     pthread(_Func&& func,Args&&... args):_data<_Func,Args...>(std::forward<_Func>(func),std::forward<Args>(args)...),_status(stop),\
    //         _joinable(false),_tid(::pthread_self()),_pid(getpid())
    //     {
    //         // 直接启动我们的线程
    //         ::pthread_create(&_tid,nullptr,runtine,(void*)this);
    //     }
    // private:
    //     int _status;
    //     bool _joinable;
    //     template <class _Func,class ...Args>
    //     struct data{
    //         _data(_Func&& __func,Args&&... __args):_func(std::forward<_Func>(__func)),_args(std::forward<Args>(__args)...)
    //         {}
    //         _Func&& _func;
    //         Args&&... _args;
    //     };

    //     pthread_t _tid;
    //     pid_t _pid;
    //     // 这里是可变参数
    // };
    
};
#include <pthread.h>
#include <unistd.h>
#include <functional>
#include <utility>
#include <memory>
#include <tuple>

namespace ThreadModule {

    enum class ThreadStatus {
        Running,
        Stopped,
        Detached,
    };

    class pthread {
        using func_t = std::function<void()>;

    public:
        template <class _Func, class... Args>
        pthread(_Func&& func, Args&&... args)
            : _status(ThreadStatus::Stopped),
              _joinable(true),
              _tid(::pthread_self()),
              _pid(getpid()),
              _task(std::make_unique<Task>(
                  std::bind(std::forward<_Func>(func), std::forward<Args>(args)...))) {
            // 启动线程
            if (::pthread_create(&_tid, nullptr, runtine, _task.get()) != 0) {
                throw std::runtime_error("Failed to create thread");
            }
            _status = ThreadStatus::Running;
        }

        ~pthread() {
            if (_joinable) {
                ::pthread_detach(_tid);
            }
        }

        // 禁用拷贝
        pthread(const pthread&) = delete;
        pthread& operator=(const pthread&) = delete;

        // 启用移动
        pthread(pthread&& other) noexcept
            : _status(other._status),
              _joinable(other._joinable),
              _tid(other._tid),
              _pid(other._pid),
              _task(std::move(other._task)) {
            other._joinable = false;
        }

        pthread& operator=(pthread&& other) noexcept {
            if (this != &other) {
                if (_joinable) {
                    ::pthread_detach(_tid);
                }
                _status = other._status;
                _joinable = other._joinable;
                _tid = other._tid;
                _pid = other._pid;
                _task = std::move(other._task);
                other._joinable = false;
            }
            return *this;
        }

        void join() {
            if (_joinable) {
                ::pthread_join(_tid, nullptr);
                _joinable = false;
                _status = ThreadStatus::Stopped;
            }
        }

        bool is_join() { return _joinable; }
        void detach() {
            if (_joinable) {
                ::pthread_detach(_tid);
                _joinable = false;
                _status = ThreadStatus::Detached;
            }
        }

        bool is_joinable() const { return _joinable; }

    private:
        struct Task {
            explicit Task(func_t func) : _func(std::move(func)) {}
            void run() { _func(); }
            func_t _func;
        };

        static void* runtine(void* args) {
            Task* task = static_cast<Task*>(args);
            task->run();
            return nullptr;
        }

        ThreadStatus _status;
        bool _joinable;
        pthread_t _tid;
        pid_t _pid;
        std::unique_ptr<Task> _task;
    };

}  // namespace ThreadModule
