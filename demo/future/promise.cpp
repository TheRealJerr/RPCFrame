#include <iostream>
#include <future>
#include <thread>
#include <memory>

int Add(int num1, int num2) {
    std::cout << "into add!\n";
    return num1 + num2;
}

int main()
{
    //   1. 在使用的时候，就是先实例化一个指定结果的promise对象，
    std::promise<int> pro;
    //   2. 通过promise对象，获取关联的future对象
    std::future<int> res = pro.get_future();
    //   3. 在任意位置给promise设置数据，就可以通过关联的future获取到这个设置的数据了
    std::thread thr([&pro](){
        int sum = Add(11 , 22);
        pro.set_value(sum);
    });

    std::cout << res.get() << std::endl;
    thr.join();
    return 0;
}