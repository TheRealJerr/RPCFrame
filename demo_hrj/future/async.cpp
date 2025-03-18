#include <iostream>
#include <future>
#include <thread>
int add(int left,int right)
{
    std::cout << "into add" << std::endl;
    return left + right;
}
int main()
{
    // 异步任务
    std::future<int> ret = std::async(std::launch::deferred,add,1,2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int tmp = ret.get();
    std::cout << tmp << std::endl;
    return 0;
}