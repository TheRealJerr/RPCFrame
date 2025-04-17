#include "../../client/RpcClient.hpp"

void callback(const std::string& key,const std::string& msg)
{
    ILOG("%s收到消息:%s",key.c_str(),msg.c_str());
}
int main()
{
    auto client = std::make_shared<rpcframe::client::TopicClient>(std::make_pair("127.0.0.1", 7070));
    bool ret = client->createTopic("hello");
    if(ret == false)
    {
        ELOG("创建失败");
    }
    else
    {
        // 订阅
        client->subscribeTopic("hello",callback);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client->shutDown();
    return 0;
}