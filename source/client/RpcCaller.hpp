#pragma once
#include "requestor.hpp"

namespace rpcframe
{
    namespace client
    {
        // 我们给用户提供了三种接口
        // 1. 单步 2. 异步  3.设置回调函数
        class RpcCaller
        {
        public:
            using JsonReponse = std::future<Json::Value>;
            using Ptr = std::shared_future<RpcCaller>;
            // 通过request发送请求
            using JsonResponse = std::function<void(const Json::Value&)>;

            RpcCaller(const Requestor::Ptr& requstor):_requestor(requstor)
            {}

            // 同步调用
            void call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,Json::Value& result)
            {
                auto req_msg = MessageFactory::create<RpcRequest>();
                req_msg->setId(UUIDTool::getUUID());
                req_msg->setMethod(method);
                req_msg->setMtype(Mtype::REQ_RPC);
                req_msg->setParams(params);
                BaseMessage::Ptr rsp_msg;
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(req_msg);
                bool ret = _requestor->send(con,tmp,rsp_msg);
                if(ret == false)
                {
                    ELOG("同步rpc请求失败");
                    return;
                }
                auto t = std::dynamic_pointer_cast<RpcResponse>(rsp_msg);
                result = t->result();
            }
            // 异步调用
            void call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,std::future<Json::Value>& result)
            {
                auto req_msg = MessageFactory::create<RpcRequest>();
                req_msg->setId(UUIDTool::getUUID());
                req_msg->setMethod(method);
                req_msg->setMtype(Mtype::REQ_RPC);
                req_msg->setParams(params);
                auto rsp_msg = MessageFactory::create<RpcResponse>();
            }

            void call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,const JsonReponse& callback)
            {

            }
        private:
            Requestor::Ptr _requestor;
        };
    }
}