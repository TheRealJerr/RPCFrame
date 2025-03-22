#include "message.hpp"
#include "net.hpp"

void onMessage(const rpcframe::BaseConnection::Ptr& con,rpcframe::BaseMessage::Ptr& msg)
{
    std::string body = msg->serialize();
    std::cout << body << std::endl;
}

int main()
{
    auto client = rpcframe::ClientFactory::create("127.0.0.1",8080);
    client->setMessageCallBack(onMessage);
    client->connect();
    auto rpc_req = rpcframe::MessageFactory::create<rpcframe::RpcRequest>();
    rpc_req->setId("1111111");
    rpc_req->setMtype(rpcframe::Mtype::REQ_RPC);
    rpc_req->setMethod("Add");
    Json::Value val;
    val["num1"] = 1;
    val["num2"] = 2;
    rpc_req->setParams(val);
    if(client->send(rpc_req)) DLOG("发送成功");
    //if(client->send(rpc_req)) DLOG("发送成功");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client->shutdown();
    return 0;
}