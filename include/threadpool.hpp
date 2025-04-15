// 线程池的设计用到了日志

///

// 这个线程池
#pragma once
#include <iostream>
#include "/home/hrj/include/pthread.hpp"
#include "/home/hrj/include/Mutex.hpp"
#include "/home/hrj/include/Cond.hpp"
#include "/home/hrj/include/Log.hpp"
#include <queue>

#include <vector>
namespace ThreadPoolModule
{
    class IsRunning{
    public:

        bool isRunning() { 
            MutexModule::LockGuard lock(mtx);
            return _isrunning;
            
        }

        void Set()
        {
            MutexModule::LockGuard lock(mtx);
            _isrunning = true;
        }

        void Reset()
        {
            MutexModule::LockGuard lock(mtx);
            _isrunning = false;
        }
    private:
        bool _isrunning = false;
        MutexModule::Mutex mtx;
    };
    using namespace PthreadModule;
    using namespace LogModule;
    using namespace CondModule;
    using namespace MutexModule;
    using thread_task_t = std::function<void(void)>;
    static const int default_thread_num = 5; // 默认创建五个线程
    class ThreadPool
    {
        // 线程执行函数
        ThreadPool(int thread_num = default_thread_num)
        {
            
            for (int i = 0; i < thread_num; i++)
            {
                std::cout << "thread" << i << "创建完成" << std::endl;
                _threadpool.emplace_back([&]() -> void
                                         { this->runtine("thread" + std::to_string(i)); });
            }
            _IR.Set();
            int count = 0;
            for (auto &thread : _threadpool)
            {
                LOG(LogModule::LogLevel::INFO) << "thread" << count++ << "开始运行了";
                thread.Start();
            }
        }
    public:

        static ThreadPool* tp;
        static ThreadPool* getObj(int num)
        {
            return new ThreadPool(num);
        }

        bool isEmpty() const
        {
            return _task_queue.empty();
        }

        

        template <class Func, class... Args>
        void Equeue(Func &&func, Args &&...args)
        {
            if (_IR.isRunning())
            {
                MutexModule::LockGuard lock(_mtx);
                _task_queue.emplace(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
                if (_wait_num > 0)
                    _cond_wait.NodefyAll();
            }else 
                LOG(LogModule::LogLevel::INFO) << "错误的对停止的线程池中添加任务";
        }

        void Stop()
        {
            // 为了我们线程的安全所以我们需要保证线程完成任务后 进行退出
            // 我们的main线程对_isrunning进行处理, 
            _IR.Reset();
            if(_wait_num)
                _cond_wait.NodefyAll(); 
            LOG(LogModule::LogLevel::DEBUG) << "线程终止了";
        }
        // 对我们所有的线程进行回收
        void Wait()
        {
            if(_IR.isRunning() == false)
            {
                for(auto& thread : _threadpool)
                {
                    thread.Join();
                }
                LOG(LogModule::LogLevel::INFO) << "所有线程完成了退出";
            }
            
        }
        ~ThreadPool()
        {
            for (auto &thread : _threadpool)
                thread.Join();
        }

        //
    private:
        std::queue<thread_task_t> _task_queue; //  任务队列
        std::vector<mythread> _threadpool;     // 线程池
        MutexModule::Mutex _mtx;
        CondValue _cond_wait; // 线程等待条件变量
        int _wait_num = 0;
        // bool _isrunning = false; // 表示我们的线程池是否正在启动
        IsRunning _IR;
    protected:
        void runtine(const std::string &name)
        {
            std::cout << "进入到了函数中" << std::endl;
            while (true)
            {
                thread_task_t task;
                // 队列中的元素是临界区
                {
                    // std::cout << "来到了这里" << std::endl;
                    MutexModule::LockGuard lock(_mtx);
                    
                    while (isEmpty() && _IR.isRunning())
                    {
                        _wait_num++;
                        LOG(LogModule::LogLevel::INFO) << name << "在等待";
                        _cond_wait.Wait(_mtx);
                        _wait_num--;
                    }
                    if(_IR.isRunning() == false && isEmpty()) return; 

                    task = std::move(_task_queue.front());
                    _task_queue.pop();
                }
                task();
            }
        }
    };
    ThreadPool* ThreadPool::tp = nullptr;
}
