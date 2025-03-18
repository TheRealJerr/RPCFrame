#include "../../client/rpc_client.hpp"

void callback(const std::string &key, const std::string &msg) {
    ILOG("%s 主题收到推送过来的消息： %s", key.c_str(), msg.c_str());
}

int main()
{
    //1. 实例化客户端对象
    auto client = std::make_shared<bitrpc::client::TopicClient>("127.0.0.1", 7070);
    //2. 创建主题
    bool ret = client->create("hello");
    if (ret == false) {
        ELOG("创建主题失败！");
    }
    //3. 订阅主题
    ret = client->subscribe("hello", callback);
    //4. 等待->退出
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client->shutdown();
    return 0;
}