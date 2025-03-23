#include <iostream>
#include <future>

int Add(int num1, int num2) {
    std::cout << "into add!\n";
    return num1 + num2;
}

int main()
{
    //std::launch::async策略：内部创建一个线程执行函数，函数运行结果通过future获取
    //std::launch::deferred策略：同步策略，获取结果的时候再去执行任务
    std::future<int> res = std::async(std::launch::deferred, Add, 11, 22);//进行了一个异步非阻塞调用
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "--------------------------\n";
    //std::future<int>::get()  用于获取异步任务的结果，如果还没有结果就会阻塞
    std::cout << res.get() << std::endl;
    return 0;
}