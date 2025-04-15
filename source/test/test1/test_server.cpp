#include "../../server/RpcServer.hpp"
#include "../../Common/detail.hpp"

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
void Div(const Json::Value& req,Json::Value& rsp)
{
    int num1 = req["num1"].asInt();
    int num2 = req["num1"].asInt();
    rsp = num1 - num2;
}
int main()
{

    
    std::unique_ptr<rpcframe::server::ServiceFactory> des(new rpcframe::server::ServiceFactory);
    des->setMethod("Add");
    des->setParams("num1",rpcframe::server::VType::INTEGRAL);
    des->setParams("num2",rpcframe::server::VType::INTEGRAL);
    des->setRetType(rpcframe::server::VType::INTEGRAL);
    des->setCallBack(Add);
    auto server = std::make_shared<rpcframe::server::RpcServer>(rpcframe::Address_t("120.0.0.1",8080));
    // 构建新的服务
    server->rigisterMethod(des->build());
    des->setMethod("Div");
    des->setParams("num1",rpcframe::server::VType::INTEGRAL);
    des->setParams("num2",rpcframe::server::VType::INTEGRAL);
    des->setRetType(rpcframe::server::VType::INTEGRAL);
    des->setCallBack(Div);
    server->rigisterMethod(des->build());
    server->start();
    return 0;
}