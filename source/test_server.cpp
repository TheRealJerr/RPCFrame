#include "message.hpp"
#include "net.hpp"

void onMessage(const rpcframe::BaseConnection::Ptr& con,rpcframe::BaseMessage::Ptr& msg)
{
    std::string body = msg->serialize();
    std::cout << body << std::endl;
    auto rpc_req = rpcframe::MessageFactory::create<rpcframe::RpcResponse>();
    rpc_req->setId("1111");
    rpc_req->setMtype(rpcframe::Mtype::REQ_RPC);
    rpc_req->setRCode(rpcframe::RCode::RCODE_OK);
    Json::Value val;
    val["sum"] = 33;
    rpc_req->setResult(val);
    con->send(rpc_req);
}

int main()
{
    auto server = rpcframe::ServerFactory::create(8080);
    server->setMessageCallBack(onMessage);
    server->start();
    return 0;
}