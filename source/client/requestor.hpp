// 这个头文件的作用的发送请求和Serivce处理相应
#pragma once
#include <iostream>
#include "../Common/detail.hpp"
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include <future>
// 通过requestor进行事件派发
// 客户端提供了三种方式
// 1. 异步通过
namespace rpcframe
{
    namespace client
    {

        class Requestor
        {
        public:
            using AsyncResponse = std::future<BaseMessage::Ptr>;
            using RequestCallBack = std::function<void(BaseMessage::Ptr &)>;
            class RequestDescribe
            {
            public:
                using Ptr = std::shared_ptr<RequestDescribe>;
    
                rpcframe::BaseMessage::Ptr request;
                RType type;
                std::promise<BaseMessage::Ptr> response; // 异步的应答
                RequestCallBack _req_call_back;          // 针对响应信息进行回调函数
            };

        public:
            using Ptr = std::shared_ptr<Requestor>;
            // 应答请求
            void onResponse(const BaseConnection::Ptr &con, BaseMessage::Ptr &msg)
            {
                // 异步处理
                ELOG("客户端收到了消息...开始处理");
                std::string rid = msg->rid();
                auto describe = findDescribe(rid);
                if(describe.get() == nullptr)
                {
                    ELOG("收到了响应:%s,但是未找到对应的描述",rid.c_str());
                    return;
                }
                if(describe->type == RType::REQ_ASYNC)
                {
                    // 异步处理
                    ELOG("处理异步请求");
                    describe->response.set_value(msg);
                }else if(describe->type == RType::REQ_CALLBACK)
                {
                    // 收到消息,调用回调函数
                    ELOG("处理callback请求");
                    describe->_req_call_back(msg);
                }
                else{
                    ELOG("请求类型未知...");
                }
                // 描述结束，删除对应的描述
                // 我们已经设置了值，就算std::promise被析构了，std::future在外部仍然可以直接获取到值
                delDescribe(rid);
            }
            // 发送消息
            // 并且告诉返回之后你该怎么做
            // 异步操作
            // 当上层调用get的时候处理请求
            bool send(const BaseConnection::Ptr &con, BaseMessage::Ptr &msg, AsyncResponse& async_val)
            {
                // 创建一个新的描述
                RequestDescribe::Ptr rdp = newDescribe(msg,RType::REQ_ASYNC);
                async_val = rdp->response.get_future();
                if(rdp.get() == nullptr)
                {
                    // 没有空间开辟
                    ELOG("构造请求描述失败");
                    return false;
                }else ELOG("构建成功");
                
                con->send(msg);
                // 获取异步描述结果
                
                return true;
            }
            // 同步处理请求
            bool send(const BaseConnection::Ptr &con, BaseMessage::Ptr &msg, BaseMessage::Ptr& ret)
            {
            
                AsyncResponse async_val;
                ELOG("发送同步请求");
                if(send(con,msg,async_val) == false) return false;
                ELOG("正在等待...(同步请求)");
                ret = async_val.get(); // 阻塞的等待
                return true;
            }

            // 通过设置回调函数处理请求
            bool send(const BaseConnection::Ptr& con,BaseMessage::Ptr& msg,const RequestCallBack& cb)
            {
                // 调用回调
                RequestDescribe::Ptr rdp = newDescribe(msg,RType::REQ_CALLBACK,cb);

                if(rdp.get() == nullptr)
                {
                    // 没有空间开辟
                    ELOG("构造请求描述失败");
                    return false;
                }
                con->send(msg);
                ELOG("正在等待...(回调等待)");
                return true;
            }
        private:
            RequestDescribe::Ptr newDescribe(const BaseMessage::Ptr &req, RType rtype,const RequestCallBack& cb = RequestCallBack()) // 提供默认参数,表示没有队形描述
            {
                
                RequestDescribe::Ptr rdp = std::make_shared<RequestDescribe>();
                if(rdp.get() == nullptr) 
                {
                    ELOG("new describe构建失败%s",strerror(errno));
                    return nullptr;
                }
                rdp->request = req;
                rdp->type = rtype;
                if(cb && rtype == RType::REQ_CALLBACK)
                    rdp->_req_call_back = cb;
                // 添加进入
                // 构建过程不需要加锁
                std::unique_lock<std::mutex> lock(_mtx);
                _requests.insert(std::make_pair(req->rid(),rdp));
                
                return rdp;
            }
            RequestDescribe::Ptr findDescribe(const std::string& rid)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _requests.find(rid);
                if(it == _requests.end()) return nullptr;
                else return it->second;
            }
            void delDescribe(const std::string &name)
            {
                // 删除对应的理解
                std::unique_lock<std::mutex> lock(_mtx);
                _requests.erase(name);
            }
            std::mutex _mtx;
            std::unordered_map<std::string, RequestDescribe::Ptr> _requests;
        };
    }
}