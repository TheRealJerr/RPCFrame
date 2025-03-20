// 测试std::promise
#include <memory>
#include <iostream>
#include <future>
#include <thread>
int add(int left,int right) { return left + right; }
int main()
{
    
    std::shared_ptr<std::promise<int>> pro = std::make_shared<std::promise<int>>();
    std::future<int> res = pro->get_future();
    // 创建线程


    std::thread thr([pro]()->std::string{
        std::this_thread::sleep_for(std::chrono::seconds(1));// 休眠一秒
        pro->set_value(add(1,2));
        return "return sucess";
    });
    std::cout << res.get() << std::endl;
    std::cout << thr.hardware_concurrency() << std::endl;
    thr.join();
    return 0;
}