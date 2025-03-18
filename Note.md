# RPC项目框架

## Jsoncpp库

jsoncpp是一个C++库，它允许操作JSON值，包括序列化和反序列化字符串。它遵循MIT许可证。
jsoncpp is a C++ library that allows manipulating JSON values, including serialization and deserialization to and from strings. It is distributed under the MIT license.

本项目通过jsoncpp实现序列化和反序列化
json囊括了对象,数组,字符串,数值,布尔值等数据类型，通过jsoncpp库可以方便地进行序列化和反序列化。
类似于map中的映射机制

{
    "name": "John",
    "age": 30,
    "married": true,
    "hobbies": ["reading", "swimming", "traveling"],
    # 嵌套对象
    "address": {
        "street": "123 Main St",
        "city": "Anytown",
        "state": "CA",
        "zip": "12345"
    }
}

**jsoncpp的用处接口**:

```c++
// Json::Value
class Json::Value{
Value &operator=(const Value &other); //Value重载了[]和=，因此所有的赋值和获取
数据都可以通过
Value& operator[](const std::string& key);//简单的⽅式完成 val["name"] =
"xx";
Value& operator[](const char* key);
Value removeMember(const char* key);//移除元素
const Value& operator[](ArrayIndex index) const; //val["score"][0]
Value& append(const Value& value);//添加数组元素val["score"].append(88);
ArrayIndex size() const;//获取数组元素个数 val["score"].size();
std::string asString() const;//转string string name =
val["name"].asString();
const char* asCString() const;//转char* char *name =
val["name"].asCString();
Int asInt() const;//转int int age = val["age"].asInt();
float asFloat() const;//转float float weight = val["weight"].asFloat();
bool asBool() const;//转 bool bool ok = val["ok"].asBool();
}
```

### StreamWriter

将Json::Value对象序列化为字符串,存储进入输出流中

StreamWriterBuilder
添加了工厂模式
用于创建StreamWriter对象
用于设置格式化选项

1. 首先通过工厂产生StreamWriter对象

### CharReader

parse(char* str, size_t length, Value& root, bool strict = true, bool allowComments = false)

### jsoncpp的使用

1. Json::Value: 中间数据存储类 ?
    1. 如果将数据对象进行序列化,就需要将数据存储进入对象中,Json::Value就是用来存储数据的对象。**是对json数据进行组织的容器**
    2. 反序列化也是将字符串提取进入json对象中,然后进行解析。

## module库

```c++
class TcpServer
{
    void start(); // 启动服务器
    void setConnectionCallBack();
    void setMessageCallBack();
}

class EventLoop
{
    void loop(); // 开始监控事件
    void quit(); // 停止循环

    Timerld runAfter(delay,cb);

};

```

## c++11中的异步操作

**std::future**:

std::async函数模版:异步执行一个函数,返回一个future对象

std::diferred: 当我们想要获取数据,才在主线程中执行

std::package_task类模版: 为一个函数生成一个一步任务对象

std::promise:类模版:实例化对象返回一个std::future,关联future

launch::async: 异步的操作,从std::future中获取结果

launch:deffered: 延迟的同步机制,只有我们想要获取结果的时候(std::future::get),才会获取结果

```c++
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
```

### package_task

std::packaged_task是一个模版类,是一个任务包,是对一个函数的二次封装,成为可调用对象放到其他线程进行调用

任务封装好了,通过上他关联的std::future来获取执行结果

1. 封装任务
2. 获取future连接
3. 得到结果

```cpp
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
```

### std::promise是对结果的封装