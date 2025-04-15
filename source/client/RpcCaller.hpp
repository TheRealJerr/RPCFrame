#pragma once
// 这个头文件的目的是封装消息的派发
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
            using JsonAsyncReponse = std::future<Json::Value>;
            using Ptr = std::shared_ptr<RpcCaller>;
            // 通过request发送请求
            using JsonResponseCallBack = std::function<void(const Json::Value&)>;

            RpcCaller(const client::Requestor::Ptr& requstor):_requestor(requstor)
            {}

            // 同步调用
            // 同步是没有问题的
            bool call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,Json::Value& result)
            {
                auto req_msg = MessageFactory::create<RpcRequest>();
                if(req_msg.get() == nullptr) ELOG("create error");
                req_msg->setId(UUIDTool::getUUID());
                req_msg->setMethod(method);
                req_msg->setMtype(Mtype::REQ_RPC);
                req_msg->setParams(params);

                BaseMessage::Ptr rsp_msg = MessageFactory::create<RpcResponse>();
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(req_msg);
                //if(tmp.get() == nullptr) ELOG("转换失败");
                ELOG("开始发送数据");
                if(tmp.get() == nullptr) {
                    ELOG("数据报转换失败%s",strerror(errno));
                    return false;
                }
                bool ret = _requestor->send(con,tmp,rsp_msg);
                if(ret == false)
                {
                    ELOG("同步rpc请求失败");
                    return false;
                }
                
                auto t = std::dynamic_pointer_cast<RpcResponse>(rsp_msg);
                if(!t)
                {
                    ELOG("Rpc Response向下转换失败");
                    return false;
                }

                if(t->rCode() != RCode::RCODE_OK)
                {
                    ELOG("rpc请求出错%s",rpcframe::errorCode(t->rCode()).c_str());
                    return false;
                }
                result = t->result();
                return true;
                
            }
            // 异步调用 , 和同步请求类似
            bool call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,JsonAsyncReponse& result)
            {
                // 向服务器发送异步回调请求 , 通过设置回调函数,将result陷入进行set_value,当我们的上层直接调用get_value的时候阻塞等待结果
                auto req_msg = MessageFactory::create<RpcRequest>();
                req_msg->setId(UUIDTool::getUUID());
                req_msg->setMethod(method);
                req_msg->setMtype(Mtype::REQ_RPC);
                req_msg->setParams(params);
                // 调用回调函数
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(req_msg);
                // 这里一定要定义智能指针, 如果定义局部变量,析构后
                auto p_result = std::make_shared<std::promise<Json::Value>>();
                result = p_result->get_future();
                rpcframe::client::Requestor::RequestCallBack callback = std::bind(&RpcCaller::CallBack,std::placeholders::_1,p_result);
                bool ret = _requestor->send(con,tmp,callback);
                ELOG("异步发送成功");
                return ret;
            }
            // 设置回调函数
            bool call(const BaseConnection::Ptr& con,\
                const std::string method,const Json::Value& params,const JsonResponseCallBack& callback)
            {
                // 组织好请求
                auto req_msg = MessageFactory::create<RpcRequest>();
                req_msg->setId(UUIDTool::getUUID());
                req_msg->setMethod(method);
                req_msg->setMtype(Mtype::REQ_RPC);
                req_msg->setParams(params);
                // 调用回调函数
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(req_msg);
                Requestor::RequestCallBack cb = std::bind(&CallBack_2,callback,std::placeholders::_1);
                bool ret = _requestor->send(con,tmp,cb);
                return ret;
            }
        private:
            // 设置响应回调给异步调用
            static void CallBack(BaseMessage::Ptr& msg,std::shared_ptr<std::promise<Json::Value>> result)
            {
                auto t = std::dynamic_pointer_cast<RpcResponse>(msg);
                if(!t)
                {
                    ELOG("Rpc Response向下转换失败");
                    return;
                }

                if(t->rCode() != RCode::RCODE_OK)
                {
                    ELOG("rpc异步请求出错%s",rpcframe::errorCode(t->rCode()).c_str());
                    return;
                }
                ELOG("异步设置内容成功调用成功");
                // 将内容设置成功交给上层调用
                result->set_value(t->result());
            }
            // 针对JsonResponseCallBack的再封装
            static void CallBack_2(const JsonResponseCallBack& callback,BaseMessage::Ptr& msg)
            {
                auto rpc_rsp_msg = std::dynamic_pointer_cast<RpcResponse>(msg);
                if(rpc_rsp_msg == nullptr)
                {
                    ELOG("rpc相应,类型装换失败%s",strerror(errno));
                    return;
                }
                if(rpc_rsp_msg->rCode() != RCode::RCODE_OK)
                {
                    ELOG("请求回调出错%s",errorCode(rpc_rsp_msg->rCode()).c_str());
                    return;
                }
                callback(rpc_rsp_msg->result());
            }     
        private:
            Requestor::Ptr _requestor;
        };
    }
}