#include <iostream>
#include <future>
#include <thread>

int add(int left,int right) { 
    return left + right; 
}
int main()
{
    // 封装任务包
    std::packaged_task<int(int,int)> task(add);
    // 获取任务包关联的任务对象
    std::future<int> ret = task.get_future();
    // 执行任务
    task(1,2);
    //获取结果
    std::cout << ret.get() << std::endl;
    return 0;
}