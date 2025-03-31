#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include "../Common/dispatcher.hpp"
#include "../server/RpcRouter.hpp"

void onMessage(const rpcframe::BaseConnection::Ptr& con,rpcframe::RpcRequest::Ptr& msg)
{
    std::cout << "收到了Rpc请求" << std::endl;
    std::string body = msg->serialize();
    std::cout << body << std::endl;
   
}

void onMessageTopic(const rpcframe::BaseConnection::Ptr& con,rpcframe::TopicRequest::Ptr& msg)
{
    std::cout << "收到了Service请求" << std::endl;
    std::string body = msg->serialize();
    std::cout << body << std::endl;
   
}
void onMessageService(const rpcframe::BaseConnection::Ptr& con,rpcframe::ServiceRequest::Ptr& msg)
{
    std::cout << "收到了Service请求" << std::endl;
    std::string body = msg->serialize();
    std::cout << body << std::endl;
   
}
// 业务回调
void Add(const Json::Value& req,Json::Value& rsp)
{
    int num1 = req["num1"].asInt();
    int num2 = req["num2"].asInt();
    rsp = num1 + num2;
}
int main()
{
    auto dispatch = std::make_shared<rpcframe::DisPatcher>(); // 创建事件派发器
   

    auto router = std::make_shared<rpcframe::server::RpcRouter>();
    // 构建新的服务
    std::unique_ptr<rpcframe::server::ServiceFactory> des(new rpcframe::server::ServiceFactory);
    des->setMethod("Add");
    des->setParams("num1",rpcframe::server::VType::INTEGRAL);
    des->setParams("num2",rpcframe::server::VType::INTEGRAL);
    des->setRetType(rpcframe::server::VType::INTEGRAL);
    des->setCallBack(Add);
    router->rigisterMethod(des->build());

    auto cb = std::bind(&rpcframe::server::RpcRouter::onRpcRequest,router.get(),std::placeholders::_1,std::placeholders::_2);
    dispatch->rigisterHandler<rpcframe::RpcRequest>(rpcframe::Mtype::REQ_RPC,cb);
    // bind事件派发起给server对象
    auto message_cb = std::bind(&rpcframe::DisPatcher::onMessage,dispatch.get(),std::placeholders::_1,std::placeholders::_2);
    auto server = rpcframe::ServerFactory::create(8080);
    server->setMessageCallBack(message_cb);
    server->start();
    return 0;
}