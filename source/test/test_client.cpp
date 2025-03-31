#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include "../client/requestor.hpp"
#include "../client/RpcCaller.hpp"
#include "../Common/dispatcher.hpp"
void onMessage(const rpcframe::BaseConnection::Ptr& con,rpcframe::BaseMessage::Ptr& msg)
{
    std::string body = msg->serialize();
    std::cout << body << std::endl; 
    
}

int main()
{

    auto requstor = std::make_shared<rpcframe::client::Requestor>();
    auto caller = std::make_shared<rpcframe::client::RpcCaller>(requstor);

    auto dispatcher = std::make_shared<rpcframe::DisPatcher>();
    auto cb = std::bind(&rpcframe::client::Requestor::onResponse,requstor.get(),std::placeholders::_1,std::placeholders::_2);

    dispatcher->rigisterHandler<rpcframe::BaseMessage>(rpcframe::Mtype::RSP_RPC,cb);
    auto client = rpcframe::ClientFactory::create("127.0.0.1",8080);
    
    client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage,dispatcher.get(),std::placeholders::_1,std::placeholders::_2));
    client->connect();
    auto con = client->connection();

    Json::Value params,result;
    params["num1"] = 1;
    params["num2"] = 2;

    bool ret = caller->call(con,"Add",params,result);
    if(ret == false)
    {
        ELOG("获取结果失败");
        
    }else std::cout << "获取结果成功: result: " << result.asInt() << std::endl;

    rpcframe::client::RpcCaller::JsonReponse rsp;
    caller->call(con,"Add",params,rsp);
    
    
    // auto rpc_req = rpcframe::MessageFactory::create<rpcframe::RpcRequest>();
    // rpc_req->setId("1111111");
    // rpc_req->setMtype(rpcframe::Mtype::REQ_RPC);
    // rpc_req->setMethod("Add");
    // Json::Value val;
    // val["num1"] = 1;
    // val["num2"] = 2;
    // rpc_req->setParams(val);
    // if(client->send(rpc_req)) DLOG("发送成功");
    // rpc_req->setMtype(rpcframe::Mtype::REQ_TOP);
    // client->send(rpc_req);
    //if(client->send(rpc_req)) DLOG("发送成功");
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    client->shutdown();
    return 0;
}