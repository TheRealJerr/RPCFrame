#include <iostream>
#include "message.hpp"


void debugRpcRequest()
{
    rpcframe::RpcRequest::Ptr rrp = rpcframe::MessageFactory::create<rpcframe::RpcRequest>();

    rrp->setMethod("Add");
    Json::Value val;
    val["num1"] = 1;
    val["num2"] = 2;
    rrp->setParams(val);
    std::string str = rrp->serialize();
    std::cout << str << std::endl;

    rpcframe::BaseMessage::Ptr trp = rpcframe::MessageFactory::create<rpcframe::RpcRequest>();
    
    rpcframe::RpcRequest::Ptr t = std::dynamic_pointer_cast<rpcframe::RpcRequest>(trp);

    t->unserialize(str);
    std::cout << t->method() << std::endl;
    std::cout << t->params()["num1"].asInt() << std::endl;
    std::cout << t->params()["num2"].asInt() << std::endl;
}

void debugTopicRequest()
{
    rpcframe::TopicRequest::Ptr trp = rpcframe::MessageFactory::create<rpcframe::TopicRequest>();
    trp->setTopicKey("new");
    trp->setTopicOptType(rpcframe::TopicOptType::TOPIC_PUBLISH);
    trp->setTopicMsg("hello world");
    if(trp->check())
        std::cout << trp->serialize() << std::endl;
    else 
        std::cout << "error" << std::endl;
}

void debugServiceRequest()
{
    rpcframe::ServiceRequest::Ptr trp = rpcframe::MessageFactory::create<rpcframe::ServiceRequest>();
    //trp->setTopicKey("new");
    //trp->setTopicOptType(rpcframe::TopicOptType::TOPIC_PUBLISH);
    trp->setMethod("new");
    trp->setServiceOptType(rpcframe::ServiceOpType::SERVICE_ONLINE);
    trp->setAddress("110.41.14.69",8080);
    if(trp->check())
        std::cout << trp->serialize() << std::endl;
    else 
        std::cout << "error" << std::endl;
    auto t = trp->address();
    std::cout << t.first << " " << t.second << std::endl;
}

void debugRpcResponse()
{
    rpcframe::RpcResponse::Ptr rrp = rpcframe::MessageFactory::create<rpcframe::RpcResponse>();
    rrp->setRCode(rpcframe::RCode::RCODE_OK);
    Json::Value val;
    val["sum"] = 1;
    rrp->setResult(val);
    if(rrp->check())
        std::cout << rrp->serialize() << std::endl;
}

void debugTopicResponse()
{
    rpcframe::TopicResponse::Ptr rrp = rpcframe::MessageFactory::create<rpcframe::TopicResponse>();

    
}

void debugServiceResponse()
{
    rpcframe::ServiceResponse::Ptr rrp = rpcframe::MessageFactory::create<rpcframe::ServiceResponse>();
    rrp->setMethod("Add");
    rrp->setRCode(rpcframe::RCode::RCODE_OK);
    rrp->setServiceOpType(rpcframe::ServiceOpType::SERVICE_DISCOVERY);
    rrp->setHost(
        {
            {"110.41" , 1},
            {"12",2}
        }
    );
    if(rrp->check())
        std::cout << rrp->serialize() << std::endl;
    
    
}
int main()
{
    debugServiceResponse();
    return 0;
}
