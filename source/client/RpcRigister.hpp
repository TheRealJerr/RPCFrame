// 客户端的服务的注册和服务的发现

#pragma once

#include "requestor.hpp"

namespace rpcframe
{
    namespace client
    {
        // 服务提供者
        class Provider
        {
        public:
            using Ptr = std::shared_ptr<Provider>;
            Provider():_requestor(std::make_shared<Requestor>())
            {}
            bool rigistMethod(const BaseConnection::Ptr& con,const std::string& method,const Address_t& addr)
            {
                // 构造请求
                auto msg = MessageFactory::create<ServiceRequest>();
                msg->setId(UUIDTool::getUUID());
                msg->setMtype(Mtype::REQ_SERVICE);
                msg->setMethod(method);
                msg->setServiceOptType(ServiceOpType::SERVICE_REGISTY);
                msg->setAddress(addr.first,addr.second);
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(msg);
                BaseMessage::Ptr msg_rsp;
                bool ret = _requestor->send(con,tmp,msg_rsp);
                if(ret == false)
                {
                    ELOG("%s注册失败",method.c_str());
                    return false;
                }
                auto service_rsp = std::dynamic_pointer_cast<ServiceResponse>(msg_rsp);
                if(service_rsp.get() == nullptr)
                {
                    // 
                    ELOG("响应转换失败");
                    return false;
                }
                if(service_rsp->rCode() != RCode::RCODE_OK)
                {
                    ELOG("服务注册失败:%s",errorCode(service_rsp->rCode()).c_str());
                    return false;
                }
                return true;
            }
        private:
            Requestor::Ptr _requestor;// 构造请求对象
        };

        // 这个类的作用帮助我们进行RR轮转
        class MethodHost
        {
        public:
            // 完美转发提高效率
            using Ptr = std::shared_ptr<MethodHost>();
            template <class Hosts>
            MethodHost(Hosts&& hosts):
                _hosts(std::forward<Hosts>(hosts)),_index(0)
            {}
            void appendHost(const Address_t& addr) { }
            void delHost(const Address_t& addr) {}
            bool empty();
            Address_t chooseAddess();
        private:
            std::mutex _mtx;
            size_t _index;
            std::vector<Address_t> _hosts;
        };
        // 服务的发现者
        class Discoverier
        {
        public:
        // RR轮转
            // 通过方法想要获取对应的主机信息，内部采用RR轮转,
            bool serviceDiscovery(const std::string& method,Address_t& host)
            {
                // 可以直接去本地查找
                // 如果为空,就去构建发现请求
            }
            // dispather调用
            void onServiceRequstor(const BaseConnection::Ptr& con,const ServiceRequest::Ptr& msg)
            {}
        private:
            // hash<method,vector<host>>
            std::unordered_map<std::string,MethodHost::Ptr> _method_hosts;
            Requestor::Ptr _requestor;
        };
    };
}