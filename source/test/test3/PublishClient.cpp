#include "../../client/RpcClient.hpp"

int main()
{
    // 实例化客户端对象
    // 创建主题
    // 向主题发送消息
    auto client = std::make_shared<rpcframe::client::TopicClient>(std::make_pair("127.0.0.1", 7070));
    bool ret = client->createTopic("hello");
    if(ret == false)
    {
        ELOG("创建主题失败");
    }
    else
    {
        ELOG("创建主题成功");
        for(int i = 0;i < 10;i++)
            client->publishTopic("hello","hello world" + std::to_string(i));
        client->shutDown();
    }
    return 0;
}